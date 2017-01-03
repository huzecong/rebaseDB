//
// Created by Kanari on 2016/12/28.
//

#ifndef REBASE_QL_INTERNAL_H
#define REBASE_QL_INTERNAL_H

#include "parser.h"
#include "printer.h"

#include <map>
#include <string>
#include <memory>

typedef std::vector<DataAttrInfo> AttrList;

typedef std::pair<std::string, std::string> AttrTag;

template <typename T>
using AttrMap = std::map<AttrTag, T>;

inline bool operator ==(const DataAttrInfo &lhs, const DataAttrInfo &rhs) {
    return !strcmp(lhs.relName, rhs.relName) && !strcmp(lhs.attrName, rhs.attrName);
}

inline bool operator ==(const Value &lhs, const Value &rhs) {
    return lhs.type == rhs.type && lhs.data == rhs.data;
}

struct QL_Condition {
    DataAttrInfo lhsAttr;
    CompOp op;
    bool bRhsIsAttr;
    DataAttrInfo rhsAttr;
    Value rhsValue;

    inline bool operator ==(const QL_Condition &rhs) const {
        return lhsAttr == rhs.lhsAttr &&
               op == rhs.op &&
               bRhsIsAttr == rhs.bRhsIsAttr &&
               ((bRhsIsAttr && rhsAttr == rhs.rhsAttr) ||
                (!bRhsIsAttr && rhsValue == rhs.rhsValue));
    }

    friend std::ostream &operator <<(std::ostream &os, const QL_Condition &condition);
};

#endif //REBASE_QL_INTERNAL_H
