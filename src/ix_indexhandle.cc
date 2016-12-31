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
    b = (PF_PAGE_SIZE - offsetof(Entry, key) - sizeof(IX_PageHeader)) / entrySize + 1;
}

RC IX_IndexHandle::new_page(int *nodeNum) {
    LOG(FATAL) << "not implemented yet";
}

RC IX_IndexHandle::delete_page(int nodeNum) {
    LOG(FATAL) << "not implemented yet";
}

RC IX_IndexHandle::insert_internal(int nodeNum, void *pData, const RID &rid) {
    LOG(FATAL) << "not implemented yet";
}

RC IX_IndexHandle::insert_leaf(int nodeNum, void *pData, const RID &rid) {
    PF_PageHandle ph;
    IX_PageHeader *header;
    TRY(pfHandle.GetThisPage(nodeNum, ph));
    TRY(ph.GetData(CVOID(header)));
    char* entries = ((char*)header) + sizeof(IX_PageHeader);
    short &n = header->childrenNum;
    int ret = 0;
    if (n != this->b - 2) {
        // not full yet, search and insert
        int index = n; // the index of entry to insert AT
                       // the default value for the case where pData is greater than
                       // all the keys
        for (int i = 0; i < n; ++i) {
            Entry* entry = (Entry*)__get_entry(entries, i);
            int c = __cmp(entry->key, pData);
            if (c == 0) {
                ret = IX_KEY_EXISTS;
                goto insert_leaf_ret;
            }
            if (c > 0) {
                index = i;
                break;
            }
        }

        // copy the link to next leaf node
        Entry* old_link = (Entry*)__get_entry(entries, n);
        Entry* new_link = (Entry*)__get_entry(entries, n + 1);
        // LOG(INFO) << "old_link offset = " << ((int*)(&old_link->pageNum)) - ((int*)header);
        // LOG(INFO) << "new_link offset = " << ((int*)(&new_link->pageNum)) - ((int*)header);
        new_link->pageNum = old_link->pageNum;
        // for (int i = 0; i < 10; ++i) {
            // LOG(INFO) << "dump " << i << " = " << ((int*)header)[i];
        // }
        // move the existing entries
        for (int i = n; i > index; --i) {
            Entry* to = (Entry*)__get_entry(entries, i);
            Entry* from = (Entry*)__get_entry(entries, i - 1);
            memcpy(to, from, entrySize); // NOTE: sizeof(Entry) won't work here
        }

        Entry* dest = (Entry*)__get_entry(entries, index);
        TRY(rid.GetPageNum(dest->pageNum));
        TRY(rid.GetSlotNum(dest->slotNum));
        memcpy(dest->key, pData, attrLength);
        ++n;
    } else {
        LOG(FATAL) << "unhandled";
    }

insert_leaf_ret:
    TRY(pfHandle.MarkDirty(nodeNum));
    TRY(pfHandle.UnpinPage(nodeNum));
    return ret;
}

RC IX_IndexHandle::InsertEntry(void *pData, const RID &rid) {
    PF_PageHandle ph;
    IX_PageHeader *root;
    TRY(pfHandle.GetThisPage(1, ph));
    TRY(ph.GetData(CVOID(root)));
    IX_NodeType type = root->type;
    TRY(pfHandle.UnpinPage(1));
    if (type == kInternalNode) {
        insert_internal(1, pData, rid);
    } else {
        insert_leaf(1, pData, rid);
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
