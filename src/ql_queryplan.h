//
// Created by Kanari on 2016/12/28.
//

#ifndef REBASE_QL_QUERYPLAN_H
#define REBASE_QL_QUERYPLAN_H

typedef std::pair<std::string, std::string> AttrTag;

template <typename T>
using AttrMap = std::map<AttrTag, T>;

struct AttrRecordInfo {
    AttrTag tag;
    int offset;                // Offset of attribute
    AttrType attrType;         // Type of attribute
    int attrLength;            // Length of attribute
    int attrSpecs;             // Attribute specifications
    int indexNo;               // Index number of attribute

    AttrRecordInfo() = default;

    AttrRecordInfo(const DataAttrInfo &info) {
        tag = AttrTag(info.relName, info.attrName);
        offset = info.offset;
        attrType = info.attrType;
        attrLength = info.attrLength;
        attrSpecs = info.attrSpecs;
        indexNo = info.indexNo;
    }
};

struct QL_Condition {
//    AttrRecordInfo lhsAttr;
    DataAttrInfo lhsAttr;
    CompOp op;
    bool bRhsIsAttr;
//    AttrRecordInfo rhsAttr;
    DataAttrInfo rhsAttr;
    Value rhsValue;

    friend std::ostream &operator <<(std::ostream &os, const QL_Condition &condition);
};

enum QueryPlanType {
    QP_SCAN,
    QP_AUTOINDEX,
    QP_SEARCH,
    QP_FINAL
};

struct QL_QueryPlan {
    QueryPlanType type;
    std::string relName;

    std::string indexAttrName;
    QL_Condition indexCondition;
    std::vector<QL_Condition> conditions;
    std::vector<std::string> projection;
    std::shared_ptr<QL_QueryPlan> innerLoop;

    std::string tempSaveName;
};

#endif //REBASE_QL_QUERYPLAN_H
