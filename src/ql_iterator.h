//
// Created by Kanari on 2017/1/3.
//

#ifndef REBASE_QL_ITERATOR_H
#define REBASE_QL_ITERATOR_H


#include "redbase.h"
#include "rm.h"
#include "ql_internal.h"

class QL_Iterator {
    static int totalIters;
protected:
    int id;
public:
    QL_Iterator() {
        id = ++totalIters;
    }

    int getID() const { return id; }

    virtual RC GetNextRec(RM_Record &rec) { return 0; };

    virtual RC Reset() { return 0; };
    virtual void Print() = 0;
};

int QL_Iterator::totalIters = 0;

class QL_FileScanIterator : public QL_Iterator {
    std::string relName;
public:
    QL_FileScanIterator(std::string relName)
            : QL_Iterator(), relName(relName) {}

    void Print() {
        std::cout << id << ": ";
        std::cout << "SCAN " << relName << std::endl;
    }
};

class QL_SelectionIterator : public QL_Iterator {
    QL_Iterator *inputIter;
    std::vector<QL_Condition> conditions;
public:
    QL_SelectionIterator(QL_Iterator *iter, const std::vector<QL_Condition> &conditions)
            : QL_Iterator(), inputIter(iter), conditions(conditions) {}

    void Print() {
        inputIter->Print();
        std::cout << id << ": ";
        std::cout << "SELECTION";
        for (auto cond : conditions)
            std::cout << " " << cond;
        std::cout << std::endl;
    }
};

class QL_ProjectionIterator : public QL_Iterator {
    QL_Iterator *inputIter;
    AttrList projectFrom;
    AttrList projectTo;
public:
    QL_ProjectionIterator(QL_Iterator *iter,
                          const AttrList &projectFrom, const AttrList &projectTo)
            : QL_Iterator(), inputIter(iter), projectFrom(projectFrom), projectTo(projectTo) {}

    void Print() {
        inputIter->Print();
        std::cout << id << ": ";
        std::cout << "PROJECTION";
        for (auto proj : projectTo)
            std::cout << " " << proj.attrName;
        std::cout << std::endl;
    }
};

class QL_IndexSearchIterator : public QL_Iterator {
    QL_Condition condition;
public:
    QL_IndexSearchIterator(QL_Condition condition)
            : QL_Iterator(), condition(condition) {}

    void Print() {
        std::cout << id << ": ";
        std::cout << "SEARCH" << " " << condition << std::endl;
    }
};

class QL_NestedLoopJoinIterator : public QL_Iterator {
    QL_Iterator *inputIter1;
    AttrList rel1;
    QL_Iterator *inputIter2;
    AttrList rel2;
public:
    QL_NestedLoopJoinIterator(QL_Iterator *iter1, const AttrList &rel1,
                              QL_Iterator *iter2, const AttrList &rel2)
            : QL_Iterator(), inputIter1(iter1), rel1(rel1), inputIter2(iter2), rel2(rel2) {}

    void Print() {
        inputIter1->Print();
        inputIter2->Print();
        std::cout << id << ": ";
        std::cout << "NESTED-LOOP JOIN " << inputIter1->getID() << " and " << inputIter2->getID() << std::endl;
    }
};

class QL_IndexedJoinIterator : public QL_Iterator {
    QL_Iterator *scanIter;
    AttrList scanRel;
    QL_IndexSearchIterator *indexIter;
    QL_Iterator *searchIter;
    AttrList searchRel;
public:
    QL_IndexedJoinIterator(QL_Iterator *scanIter, const AttrList &scanRel,
                           QL_IndexSearchIterator *indexIter,
                           QL_Iterator *searchIter, const AttrList &searchRel)
            : QL_Iterator(), scanIter(scanIter), scanRel(scanRel), indexIter(indexIter),
              searchIter(searchIter), searchRel(searchRel) {}

    void Print() {
        scanIter->Print();
        searchIter->Print();
        std::cout << id << ": ";
        std::cout << "INDEXED JOIN " << scanIter->getID() << " and " << searchIter->getID() << std::endl;
    }
};


#endif //REBASE_QL_ITERATOR_H
