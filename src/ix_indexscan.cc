//
// Created by Kanari on 2016/12/24.
//

#include "ix.h"
#include "ix_internal.h"

IX_IndexScan::IX_IndexScan() {
    scanOpened = false;
}

IX_IndexScan::~IX_IndexScan() { }

bool IX_IndexScan::__check(void* key) {
#define CMP_RESULT (indexHandle->__cmp(key, this->value))
    switch (compOp) {
        case GT_OP:
        case GE_OP:
            // initial search has done all the work
            return true;
        case EQ_OP:
            return CMP_RESULT == 0;
        case NE_OP:
            return CMP_RESULT != 0;
        case LT_OP:
            return CMP_RESULT < 0;
        case LE_OP:
            return CMP_RESULT <= 0;
        case NO_OP:
            return true;
        case ISNULL_OP:
        case NOTNULL_OP:
            LOG(FATAL) << "null is not supported in IX";
    }
}

RC IX_IndexScan::OpenScan(const IX_IndexHandle &indexHandle, CompOp compOp, void *value, ClientHint pinHint) {
    if (scanOpened) {
        return IX_SCAN_NOT_CLOSED;
    }
    this->indexHandle = &indexHandle;
    this->compOp = compOp;
    this->value = value;
    bool initial_search_needed = false;
    switch (compOp) {
        case GT_OP:
        case GE_OP:
            initial_search_needed = true;
            break;
        case NO_OP:
        case EQ_OP:
        case NE_OP:
        case LT_OP:
        case LE_OP:
            initial_search_needed = false;
            break;
        case ISNULL_OP:
        case NOTNULL_OP:
            LOG(FATAL) << "null is not supported in IX";
    }
    currentNodeNum = indexHandle.root;
    currentEntryIndex = 0;
    const PF_FileHandle &file = indexHandle.pfHandle;
    bool should_stop = false;
    while (!should_stop) {
        PF_PageHandle page;
        IX_PageHeader *header;
        TRY(file.GetThisPage(currentNodeNum, page));
        int openedPageNum = currentNodeNum;
        TRY(page.GetData(CVOID(header)));
        if (header->type == kLeafNode) {
            if (compOp == GT_OP || compOp == GE_OP) {
                for (currentEntryIndex = 0; currentEntryIndex < header->childrenNum;
                        ++currentEntryIndex) {
                    Entry *entry = (Entry*)indexHandle.__get_entry(
                            header->entries, currentEntryIndex);
                    int c = indexHandle.__cmp(entry->key, value);
                    if ((compOp == GT_OP && c > 0) ||
                            (compOp == GE_OP && c >= 0)) {
                        break;
                    }
                }
            }
            should_stop = true;
        } else {
            int index = 0;
            if (compOp == GT_OP || compOp == GE_OP) {
                index = header->childrenNum;
                for (int i = 0; i < header->childrenNum; ++i) {
                    if (indexHandle.__cmp(((Entry*)indexHandle.__get_entry(
                                        header->entries, i))->key, value) > 0) {
                        index = i;
                        break;
                    }
                }
            }
            currentNodeNum = ((Entry*)indexHandle.__get_entry(
                        header->entries, index))->pageNum;
        }
        TRY(file.UnpinPage(openedPageNum));
    }
    scanOpened = true;
    return 0;
}

RC IX_IndexScan::GetNextEntry(RID &rid) {
    if (!scanOpened) {
        return IX_SCAN_NOT_OPENED;
    }
    const PF_FileHandle &file = indexHandle->pfHandle;
    PF_PageHandle page;
    int ret = 0;
    bool should_exit = false;
    while (!should_exit) {
        IX_PageHeader* header;
        int openedPageNum = currentNodeNum;
        TRY(file.GetThisPage(currentNodeNum, page));
        TRY(page.GetData(CVOID(header)));
        Entry* entry = (Entry*)indexHandle->__get_entry(header->entries, currentEntryIndex);
        if (currentEntryIndex == header->childrenNum) {
            if (entry->pageNum == kNullNode) {
                ret = IX_EOF;
                should_exit = true;
            } else {
                currentNodeNum = entry->pageNum;
                currentEntryIndex = 0;
            }
        } else {
            if (__check(entry->key)) {
                rid = RID(entry->pageNum, entry->slotNum);
                should_exit = true;
            } else {
                if (compOp == LT_OP || compOp == LE_OP) {
                    ret = IX_EOF;
                    should_exit = true;
                }
            }
            ++currentEntryIndex;
        }
        TRY(file.UnpinPage(openedPageNum));
    }
    return ret;
}

RC IX_IndexScan::CloseScan() {
    scanOpened = false;
    return 0;
}
