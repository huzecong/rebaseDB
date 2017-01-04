//
// Created by Kanari on 2017/1/3.
//

#ifndef REBASE_QL_ITERATOR_H
#define REBASE_QL_ITERATOR_H


#include "redbase.h"
#include "rm.h"
#include "ix.h"
#include "ql.h"
#include "ql_internal.h"

class QL_Iterator {
    static int totalIters;
protected:
    int id;
    static RM_Manager *rmm;
    static IX_Manager *ixm;
    void editPrefix(std::string &prefix);
public:
    QL_Iterator() {
        id = ++totalIters;
    }

    static void setRM(RM_Manager *rmm) {
        QL_Iterator::rmm = rmm;
    }

    static void setIX(IX_Manager *ixm) {
        QL_Iterator::ixm = ixm;
    }

    int getID() const { return id; }

    virtual RC GetNextRec(RM_Record &rec) = 0;

    virtual RC Reset() = 0;
    virtual void Print(std::string prefix = "") = 0;
};

class QL_FileScanIterator : public QL_Iterator {
    std::string relName;
    RM_FileHandle fileHandle;
    RM_FileScan scan;
public:
    QL_FileScanIterator(std::string relName);

    RC GetNextRec(RM_Record &rec) override;
    RC Reset() override;
    void Print(std::string prefix = "") override;
};

class QL_SelectionIterator : public QL_Iterator {
    QL_Iterator *inputIter;
    std::vector<QL_Condition> conditions;
public:
    QL_SelectionIterator(QL_Iterator *iter, const std::vector<QL_Condition> &conditions);

    RC GetNextRec(RM_Record &rec) override;
    RC Reset() override;
    void Print(std::string prefix = "") override;
};

class QL_ProjectionIterator : public QL_Iterator {
    QL_Iterator *inputIter;
    AttrList projectFrom;
    AttrList projectTo;

    size_t projectedSize;
    short nullableNum;
    char *data;
    bool *isnull;
public:
    QL_ProjectionIterator(QL_Iterator *iter,
                          const AttrList &projectFrom, const AttrList &projectTo);
    virtual ~QL_ProjectionIterator();

    RC GetNextRec(RM_Record &rec) override;
    RC Reset() override;
    void Print(std::string prefix = "") override;
};

class QL_IndexSearchIterator : public QL_Iterator {
    QL_Condition condition;
    RM_FileHandle fileHandle;
    IX_IndexHandle indexHandle;
    IX_IndexScan scan;
public:
    QL_IndexSearchIterator(QL_Condition condition);
    void ChangeValue(char *value);

    RC GetNextRec(RM_Record &rec) override;
    RC Reset() override;
    void Print(std::string prefix = "") override;
};

class QL_NestedLoopJoinIterator : public QL_Iterator {
    QL_Iterator *inputIter1;
    AttrList rel1;
    QL_Iterator *inputIter2;
    AttrList rel2;

    RM_Record rec1;
    RM_Record rec2;
    size_t joinedSize, rec1Size;
    short nullableNum, nullableNum1;
    char *data, *data1, *data2;
    bool *isnull, *isnull1, *isnull2;
public:
    QL_NestedLoopJoinIterator(QL_Iterator *iter1, const AttrList &rel1,
                              QL_Iterator *iter2, const AttrList &rel2);
    virtual ~QL_NestedLoopJoinIterator();

    RC GetNextRec(RM_Record &rec) override;
    RC Reset() override;
    void Print(std::string prefix = "") override;
};

class QL_IndexedJoinIterator : public QL_Iterator {
    QL_Iterator *scanIter;
    AttrList scanRel;
    QL_IndexSearchIterator *indexIter;
    int searchAttrOffset;
    QL_Iterator *searchIter;
    AttrList searchRel;

    RM_Record rec1;
    RM_Record rec2;
    size_t joinedSize, scanSize;
    short nullableNum, nullableNum1;
    char *data, *data1, *data2;
    bool *isnull, *isnull1, *isnull2;
public:
    QL_IndexedJoinIterator(QL_Iterator *scanIter, const AttrList &scanRel,
                           QL_IndexSearchIterator *indexIter, int searchAttrOffset,
                           QL_Iterator *searchIter, const AttrList &searchRel);
    virtual ~QL_IndexedJoinIterator();

    RC GetNextRec(RM_Record &rec) override;
    RC Reset() override;
    void Print(std::string prefix = "") override;
};


#endif //REBASE_QL_ITERATOR_H
