//
// Created by Kanari on 2017/1/4.
//

#include "ql_iterator.h"

QL_FileScanIterator::QL_FileScanIterator(std::string relName)
        : QL_Iterator(), relName(relName) {
    QL_Iterator::rmm->OpenFile(relName.c_str(), fileHandle);
    scan.OpenScan(fileHandle, INT, 4, 0, NO_OP, NULL);
}

RC QL_FileScanIterator::GetNextRec(RM_Record &rec) {
    return scan.GetNextRec(rec);
}

RC QL_FileScanIterator::Reset() {
    TRY(scan.CloseScan());
    TRY(scan.OpenScan(fileHandle, INT, 4, 0, NO_OP, NULL));
    return 0;
}

void QL_FileScanIterator::Print(std::string prefix) {
    std::cout << prefix;
    std::cout << id << ": ";
    std::cout << "SCAN " << relName << std::endl;
}
