//
// Created by Kanari on 2016/12/24.
//

#include "ix.h"
#include "ix_internal.h"

#include <stddef.h>
#include <memory>

IX_IndexHandle::IX_IndexHandle() { }

IX_IndexHandle::~IX_IndexHandle() { }

int IX_IndexHandle::__cmp(void* lhs, void* rhs) const {
#define THREE_WAY(lhs, rhs) ((lhs < rhs) ? (-1) : ((lhs == rhs) ? 0 : 1))
    if (attrType == INT) {
        return THREE_WAY(*(int*)lhs, *(int*)rhs);
    } else if (attrType == FLOAT) {
        return THREE_WAY(*(float*)lhs, *(float*)rhs);
    } else if (attrType == STRING) {
        return strncmp((const char*)lhs, (const char*)rhs, attrLength);
    } else {
        LOG(FATAL) << "unknown attrType";
    }
#undef THREE_WAY
}

void IX_IndexHandle::__initialize() {
    entrySize = offsetof(Entry, key) + upper_align<4>(attrLength);
    // b = (PF_PAGE_SIZE - offsetof(Entry, key) - offsetof(IX_PageHeader, entries)) / entrySize + 1;
    b = 4; // for debugging
    ridsPerBucket = (PF_PAGE_SIZE - offsetof(IX_BucketHeader, rids)) / sizeof(RID);
}

RC IX_IndexHandle::new_node(int *nodeNum) {
    PF_PageHandle ph;
    TRY(pfHandle.AllocatePage(ph));
    TRY(ph.GetPageNum(*nodeNum));
    TRY(pfHandle.UnpinPage(*nodeNum));
    return 0;
}

RC IX_IndexHandle::delete_node(int nodeNum) {
    LOG(FATAL) << "not implemented yet";
}

RC IX_IndexHandle::new_bucket(int *pageNum) {
    PF_PageHandle ph;
    IX_BucketHeader *header;
    TRY(pfHandle.AllocatePage(ph));
    TRY(ph.GetPageNum(*pageNum));
    TRY(ph.GetData(CVOID(header)));
    header->ridNum = 0;
    TRY(pfHandle.UnpinPage(*pageNum));
    return 0;
}

RC IX_IndexHandle::delete_bucket(int pageNum) {
    return pfHandle.DisposePage(pageNum);
}

RC IX_IndexHandle::bucket_insert(int *pageNum, const RID &rid) {
    if (*pageNum == kInvalidBucket) {
        TRY(new_bucket(pageNum));
    }
    PF_PageHandle page;
    IX_BucketHeader *header;
    TRY(pfHandle.GetThisPage(*pageNum, page));
    TRY(page.GetData(CVOID(header)));
    int ret = 0;
    if (header->ridNum == ridsPerBucket) {
        ret = IX_BUCKET_FULL;
    } else {
        for (int i = 0; i < header->ridNum; ++i) {
            if (rid == header->rids[i]) {
                ret = IX_ENTRY_EXISTS;
            }
        }
        if (ret == 0) {
            header->rids[header->ridNum++] = rid;
            TRY(pfHandle.MarkDirty(*pageNum));
        }
    }
    TRY(pfHandle.UnpinPage(*pageNum));
    return ret;
}

RC IX_IndexHandle::bucket_delete(int *pageNum, const RID &rid) {
    if (*pageNum == kInvalidBucket) {
        return IX_ENTRY_DOES_NOT_EXIST;
    }
    PF_PageHandle page;
    IX_BucketHeader *header;
    TRY(pfHandle.GetThisPage(*pageNum, page));
    TRY(page.GetData(CVOID(header)));
    RID* rids = header->rids;
    int ret = IX_ENTRY_DOES_NOT_EXIST;
    for (int i = 0; i < header->ridNum; ++i) {
        if (rid == rids[i]) {
            ret = 0;
            memmove(rids + i, rids + (i + 1), sizeof(RID) * (header->ridNum - i - 1));
            --header->ridNum;
        }
    }
    bool to_delete = (header->ridNum == 0);
    if (ret == 0) {
        TRY(pfHandle.MarkDirty(*pageNum));
    }
    TRY(pfHandle.UnpinPage(*pageNum));
    if (to_delete) {
        delete_bucket(*pageNum);
        *pageNum = kInvalidBucket;
    }
    return ret;
}

RC IX_IndexHandle::insert_entry(void *_header, void* pData, const RID &rid) {
    IX_PageHeader *header = (IX_PageHeader*)_header;
    short &n = header->childrenNum;
    int index = n; // the index of entry to insert AT
                   // the default value for the case where pData is greater than
                   // all the keys
    for (int i = 0; i < n; ++i) {
        Entry* entry = (Entry*)__get_entry(header->entries, i);
        int c = __cmp(entry->key, pData);
        if (c == 0) {
            TRY(bucket_insert(&entry->pageNum, rid));
            return 0;
        }
        if (c > 0) {
            index = i;
            break;
        }
    }

    ((Entry*)__get_entry(header->entries, n + 1))->pageNum =
        ((Entry*)__get_entry(header->entries, n))->pageNum;
    for (int i = n; i > index; --i) {
        Entry* to = (Entry*)__get_entry(header->entries, i);
        Entry* from = (Entry*)__get_entry(header->entries, i - 1);
        memcpy(to, from, entrySize); // NOTE: sizeof(Entry) won't work here
    }

    Entry* dest = (Entry*)__get_entry(header->entries, index);
    dest->pageNum = kInvalidBucket;
    memcpy(dest->key, pData, attrLength);
    TRY(bucket_insert(&dest->pageNum, rid));
    ++n;
    return 0;
}

RC IX_IndexHandle::insert_internal_entry(void *_header, int index, void* key, int node) {
    IX_PageHeader *header = (IX_PageHeader*)_header;
    short &n = header->childrenNum;
    char* entries = header->entries;

    if (index != n - 1) {
        char* src = ((Entry*)__get_entry(entries, index))->key;
        memmove(src + entrySize, src,
                (char*)(&((Entry*)__get_entry(entries, n - 1))->key) - src);
    }
    memcpy(((Entry*)__get_entry(entries, index))->key,
            key, attrLength);
    ((Entry*)__get_entry(entries, index + 1))->pageNum = node;
    ++n;

    return 0;
}

RC IX_IndexHandle::insert_internal(int nodeNum, int *splitNode, std::unique_ptr<char[]> *splitKey, void *pData, const RID &rid) {
    PF_PageHandle ph;
    IX_PageHeader *header;
    TRY(pfHandle.GetThisPage(nodeNum, ph));
    TRY(ph.GetData(CVOID(header)));
    char* entries = header->entries;
    short &n = header->childrenNum;
    // NOTE: entry[0] .. entry[n - 1] contains child_ptr[i] and key[i]
    //       entry[n] contains child_ptr[n]
    int ret = 0;
    *splitNode = kNullNode;
    splitKey->reset();

    int index = n - 1;
    for (int i = 0; i < n- 1; ++i) {
        Entry* entry = (Entry*)__get_entry(entries, i);
        if (__cmp(entry->key, pData) > 0) {
            index = i;
            break;
        }
    }
    int child = ((Entry*)__get_entry(entries, index))->pageNum;
    CHECK(child != kNullNode);
    PF_PageHandle c_ph;
    IX_PageHeader *c_header;
    TRY(pfHandle.GetThisPage(child, c_ph));
    TRY(c_ph.GetData(CVOID(c_header)));
    short c_type = c_header->type;
    TRY(pfHandle.UnpinPage(child));
    int lowerSplitNode;
    std::unique_ptr<char[]> lowerSplitKey;
    if (c_type == kInternalNode) {
        ret = insert_internal(child, &lowerSplitNode, &lowerSplitKey, pData, rid);
    } else {
        ret = insert_leaf(child, &lowerSplitNode, pData, rid);
    }
    if (lowerSplitNode != kNullNode) {
        // LOG(INFO) << "insert_internal: lower split";
        PF_PageHandle l_ph;
        IX_PageHeader *l_header;
        TRY(pfHandle.GetThisPage(lowerSplitNode, l_ph));
        TRY(l_ph.GetData(CVOID(l_header)));
        char* l_key;
        if (lowerSplitKey) {
            l_key = lowerSplitKey.get();
        } else {
            l_key = ((Entry*)__get_entry(l_header->entries, 0))->key;
        }
        if (n != this->b) {
            TRY(insert_internal_entry(header, index, l_key, lowerSplitNode));
        } else {
            // LOG(INFO) << "insert_internal: split";
            int m = n / 2 - 1; // entries [m, n) goes to the new node
            if (m + 1 < n - m - 1 && m + 1 <= index) {
                ++m;
            }

            splitKey->reset(new char[attrLength]);
            memcpy(splitKey->get(), ((Entry*)__get_entry(entries, m))->key,
                    attrLength);

            TRY(new_node(splitNode));
            PF_PageHandle s_ph;
            IX_PageHeader *s_header;
            TRY(pfHandle.GetThisPage(*splitNode, s_ph));
            TRY(s_ph.GetData(CVOID(s_header)));
            s_header->childrenNum = n - m - 1;
            s_header->type = kInternalNode;

            char* src = (char*)(&((Entry*)__get_entry(entries, m + 1))->pageNum);
            char* src_end = ((Entry*)__get_entry(entries, n - 1))->key;
            memcpy(s_header->entries, src, src_end - src);

            n = m + 1;
            if (index <= m) {
                TRY(insert_internal_entry(header, index, l_key, lowerSplitNode));
            } else {
                TRY(insert_internal_entry(s_header, index - (m + 1),
                            l_key, lowerSplitNode));
            }

            TRY(pfHandle.MarkDirty(*splitNode));
            TRY(pfHandle.UnpinPage(*splitNode));
        }
        TRY(pfHandle.UnpinPage(lowerSplitNode));
    }

    TRY(pfHandle.MarkDirty(nodeNum));
    TRY(pfHandle.UnpinPage(nodeNum));
    return ret;
}

RC IX_IndexHandle::insert_leaf(int nodeNum, int *splitNode, void *pData, const RID &rid) {
    PF_PageHandle ph;
    IX_PageHeader *header;
    TRY(pfHandle.GetThisPage(nodeNum, ph));
    TRY(ph.GetData(CVOID(header)));
    char* entries = header->entries;
    volatile short &n = header->childrenNum;
    *splitNode = kNullNode;
    int ret = 0;
    if (n != this->b - 1) {
        ret = insert_entry(header, pData, rid);
    } else {
        TRY(new_node(splitNode));
        PF_PageHandle s_ph;
        IX_PageHeader *s_header;
        TRY(pfHandle.GetThisPage(*splitNode, s_ph));
        TRY(s_ph.GetData(CVOID(s_header)));
        short &s_n = s_header->childrenNum;
        s_header->type = kLeafNode;
        int m = n / 2; // entries [m, n) goes to the new leaf
        bool insert_entry_in_new_leaf =
            (__cmp(((Entry*)__get_entry(entries, m))->key, pData) <= 0);
        if (m < n - m && insert_entry_in_new_leaf) {
            ++m;
        }
        s_n = n - m;
        for (int i = m; i < n; ++i) {
            Entry* to = (Entry*)__get_entry(s_header->entries, i - m);
            Entry* from = (Entry*)__get_entry(entries, i);
            memcpy(to, from, entrySize);
        }
        ((Entry*)__get_entry(s_header->entries, s_n))->pageNum =
            ((Entry*)__get_entry(entries, n))->pageNum;
        n = m;
        ((Entry*)__get_entry(header->entries, n))->pageNum = *splitNode;
        if (__cmp(((Entry*)__get_entry(s_header->entries, 0))->key, pData) <= 0) {
            ret = insert_entry(s_header, pData, rid);
        } else {
            ret = insert_entry(header, pData, rid);
        }
        TRY(pfHandle.MarkDirty(*splitNode));
        TRY(pfHandle.UnpinPage(*splitNode));
    }

    TRY(pfHandle.MarkDirty(nodeNum));
    TRY(pfHandle.UnpinPage(nodeNum));
    return ret;
}

RC IX_IndexHandle::InsertEntry(void *pData, const RID &rid) {
    PF_PageHandle ph;
    IX_PageHeader *root_header;
    TRY(pfHandle.GetThisPage(root, ph));
    TRY(ph.GetData(CVOID(root_header)));
    short type = root_header->type;
    TRY(pfHandle.UnpinPage(root));
    int splitNode;
    std::unique_ptr<char[]> splitKey;
    if (type == kInternalNode) {
        TRY(insert_internal(root, &splitNode, &splitKey, pData, rid));
    } else {
        TRY(insert_leaf(root, &splitNode, pData, rid));
    }
    if (splitNode != kNullNode) {
        /**     p (new root)
         *     / \
         *    /   \
         *  root splitNode
         */
        int p;
        TRY(new_node(&p));
        PF_PageHandle p_ph;
        IX_PageHeader *p_header;
        TRY(pfHandle.GetThisPage(p, p_ph));
        TRY(p_ph.GetData(CVOID(p_header)));
        PF_PageHandle s_ph;
        IX_PageHeader *s_header;
        TRY(pfHandle.GetThisPage(splitNode, s_ph));
        TRY(s_ph.GetData(CVOID(s_header)));

        p_header->type = kInternalNode;
        p_header->childrenNum = 2;
        Entry* p_entry_0 = (Entry*)__get_entry(p_header->entries, 0);
        Entry* p_entry_1 = (Entry*)__get_entry(p_header->entries, 1);
        p_entry_0->pageNum = root;
        if (splitKey) {
            memcpy(p_entry_0->key,
                    splitKey.get(), attrLength);
        } else {
            memcpy(p_entry_0->key,
                    ((Entry*)__get_entry(s_header->entries, 0))->key, attrLength);
        }
        p_entry_1->pageNum = splitNode;

        TRY(pfHandle.UnpinPage(splitNode));
        TRY(pfHandle.MarkDirty(p));
        TRY(pfHandle.UnpinPage(p));

        root = p;
        isHeaderDirty = true;
    }
    return 0;
}

RC IX_IndexHandle::DeleteEntry(void *pData, const RID &rid) {
    PF_PageHandle page;
    IX_PageHeader *header;
    int currentNodeNum = root;
    bool should_stop = false;
    int ret = 0;
    while (!should_stop) {
        TRY(pfHandle.GetThisPage(currentNodeNum, page));
        int openedPageNum = currentNodeNum;
        TRY(page.GetData(CVOID(header)));
        if (header->type == kLeafNode) {
            should_stop = true;
            for (int i = 0; i < header->childrenNum; ++i) {
                Entry* entry = (Entry*)__get_entry(header->entries, i);
                int c = __cmp(entry->key, pData);
                if (c == 0) {
                    ret = bucket_delete(&entry->pageNum, rid);
                    TRY(pfHandle.MarkDirty(openedPageNum));
                    break;
                } else if (c > 0) {
                    ret = IX_ENTRY_DOES_NOT_EXIST;
                    break;
                }
            }
        } else {
            int index = header->childrenNum - 1;
            for (int i = 0; i < header->childrenNum - 1; ++i) {
                Entry* entry = (Entry*)__get_entry(header->entries, i);
                if (__cmp(entry->key, pData) > 0) {
                    index = i;
                    break;
                }
            }
            currentNodeNum = ((Entry*)__get_entry(header->entries, index))->pageNum;
        }
        TRY(pfHandle.UnpinPage(openedPageNum));
    }
    return ret;
}

RC IX_IndexHandle::ForcePages() {
    pfHandle.ForcePages();
    // rmHandle.ForcePages();
    return 0;
}

RC IX_IndexHandle::Traverse(int nodeNum, int depth) {
    if (nodeNum <= 0) {
        nodeNum = root;
    }
    PF_PageHandle page;
    IX_PageHeader *header;
    TRY(pfHandle.GetThisPage(nodeNum, page));
    TRY(page.GetData(CVOID(header)));
    for (int i = 0; i < depth; ++i) {
        printf("  ");
    }
    printf("[%d] %d ", nodeNum, header->childrenNum);
    if (header->type == kInternalNode) {
        printf("I");
        for (int i = 0; i < header->childrenNum - 1; ++i) {
            Entry* entry = (Entry*)__get_entry(header->entries, i);
            printf(" p:%d k:%d", entry->pageNum, *(int*)&entry->key);
        }
        printf(" p:%d\n", (((Entry*)__get_entry(header->entries, header->childrenNum - 1))->pageNum));
        for (int i = 0; i < header->childrenNum - 1; ++i) {
            Entry* entry = (Entry*)__get_entry(header->entries, i);
            Traverse(entry->pageNum, depth + 1);
        }
        Traverse(((Entry*)__get_entry(header->entries, header->childrenNum - 1))->pageNum, depth + 1);
    } else {
        printf("L");
        for (int i = 0; i < header->childrenNum; ++i) {
            Entry* entry = (Entry*)__get_entry(header->entries, i);
            printf(" p:%d k:%d", entry->pageNum, *(int*)&entry->key);
        }
        printf(" >:%d\n", (((Entry*)__get_entry(header->entries, header->childrenNum))->pageNum));
    }
    TRY(pfHandle.UnpinPage(nodeNum));
    return 0;
}

