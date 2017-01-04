//
// Created by Kanari on 2017/1/4.
//

#include "ql_iterator.h"

QL_SelectionIterator::QL_SelectionIterator(QL_Iterator *iter, const std::vector<QL_Condition> &conditions)
        : QL_Iterator(), inputIter(iter), conditions(conditions) {}

RC QL_SelectionIterator::GetNextRec(RM_Record &rec) {
    char *data;
    bool *isnull;
    while (true) {
        int retcode = inputIter->GetNextRec(rec);
        if (retcode == RM_EOF) return RM_EOF;
        TRY(retcode);
        TRY(rec.GetData(data));
        TRY(rec.GetIsnull(isnull));
        bool ok = true;
        for (int i = 0; i < conditions.size() && ok; ++i)
            ok = checkSatisfy(data, isnull, conditions[i]);
        if (ok) break;
    }
    return 0;
}

RC QL_SelectionIterator::Reset() {
    return inputIter->Reset();
}

void QL_SelectionIterator::Print(std::string prefix) {
    std::cout << prefix;
    std::cout << id << ": ";
    std::cout << "SELECTION";
    for (auto cond : conditions)
        std::cout << " " << cond;
    std::cout << std::endl;
    editPrefix(prefix);
    inputIter->Print(prefix + "└──");
}
