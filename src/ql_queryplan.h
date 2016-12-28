//
// Created by Kanari on 2016/12/28.
//

#ifndef REBASE_QL_QUERYPLAN_H
#define REBASE_QL_QUERYPLAN_H

struct QL_Condition {
    DataAttrInfo lhsAttr;
    CompOp op;
    bool bRhsIsAttr;
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
    
    std::string attrName;
    std::vector<QL_Condition> conds;
    std::vector<std::string> projection;
    typedef std::vector<QL_QueryPlan> InnerLoopType;
    std::shared_ptr<InnerLoopType> innerLoop;
    
    std::string tempSaveName;
};

#endif //REBASE_QL_QUERYPLAN_H
