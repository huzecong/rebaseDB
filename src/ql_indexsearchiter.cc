//
// Created by Kanari on 2017/1/4.
//

#include "ql_iterator.h"

QL_IndexSearchIterator::QL_IndexSearchIterator(QL_Condition condition)
        : QL_Iterator(), condition(condition) {
    QL_Iterator::rmm->OpenFile(condition.lhsAttr.relName, fileHandle);
    QL_Iterator::ixm->OpenIndex(condition.lhsAttr.relName, condition.lhsAttr.indexNo, indexHandle);
    scan.OpenScan(indexHandle, condition.op, condition.rhsValue.data);
}

void QL_IndexSearchIterator::ChangeValue(char *value) {
    condition.rhsValue.data = value;
}

RC QL_IndexSearchIterator::GetNextRec(RM_Record &rec) {
    RID rid;
    int retcode = scan.GetNextEntry(rid);
    if (retcode == IX_EOF) return RM_EOF;
    TRY(retcode);
    TRY(fileHandle.GetRec(rid, rec));
    return 0;
}

RC QL_IndexSearchIterator::Reset() {
    TRY(scan.CloseScan());
    TRY(scan.OpenScan(indexHandle, condition.op, condition.rhsValue.data));
    return 0;
}

void QL_IndexSearchIterator::Print(std::string prefix) {
    std::cout << prefix;
    std::cout << id << ": ";
    std::cout << "SEARCH" << " " << condition << std::endl;
}
