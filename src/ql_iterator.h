//
// Created by Kanari on 2017/1/3.
//

#ifndef REBASE_QL_ITERATOR_H
#define REBASE_QL_ITERATOR_H


#include "redbase.h"
#include "rm.h"
#include "ql_internal.h"

class QL_Iterator {
public:
    virtual RC GetNextRec(RM_Record &rec) = 0;
    virtual RC Reset() = 0;
    virtual void Print() = 0;
};

class QL_FileScanIterator : public QL_Iterator {
public:
    QL_FileScanIterator(std::string relName);
};

class QL_SelectionIterator : public QL_Iterator {
public:
    QL_SelectionIterator(QL_Iterator *iter, std::vector<QL_Condition> conditions);
};

class QL_ProjectionIterator : public QL_Iterator {
public:
    QL_ProjectionIterator(QL_Iterator *iter, std::vector<DataAttrInfo> from, std::vector<DataAttrInfo> to);
};

class QL_IndexSearchIterator : public QL_Iterator {
public:
    QL_IndexSearchIterator(QL_Condition condition);
};

class QL_NestedLoopJoinIterator : public QL_Iterator {
public:
    QL_NestedLoopJoinIterator(QL_Iterator *iter1, QL_Iterator *iter2);
};

class QL_IndexedJoinIterator : public QL_Iterator {
public:
    QL_IndexedJoinIterator(QL_Iterator *scanIter, QL_IndexSearchIterator *searchIter);
};


#endif //REBASE_QL_ITERATOR_H
