//
// Created by Kanari on 2017/1/4.
//

#include "ql_iterator.h"

QL_IndexedJoinIterator::QL_IndexedJoinIterator(QL_Iterator *scanIter, const std::vector<DataAttrInfo> &scanRel,
                                               QL_IndexSearchIterator *indexIter, int searchAttrOffset,
                                               QL_Iterator *searchIter, const std::vector<DataAttrInfo> &searchRel)
        : QL_Iterator(), scanIter(scanIter), scanRel(scanRel),
          indexIter(indexIter), searchAttrOffset(searchAttrOffset),
          searchIter(searchIter), searchRel(searchRel) {
    joinedSize = 0;
    nullableNum = 0;
    for (auto info : scanRel) {
        joinedSize += upper_align<4>(info.attrSize);
        if (!(info.attrSpecs & ATTR_SPEC_NOTNULL)) ++nullableNum;
    }
    scanSize = joinedSize;
    nullableNum1 = nullableNum;
    for (auto info : searchRel) {
        joinedSize += upper_align<4>(info.attrSize);
        if (!(info.attrSpecs & ATTR_SPEC_NOTNULL)) ++nullableNum;
    }
    data = new char[joinedSize];
    isnull = new bool[nullableNum];

    scanIter->GetNextRec(rec1);
    rec1.GetData(data1);
    rec1.GetIsnull(isnull1);
    indexIter->ChangeValue(data1 + searchAttrOffset);
    searchIter->Reset();
}

QL_IndexedJoinIterator::~QL_IndexedJoinIterator() {
    delete[] data;
    delete[] isnull;
}

RC QL_IndexedJoinIterator::GetNextRec(RM_Record &rec) {
    int retcode = searchIter->GetNextRec(rec2);
    while (retcode == RM_EOF) {
        // prevent RM_EOF from triggering error reporting mechanism in TRY
        if (int rc = scanIter->GetNextRec(rec1)) {
            if (rc != RM_EOF) {
                TRY(rc);
            }
            return rc;
        }
        TRY(rec1.GetData(data1));
        TRY(rec1.GetIsnull(isnull1));
        indexIter->ChangeValue(data1 + searchAttrOffset);
        TRY(searchIter->Reset());
        retcode = searchIter->GetNextRec(rec2);
    }
    if (retcode) return retcode;
    TRY(rec2.GetData(data2));
    TRY(rec2.GetIsnull(isnull2));
    memcpy(data, data1, scanSize);
    memcpy(data + scanSize, data2, joinedSize - scanSize);
    memcpy(isnull, isnull1, sizeof(bool) * nullableNum1);
    memcpy(isnull + nullableNum1, isnull2, sizeof(bool) * (nullableNum - nullableNum1));
    rec.SetData(data, joinedSize);
    rec.SetIsnull(isnull, nullableNum);
    return 0;
}

RC QL_IndexedJoinIterator::Reset() {
    TRY(scanIter->Reset());
    TRY(searchIter->Reset());
    TRY(scanIter->GetNextRec(rec1));
    TRY(rec1.GetData(data1));
    TRY(rec1.GetIsnull(isnull1));
    indexIter->ChangeValue(data1 + searchAttrOffset);
    return 0;
}

void QL_IndexedJoinIterator::Print(std::string prefix) {
    std::cout << prefix;
    std::cout << id << ": ";
    std::cout << "INDEXED JOIN " << scanIter->getID() << " and " << searchIter->getID() << std::endl;
    editPrefix(prefix);
    scanIter->Print(prefix + "├──");
    searchIter->Print(prefix + "└──");
}
