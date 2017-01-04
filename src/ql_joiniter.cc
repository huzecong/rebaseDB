//
// Created by Kanari on 2017/1/4.
//

#include "ql_iterator.h"

QL_NestedLoopJoinIterator::QL_NestedLoopJoinIterator(QL_Iterator *iter1, const AttrList &rel1, QL_Iterator *iter2, const AttrList &rel2)
        : QL_Iterator(), inputIter1(iter1), rel1(rel1), inputIter2(iter2), rel2(rel2) {
    joinedSize = 0;
    nullableNum = 0;
    for (auto info : rel1) {
        joinedSize += upper_align<4>(info.attrSize);
        if (!(info.attrSpecs & ATTR_SPEC_NOTNULL)) ++nullableNum;
    }
    rec1Size = joinedSize;
    nullableNum1 = nullableNum;
    for (auto info : rel2) {
        joinedSize += upper_align<4>(info.attrSize);
        if (!(info.attrSpecs & ATTR_SPEC_NOTNULL)) ++nullableNum;
    }
    data = new char[joinedSize];
    isnull = new bool[nullableNum];

    inputIter1->GetNextRec(rec1);
    rec1.GetData(data1);
    rec1.GetIsnull(isnull1);
}

QL_NestedLoopJoinIterator::~QL_NestedLoopJoinIterator() {
    delete[] data;
    delete[] isnull;
}

RC QL_NestedLoopJoinIterator::GetNextRec(RM_Record &rec) {
    int retcode = inputIter2->GetNextRec(rec2);
    while (retcode == RM_EOF) {
        if (int rc = inputIter1->GetNextRec(rec1)) {
            if (rc == RM_EOF) return RM_EOF;
            TRY(rc);
        }
        TRY(rec1.GetData(data1));
        TRY(rec1.GetIsnull(isnull1));
        TRY(inputIter2->Reset());
        retcode = inputIter2->GetNextRec(rec2);
    }
    TRY(retcode);
    TRY(rec2.GetData(data2));
    TRY(rec2.GetIsnull(isnull2));
    memcpy(data, data1, rec1Size);
    memcpy(data + rec1Size, data2, joinedSize - rec1Size);
    memcpy(isnull, isnull1, sizeof(bool) * nullableNum1);
    memcpy(isnull + nullableNum1, isnull2, sizeof(bool) * (nullableNum - nullableNum1));
    rec.SetData(data, joinedSize);
    rec.SetIsnull(isnull, nullableNum);
    return 0;
}

RC QL_NestedLoopJoinIterator::Reset() {
    TRY(inputIter1->Reset());
    TRY(inputIter2->Reset());
    TRY(inputIter1->GetNextRec(rec1));
    TRY(rec1.GetData(data1));
    TRY(rec1.GetIsnull(isnull1));
    return 0;
}

void QL_NestedLoopJoinIterator::Print(std::string prefix) {
    std::cout << prefix;
    std::cout << id << ": ";
    std::cout << "NESTED-LOOP JOIN " << inputIter1->getID() << " and " << inputIter2->getID() << std::endl;
    editPrefix(prefix);
    inputIter1->Print(prefix + "├──");
    inputIter2->Print(prefix + "└──");
}
