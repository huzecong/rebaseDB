//
// Created by Kanari on 2016/12/24.
//

#include <set>
#include <cassert>
#include "ql.h"
#include "ql_graph.h"

QL_Manager::QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm) {
    pSmm = &smm;
    pIxm = &ixm;
    pRmm = &rmm;
}

QL_Manager::~QL_Manager() {
}

static bool can_assign_to(AttrType rt, ValueType vt, bool nullable) {
    return (vt == VT_NULL && nullable) ||
           (vt == VT_INT && rt == INT) ||
           (vt == VT_INT && rt == FLOAT) ||
           (vt == VT_FLOAT && rt == FLOAT) ||
           (vt == VT_STRING && rt == STRING);
}

inline AttrTag make_tag(const RelAttr &info) {
    return AttrTag(info.relName ? std::string(info.relName) : "",
                   std::string(info.attrName));
};

#define DEFINE_ATTRINFO(_name, _key) \
    auto __iter__##_name = attrMap.find(_key); \
    if (__iter__##_name == attrMap.end()) return QL_ATTR_NOTEXIST; \
    DataAttrInfo &_name = __iter__##_name->second;

RC QL_Manager::Select(int nSelAttrs, const RelAttr *selAttrs, int nRelations, const char *const *relations, int nConditions, const Condition *conditions) {
    // open files
    ARR_PTR(fileHandles, RM_FileHandle, nRelations);
    for (int i = 0; i < nRelations; ++i)
        TRY(pRmm->OpenFile(relations[i], fileHandles[i]));
    VLOG(1) << "files opened";
    
    /**
     * Check if query is valid
     */
    // create mappings of attribute names to corresponding info
    ARR_PTR(attrInfo, std::vector<DataAttrInfo>, nRelations);
    ARR_PTR(attrCount, int, nRelations);
    std::map<std::string, int> attrNameCount;
    AttrMap<DataAttrInfo> attrMap;
    for (int i = 0; i < nRelations; ++i) {
        TRY(pSmm->GetDataAttrInfo(relations[i], attrCount[i], attrInfo[i], true));
        std::string relName(relations[i]);
        for (int j = 0; j < attrCount[i]; ++j) {
            std::string attrName(attrInfo[i][j].attrName);
            attrMap[std::make_pair(relName, attrName)] = attrInfo[i][j];
            ++attrNameCount[attrName];
        }
    }
    for (int i = 0; i < nRelations; ++i)
        for (int j = 0; j < attrCount[i]; ++j) {
            std::string attrName(attrInfo[i][j].attrName);
            if (attrNameCount[attrName] == 1)
                attrMap[std::make_pair(std::string(), attrName)] = attrInfo[i][j];
        }
    VLOG(1) << "attribute name mapping created";
              
    // check selected attributes exist
    if (nSelAttrs == 1 && !strcmp(selAttrs[0].attrName, "*"))
        nSelAttrs = 0;
    for (int i = 0; i < nSelAttrs; ++i) {
        DEFINE_ATTRINFO(_, make_tag(selAttrs[i]));
    }
    VLOG(1) << "all attributes exist";
    
    // check conditions are valid
    for (int i = 0; i < nConditions; ++i) {
        DEFINE_ATTRINFO(lhsAttr, make_tag(conditions[i].lhsAttr));
        bool nullable = ((lhsAttr.attrSpecs & ATTR_SPEC_NOTNULL) == 0);
        if (conditions[i].bRhsIsAttr) {
            DEFINE_ATTRINFO(rhsAttr, make_tag(conditions[i].rhsAttr));
            if (lhsAttr.attrType != rhsAttr.attrType)
                return QL_ATTR_TYPES_MISMATCH;
        } else {
            if (!can_assign_to(lhsAttr.attrType, conditions[i].rhsValue.type, nullable))
                return QL_VALUE_TYPES_MISMATCH;
        }
    }
    VLOG(1) << "all conditions are valid";
    
    /**
     * Generate query plan
     */
    std::vector<QL_QueryPlan> queryPlans;
    
    std::map<std::string, int> relNumMap;
    for (int i = 0; i < nRelations; ++i)
        relNumMap[std::string(relations[i])] = i;
    
    // build target projections
    ARR_PTR(targetProjections, std::vector<std::string>, nRelations);
    if (nSelAttrs == 0) {
        for (int i = 0; i < nRelations; ++i)
            for (int j = 0; j < attrCount[i]; ++j)
                targetProjections[i].push_back(std::string(attrInfo[i][j].attrName));
    } else {
        for (int i = 0; i < nSelAttrs; ++i) {
            DataAttrInfo &info = attrMap[make_tag(selAttrs[i])];
            targetProjections[relNumMap[std::string(info.relName)]].push_back(std::string(info.attrName));
        }
    }
    VLOG(1) << "target projections built";
    
    // gather simple conditions and projections for each table
    ARR_PTR(simpleConditions, std::vector<QL_Condition>, nRelations);
    std::vector<QL_Condition> complexConditions;
    ARR_PTR(simpleProjections, std::set<std::string>, nRelations);
    for (int i = 0; i < nRelations; ++i) {
        std::string relName(relations[i]);
        for (auto attrName : targetProjections[i])
            simpleProjections[i].insert(attrName);
    }
    for (int i = 0; i < nConditions; ++i) {
        DataAttrInfo &lhsAttr = attrMap[make_tag(conditions[i].lhsAttr)];
        int lhsAttrNum = relNumMap[std::string(lhsAttr.relName)];
        QL_Condition cond;
        cond.lhsAttr = lhsAttr;
        cond.op = conditions[i].op;
        cond.bRhsIsAttr = (bool)conditions[i].bRhsIsAttr;
        if (conditions[i].bRhsIsAttr) {
            DataAttrInfo &rhsAttr = attrMap[make_tag(conditions[i].rhsAttr)];
            cond.rhsAttr = rhsAttr;
            if (!strcmp(lhsAttr.relName, rhsAttr.relName)) {
                simpleConditions[lhsAttrNum].push_back(cond);
            } else {
                complexConditions.push_back(cond);
            }
            int rhsAttrNum = relNumMap[std::string(rhsAttr.relName)];
            simpleProjections[lhsAttrNum].insert(std::string(lhsAttr.attrName));
            simpleProjections[rhsAttrNum].insert(std::string(rhsAttr.attrName));
        } else {
            cond.rhsValue = conditions[i].rhsValue;
            simpleConditions[lhsAttrNum].push_back(cond);
            simpleProjections[lhsAttrNum].insert(std::string(lhsAttr.attrName));
        }
    }
    VLOG(1) << "simple conditions and projections gathered";
    
    // select and projections related to single relations
    ARR_PTR(filteredRefName, std::string, nRelations);
    for (int i = 0; i < nRelations; ++i) {
        std::string relName(relations[i]);
        int relNum = relNumMap[relName];
        auto &relCond = simpleConditions[relNum];
        if (relCond.size() > 0) {
            filteredRefName[i] = pSmm->GenerateTempTableName(relName);
            QL_QueryPlan plan;
            plan.type = QP_SCAN;
            plan.relName = relName;
            plan.tempSaveName = filteredRefName[i];
            plan.conds = relCond;
            for (auto attrName : simpleProjections[relNum])
                plan.projection.push_back(attrName);
            queryPlans.push_back(plan);
        } else {
            filteredRefName[i] = std::string(relations[i]);
        }
    }
    VLOG(1) << "query plan part 1 (simple statements) generated";
    
    // part relations related by conditions and join separately
    QL_Graph graph(nRelations);
    ARR_PTR(relatedConditions, std::vector<QL_Condition>, nRelations);
    for (auto condition : complexConditions) {
        int a = relNumMap[std::string(condition.lhsAttr.relName)];
        int b = relNumMap[std::string(condition.rhsAttr.relName)];
        if (a > b) std::swap(a, b);
        graph.insertEdge(a, b);
        relatedConditions[b].push_back(condition);
    }
    VLOG(1) << "relation graph built";
    
    std::vector<std::string> descartesProductRelations;
    for (auto block : graph) {
        QL_QueryPlan root, child;
        root.type = QP_SCAN;
        root.relName = filteredRefName[block[0]];
        root.projection = targetProjections[block[0]];
        root.tempSaveName = pSmm->GenerateTempTableName("joined_block");
        auto innerLoop = std::shared_ptr<QL_QueryPlan::InnerLoopType>(nullptr);
        for (int i = (int)block.size() - 1; i > 0; --i) {
            child.type = QP_SCAN;
            child.relName = filteredRefName[block[i]];
            child.projection = targetProjections[block[i]];
            child.conds = relatedConditions[block[i]];
            child.innerLoop = innerLoop;
            innerLoop = std::make_shared<QL_QueryPlan::InnerLoopType>();
            innerLoop.get()->push_back(child);
        }
        root.innerLoop = innerLoop;
        descartesProductRelations.push_back(root.tempSaveName);
        queryPlans.push_back(root);
    }
    VLOG(1) << "query plan part 2 (non-descartes-product-joins) generated";
    
    // join unrelated relations (unfiltered descartes product)
    std::string finalRelName = descartesProductRelations[0];
    if (descartesProductRelations.size() > 1) {
        QL_QueryPlan root, child;
        root.type = QP_SCAN;
        root.relName = descartesProductRelations[0];
        root.tempSaveName = pSmm->GenerateTempTableName("final");
        finalRelName = root.tempSaveName;
        auto innerLoop = std::shared_ptr<QL_QueryPlan::InnerLoopType>(nullptr);
        for (int i = (int)descartesProductRelations.size() - 1; i > 0; --i) {
            child.type = QP_SCAN;
            child.relName = descartesProductRelations[i];
            child.innerLoop = innerLoop;
            innerLoop = std::make_shared<QL_QueryPlan::InnerLoopType>();
            innerLoop.get()->push_back(child);
        }
        root.innerLoop = innerLoop;
        queryPlans.push_back(root);
    }
    VLOG(1) << "query plan part 3 (descartes product join) generated";
    
    QL_QueryPlan final;
    final.type = QP_FINAL;
    final.relName = finalRelName;
    queryPlans.push_back(final);
    
    // print and execute plan
    if (bQueryPlans) {
        for (auto plan : queryPlans) {
            TRY(PrintQueryPlan(plan));
            std::cout << std::endl;
        }
    }
    VLOG(1) << "query plan printed";
//    for (auto plan : queryPlans)
//        TRY(ExecuteQueryPlan(plan, attrMap, AttrMap<Value>));
    
    return 0;
}

#undef DEFINE_ATTRINFO

std::ostream &operator <<(std::ostream &os, const QL_Condition &condition) {
    if (condition.op == NO_OP) {
        os << "*";
    } else {
        os << condition.lhsAttr.relName << "." << condition.lhsAttr.attrName;
        os << " ";
        if (condition.op == ISNULL_OP || condition.op == NOTNULL_OP) {
            if (condition.op == ISNULL_OP) os << "is null";
            else os << "is not null";
        } else {
            switch (condition.op) {
                case EQ_OP:
                    os << "=";
                    break;
                case NE_OP:
                    os << "!=";
                    break;
                case LT_OP:
                    os << "<";
                    break;
                case GT_OP:
                    os << ">";
                    break;
                case LE_OP:
                    os << "<=";
                    break;
                case GE_OP:
                    os << ">=";
                    break;
                default:
                    break;
            }
            os << " ";
            if (condition.bRhsIsAttr) {
                os << condition.rhsAttr.relName << "." << condition.rhsAttr.attrName;
            } else {
                switch (condition.rhsValue.type) {
                    case VT_INT:
                        os << *(int *)condition.rhsValue.data;
                        break;
                    case VT_FLOAT:
                        os << *(float *)condition.rhsValue.data;
                        break;
                    case VT_STRING:
                        os << (char *)condition.rhsValue.data;
                        break;
                    default:
                        break;
                }
            }
        }
    }
    return os;
}

RC QL_Manager::PrintQueryPlan(const QL_QueryPlan &queryPlan, int indent) {
    std::string prefix = "";
    prefix.append((unsigned long)indent, ' ');
    switch (queryPlan.type) {
        case QP_SCAN:
            std::cout << prefix;
            std::cout << "SCAN " << queryPlan.relName;
            if (queryPlan.conds.size() > 0) {
                std::cout << " FILTER:" << std::endl;
                for (auto cond : queryPlan.conds)
                    std::cout << prefix << " - " << cond << std::endl;
            } else {
                std::cout << std::endl;
            }
            if (queryPlan.projection.size() > 0) {
                std::cout << prefix;
                std::cout << "> PROJECTION:";
                for (auto attrName : queryPlan.projection)
                    std::cout << " " << attrName;
                std::cout << std::endl;
            }
            if (queryPlan.innerLoop != nullptr) {
                for (auto plan : *queryPlan.innerLoop)
                    TRY(PrintQueryPlan(plan, indent + 4));
            }
            if (queryPlan.tempSaveName != "") {
                std::cout << prefix;
                std::cout << "=> SAVING AS " << queryPlan.tempSaveName;
                std::cout << std::endl;
            }
            break;
        case QP_SEARCH:
            std::cout << prefix;
            std::cout << "SEARCH " << queryPlan.relName;
            std::cout << " USING INDEX ON " << queryPlan.attrName;
            assert(queryPlan.conds.size() == 1);
            std::cout << " FILTER: " << queryPlan.conds[0] << std::endl;
            if (queryPlan.projection.size() > 0) {
                std::cout << prefix;
                std::cout << "> PROJECTION:";
                for (auto attrName : queryPlan.projection)
                    std::cout << " " << attrName;
                std::cout << std::endl;
            }
            if (queryPlan.tempSaveName != "") {
                std::cout << prefix;
                std::cout << "=> SAVING AS " << queryPlan.tempSaveName;
                std::cout << std::endl;
            }
            break;
        case QP_AUTOINDEX:
            std::cout << prefix;
            std::cout << "CREATE AUTO INDEX FOR " << queryPlan.relName << "(" << queryPlan.attrName << ")";
            std::cout << std::endl;
            break;
        case QP_FINAL:
            std::cout << prefix;
            std::cout << "FINAL RESULT " << queryPlan.relName;
            std::cout << std::endl;
            break;
    }
    return 0;
}

RC QL_Manager::ExecuteQueryPlan(const QL_QueryPlan &queryPlan, const AttrMap<DataAttrInfo &> &attrMap, const AttrMap<Value> &valMap) {
    return 0;
}

RC QL_Manager::Insert(const char *relName, int nValues, const Value *values) {
    RelCatEntry relEntry;
    TRY(pSmm->GetRelEntry(relName, relEntry));
    int attrCount;
    bool kSort = true;
    std::vector<DataAttrInfo> attributes;
    TRY(pSmm->GetDataAttrInfo(relName, attrCount, attributes, kSort));
    if (nValues != attrCount) {
        return QL_ATTR_COUNT_MISMATCH;
    }
    int nullableNum = 0;
    for (int i = 0; i < attrCount; ++i) {
        bool nullable = ((attributes[i].attrSpecs & ATTR_SPEC_NOTNULL) == 0);
        if (nullable) {
            ++nullableNum;
        }
        if (!can_assign_to(attributes[i].attrType, values[i].type, nullable)) {
            return QL_VALUE_TYPES_MISMATCH;
        }
    }
    
    ARR_PTR(data, char, relEntry.tupleLength);
    ARR_PTR(isnull, bool, nullableNum);
    int nullableIndex = 0;
    for (int i = 0; i < attrCount; ++i) {
        bool nullable = ((attributes[i].attrSpecs & ATTR_SPEC_NOTNULL) == 0);
        if (nullable) {
            isnull[nullableIndex++] = (values[i].type == VT_NULL);
        }
        if (values[i].type == VT_NULL) {
            continue;
        }
        auto &attr = attributes[i];
        void *value = values[i].data;
        char *dest = data + attr.offset;
        switch (attr.attrType) {
            case INT: {
                *(int *)dest = *(int *)value;
                break;
            }
            case FLOAT: {
                *(float *)dest = *(float *)value;
                break;
            }
            case STRING: {
                char *src = (char *)value;
                if (strlen(src) > attr.attrLength) return QL_STRING_VAL_TOO_LONG;
                strcpy(dest, src);
                break;
            }
        }
    }
    
    RM_FileHandle fh;
    RID rid;
    
    TRY(pRmm->OpenFile(relName, fh));
    TRY(fh.InsertRec(data, rid, isnull));
    TRY(pRmm->CloseFile(fh));
    
    return 0;
}

RC QL_Manager::Delete(const char *relName, int nConditions, const Condition *conditions) {
    return 0;
}

RC QL_Manager::Update(const char *relName, const RelAttr &updAttr, const int bIsValue, const RelAttr &rhsRelAttr, const Value &rhsValue, int nConditions,
                      const Condition *conditions) {
    return 0;
}
