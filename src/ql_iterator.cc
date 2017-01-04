//
// Created by Kanari on 2017/1/4.
//

#include "ql_iterator.h"

int QL_Iterator::totalIters = 0;

void QL_Iterator::editPrefix(std::string &prefix) {
    if (prefix.length() > 0) {
        if (prefix.substr(prefix.length() - 9, 3) == "├") {
            prefix = prefix.substr(0, prefix.length() - 9) + "│  ";
        } else {
            prefix = prefix.substr(0, prefix.length() - 9) + "   ";
        }
    }
}

RM_Manager *QL_Iterator::rmm;
IX_Manager *QL_Iterator::ixm;
