//
// Created by Kanari on 2016/12/24.
//

#include "ix.h"
#include "ix_internal.h"

#include <stddef.h>

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
            return IX_KEY_EXISTS;
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
    TRY(rid.GetPageNum(dest->pageNum));
    TRY(rid.GetSlotNum(dest->slotNum));
    memcpy(dest->key, pData, attrLength);
    ++n;
    return 0;
}

RC IX_IndexHandle::insert_internal(int nodeNum, int *splitNode, void *pData, const RID &rid) {
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
    if (c_type == kInternalNode) {
        TRY(insert_internal(child, &lowerSplitNode, pData, rid));
    } else {
        TRY(insert_leaf(child, &lowerSplitNode, pData, rid));
    }
    if (lowerSplitNode != kNullNode) {
        LOG(INFO) << "insert_internal: lower split";
        PF_PageHandle s_ph;
        IX_PageHeader *s_header;
        TRY(pfHandle.GetThisPage(lowerSplitNode, s_ph));
        TRY(s_ph.GetData(CVOID(s_header)));
        char* s_key = ((Entry*)__get_entry(s_header->entries, 0))->key;
        if (n != this->b) {
            int index = n - 1;
            for (int i = 0; i < n - 1; ++i) {
                if (__cmp(((Entry*)__get_entry(entries, i))->key, s_key) > 0) {
                    index = i;
                    break;
                }
            }
            if (index != n - 1) {
                char* src = ((Entry*)__get_entry(entries, index + 1))->key;
                memmove(src + entrySize, src,
                        (char*)__get_entry(entries, n) - src);
            }
            memcpy(((Entry*)__get_entry(entries, index))->key,
                    s_key, attrLength);
            ((Entry*)__get_entry(entries, index + 1))->pageNum = lowerSplitNode;
            ++n;
        } else {
            LOG(FATAL) << "unhandled";
        }
        TRY(pfHandle.UnpinPage(lowerSplitNode));
    }

insert_internal_ret:
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
            (__cmp(((Entry*)__get_entry(header, m))->key, pData) <= 0);
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
        if (__cmp(((Entry*)__get_entry(s_header, 0))->key, pData) <= 0) {
            ret = insert_entry(s_header, pData, rid);
        } else {
            ret = insert_entry(header, pData, rid);
        }
        TRY(pfHandle.MarkDirty(*splitNode));
        TRY(pfHandle.UnpinPage(*splitNode));
    }

insert_leaf_ret:
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
    if (type == kInternalNode) {
        TRY(insert_internal(root, &splitNode, pData, rid));
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
        memcpy(p_entry_0->key,
                ((Entry*)__get_entry(s_header->entries, 0))->key, attrLength);
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
    return 0;
}

RC IX_IndexHandle::ForcePages() {
    pfHandle.ForcePages();
    // rmHandle.ForcePages();
    return 0;
}
