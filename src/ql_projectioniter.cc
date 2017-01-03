//
// Created by Kanari on 2017/1/4.
//

#include "ql_iterator.h"

QL_ProjectionIterator::QL_ProjectionIterator(QL_Iterator *iter, const AttrList &projectFrom, const AttrList &projectTo)
        : QL_Iterator(), inputIter(iter), projectFrom(projectFrom), projectTo(projectTo) {
    projectedSize = 0;
    nullableNum = 0;
    for (auto info : projectTo) {
        projectedSize += upper_align<4>(info.attrSize);
        if (!(info.attrSpecs & ATTR_SPEC_NOTNULL)) ++nullableNum;
    }
    data = new char[projectedSize];
    isnull = new bool[nullableNum];
}

QL_ProjectionIterator::~QL_ProjectionIterator() {
    delete[] data;
    delete[] isnull;
}

RC QL_ProjectionIterator::GetNextRec(RM_Record &rec) {
    RM_Record input;
    char *inputData;
    bool *inputIsnull;
    TRY(inputIter->GetNextRec(input));
    TRY(input.GetData(inputData));
    TRY(input.GetIsnull(inputIsnull));
    for (int i = 0; i < projectFrom.size(); ++i) {
        memcpy(data + projectTo[i].offset, inputData + projectFrom[i].offset, (size_t)projectTo[i].attrSize);
        if (!(projectFrom[i].attrSpecs & ATTR_SPEC_NOTNULL))
            isnull[projectTo[i].nullableIndex] = inputIsnull[projectFrom[i].nullableIndex];
    }
    rec.SetData(data, projectedSize);
    rec.SetIsnull(isnull, nullableNum);
    return 0;
}

RC QL_ProjectionIterator::Reset() {
    return inputIter->Reset();
}

void QL_ProjectionIterator::Print() {
    inputIter->Print();
    std::cout << id << ": ";
    std::cout << "PROJECTION";
    for (auto proj : projectTo)
        std::cout << " " << proj.attrName;
    std::cout << std::endl;
}
