//
// Created by Kanari on 2016/12/24.
//

#include <algorithm>
#include <set>
#include <numeric>
#include <cassert>
#include "ql.h"
#include "ql_iterator.h"
#include "ql_disjoint.h"

QL_Manager::QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm) {
    pSmm = &smm;
    pIxm = &ixm;
    pRmm = &rmm;
    QL_Iterator::setRM(pRmm);
    QL_Iterator::setIX(pIxm);
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
    return AttrTag(info.relName ? std::string(info.relName) : "", std::string(info.attrName));
};

inline AttrTag make_tag(const DataAttrInfo &info) {
    return AttrTag(info.relName, info.attrName);
}

#define DEFINE_ATTRINFO(_name, _key) \
    auto __iter__##_name = attrMap.find(_key); \
    if (__iter__##_name == attrMap.end()) return QL_ATTR_NOTEXIST; \
    DataAttrInfo &_name = __iter__##_name->second;

template <typename T>
inline void erase_from(std::vector<T> &vector, const T &val) {
    vector.erase(std::remove_if(vector.begin(), vector.end(), [&val](const T &lhs) { return lhs == val; }), vector.end());
}

inline AttrMap<DataAttrInfo> create_map(const AttrList &vector) {
    AttrMap<DataAttrInfo> map;
    for (auto info : vector)
        map[make_tag(info)] = info;
    return map;
}

AttrList joinRelations(const AttrList &relA, const AttrList &relB) {
    AttrList ret;
    int offset = 0;
    for (auto info : relA) {
        ret.push_back(info);
        offset += upper_align<4>(info.attrSize);
    }
    for (auto info : relB) {
        info.offset += offset;
        ret.push_back(info);
    }
    return ret;
}

AttrList projectRelation(const AttrList &projection) {
    AttrList ret;
    int offset = 0;
    short nullableIndex = 0;
    for (auto proj : projection) {
        proj.offset = offset;
        offset += upper_align<4>(proj.attrSize);
        if (!(proj.attrSpecs & ATTR_SPEC_NOTNULL)) {
            proj.nullableIndex = nullableIndex++;
        } else {
            proj.nullableIndex = -1;
        }
        ret.push_back(proj);
    }
    return ret;
}

template <typename T, typename F>
inline void map_function(T &container, const F &func) {
    std::transform(container.begin(), container.end(), container.begin(), func);
};

RC QL_Manager::Select(int nSelAttrs, const RelAttr *selAttrs,
                      int nRelations, const char *const *relations,
                      int nConditions, const Condition *conditions) {
    // open files
    std::vector<RM_FileHandle> fileHandles((unsigned long)nRelations);
    for (int i = 0; i < nRelations; ++i)
        TRY(pRmm->OpenFile(relations[i], fileHandles[i]));
    std::vector<RelCatEntry> relEntries((unsigned long)nRelations);
    for (int i = 0; i < nRelations; ++i)
        TRY(pSmm->GetRelEntry(relations[i], relEntries[i]));
    VLOG(1) << "files opened";

    /**
     * Check if query is valid
     */
    // create mappings of attribute names to corresponding info
    std::vector<AttrList> attrInfo((unsigned long)nRelations);
    std::vector<int> attrCount((unsigned long)nRelations);
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
        if (selAttrs[i].relName == NULL && attrNameCount[std::string(selAttrs[i].attrName)] > 1)
            return QL_AMBIGUOUS_ATTR_NAME;
    }
    VLOG(1) << "all attributes exist";

    // check conditions are valid
    for (int i = 0; i < nConditions; ++i) {
        DEFINE_ATTRINFO(lhsAttr, make_tag(conditions[i].lhsAttr));
        bool nullable = ((lhsAttr.attrSpecs & ATTR_SPEC_NOTNULL) == 0);
        if (conditions[i].bRhsIsAttr) {
            DEFINE_ATTRINFO(rhsAttr, make_tag(conditions[i].rhsAttr));
            if (lhsAttr.attrType != rhsAttr.attrType) {
                return QL_ATTR_TYPES_MISMATCH;
            }
        } else {
            if (!can_assign_to(lhsAttr.attrType, conditions[i].rhsValue.type, nullable))
                return QL_VALUE_TYPES_MISMATCH;
        }
    }
    VLOG(1) << "all conditions are valid";

    /**
     * Generate query plan
     */
    std::vector<QL_Iterator *> queryPlans;
    std::vector<std::string> temporaryTables;

    std::map<std::string, int> relNumMap;
    for (int i = 0; i < nRelations; ++i)
        relNumMap[std::string(relations[i])] = i;

    // build target projections
    AttrList finalProjections;
    if (nSelAttrs == 0) {
        for (int i = 0; i < nRelations; ++i)
            for (int j = 0; j < attrCount[i]; ++j)
                finalProjections.push_back(attrInfo[i][j]);
    } else {
        for (int i = 0; i < nSelAttrs; ++i) {
            DataAttrInfo &info = attrMap[make_tag(selAttrs[i])];
            finalProjections.push_back(info);
        }
    }
    VLOG(1) << "target projections built";

    // gather simple conditions and projections for each table
    std::vector<std::vector<QL_Condition>> simpleConditions((unsigned long)nRelations);
    std::vector<QL_Condition> complexConditions;
    std::vector<std::set<std::string>> simpleProjectionNames((unsigned long)nRelations);
    std::vector<AttrList> simpleProjections((unsigned long)nRelations);
    if (nSelAttrs == 0) {
        for (int i = 0; i < nRelations; ++i)
            for (auto info : attrInfo[i])
                simpleProjectionNames[i].insert(std::string(info.attrName));
    } else {
        for (int i = 0; i < nSelAttrs; ++i) {
            DataAttrInfo &info = attrMap[make_tag(selAttrs[i])];
            simpleProjectionNames[relNumMap[std::string(info.relName)]].insert(std::string(info.attrName));
        }
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
            simpleProjectionNames[lhsAttrNum].insert(std::string(lhsAttr.attrName));
            simpleProjectionNames[rhsAttrNum].insert(std::string(rhsAttr.attrName));
        } else {
            cond.rhsValue = conditions[i].rhsValue;
            simpleConditions[lhsAttrNum].push_back(cond);
            simpleProjectionNames[lhsAttrNum].insert(std::string(lhsAttr.attrName));
        }
    }
    for (int i = 0; i < nRelations; ++i) {
        for (auto attrName : simpleProjectionNames[i])
            simpleProjections[i].push_back(attrMap[AttrTag(relations[i], attrName)]);
        sort(simpleProjections[i].begin(), simpleProjections[i].end(),
             [](const DataAttrInfo &lhs, const DataAttrInfo &rhs) {
                 return lhs.offset < rhs.offset;
             });
        if (simpleProjections[i] == attrInfo[i])
            simpleProjections[i].clear();
    }
    VLOG(1) << "simple conditions and projections gathered";

    // helper functions
    std::vector<bool> filtered((unsigned long)nRelations);
    std::vector<QL_Iterator *> iterators((unsigned long)nRelations);
    QL_DisjointSet disjointSet(nRelations);
    std::vector<AttrMap<DataAttrInfo>> attrMaps((unsigned long)nRelations);
    for (int i = 0; i < nRelations; ++i)
        attrMaps[i] = create_map(attrInfo[i]);
    auto findRelNum = [&](const DataAttrInfo &info) {
        return disjointSet.find(relNumMap[info.relName]);
    };
    auto updateCondition = [&](const QL_Condition &condition) {
        QL_Condition ret = condition;
        ret.lhsAttr = attrMaps[findRelNum(ret.lhsAttr)][make_tag(ret.lhsAttr)];
        ret.rhsAttr = attrMaps[findRelNum(ret.rhsAttr)][make_tag(ret.rhsAttr)];
        return ret;
    };
    auto updateAttrInfo = [&](int relNum, const AttrList &list) {
        attrInfo[relNum] = list;
        attrMaps[relNum] = create_map(list);
    };
    auto findCorrespondingAttrs = [&](int relNum, const AttrList &list) {
        AttrList ret;
        for (auto info : list)
            ret.push_back(attrMaps[relNum][make_tag(info)]);
        return ret;
    };
    auto performSimpleOperations = [&](QL_Iterator *iter, int relNum) {
        queryPlans.push_back(iter);
        if (simpleConditions[relNum].size() > 0)
            queryPlans.push_back(iter = new QL_SelectionIterator(iter, simpleConditions[relNum]));
        if (simpleProjections[relNum].size() > 0) {
            AttrList projectTo = projectRelation(simpleProjections[relNum]);
            AttrList projectFrom = findCorrespondingAttrs(relNum, projectTo);
            queryPlans.push_back(iter = new QL_ProjectionIterator(iter, projectFrom, projectTo));
            updateAttrInfo(relNum, projectTo);
        }
        iterators[relNum] = iter;
        VLOG(1) << "performed simple operations on " << relations[relNum];
    };
    auto findIndexedCondition = [&](int relNum, QL_Condition &indexedCondition) -> bool {
        bool found = false;
        for (auto cond : simpleConditions[relNum]) {
            if (!cond.bRhsIsAttr && cond.lhsAttr.indexNo != -1) {
                indexedCondition = cond;
                found = true;
                if (cond.op == EQ_OP) return true;
            }
        }
        return found;
    };
    auto performSimpleOperationsWithIndex = [&](int relNum) {
        QL_Condition indexedCondition;
        bool hasIndexedCondition = findIndexedCondition(relNum, indexedCondition);
        QL_Iterator *rhs;
        if (hasIndexedCondition) {
            rhs = new QL_IndexSearchIterator(indexedCondition);
            erase_from(simpleConditions[relNum], indexedCondition);
            VLOG(1) << relations[relNum] << " contains indexed condition";
        } else {
            rhs = new QL_FileScanIterator(relations[relNum]);
        }
        performSimpleOperations(rhs, relNum);
    };
    auto findBatchConditions = [&]() {
        std::vector<QL_Condition> ret;
        for (auto cond : complexConditions) {
            int l = findRelNum(cond.lhsAttr);
            int r = findRelNum(cond.rhsAttr);
            if (disjointSet.connected(l, r))
                ret.push_back(cond);
        }
        for (auto &cond : ret) {
            erase_from(complexConditions, cond);
            cond = updateCondition(cond);
        }
        return ret;
    };
    auto updateAttrList = [&](AttrList &list) {
        for (auto info : list)
            info = attrMaps[findRelNum(info)][make_tag(info)];
    };

    // deal with complex conditions with indexed attributes
    while (true) {
        QL_Condition condition;
        bool found = false;
        for (auto cond : complexConditions) {
            if (cond.lhsAttr.indexNo != -1 && !filtered[relNumMap[cond.lhsAttr.relName]]) {
                condition = cond;
                found = true;
                if (cond.op == EQ_OP) break;
            }
            if (cond.rhsAttr.indexNo != -1 && !filtered[relNumMap[cond.rhsAttr.relName]]) {
                condition = cond;
                std::swap(condition.lhsAttr, condition.rhsAttr);
                found = true;
                if (cond.op == EQ_OP) break;
            }
        }
        if (!found) break;
        erase_from(complexConditions, condition);

        int lhsNum = relNumMap[condition.lhsAttr.relName];
        filtered[lhsNum] = true;
        QL_IndexSearchIterator *lhsSearch = new QL_IndexSearchIterator(condition);
        performSimpleOperations(lhsSearch, lhsNum);
        int rhsNum = relNumMap[condition.rhsAttr.relName];
        if (!filtered[rhsNum]) {
            filtered[rhsNum] = true;
            performSimpleOperationsWithIndex(rhsNum);
        } else {
            rhsNum = findRelNum(condition.rhsAttr);
        }

        int searchAttrOffset = attrMaps[rhsNum][make_tag(condition.rhsAttr)].offset;
        VLOG(1) << "search offset " << searchAttrOffset;
        QL_Iterator *iter = new QL_IndexedJoinIterator(iterators[rhsNum], attrInfo[rhsNum],
                                                       lhsSearch, searchAttrOffset,
                                                       iterators[lhsNum], attrInfo[lhsNum]);
        queryPlans.push_back(iter);
        int joinedNum = disjointSet.join(lhsNum, rhsNum);
        AttrList joinedRelation = joinRelations(attrInfo[rhsNum], attrInfo[lhsNum]);
        updateAttrInfo(joinedNum, joinedRelation);
        std::vector<QL_Condition> batchConditions = findBatchConditions();
        if (batchConditions.size() > 0)
            queryPlans.push_back(iter = new QL_SelectionIterator(iter, batchConditions));
        iterators[joinedNum] = iter;
    }

    // perform simple operations on rest of relations
    for (int i = 0; i < nRelations; ++i) {
        if (filtered[i]) continue;
        performSimpleOperationsWithIndex(i);
    }

    // deal with rest of complex conditions
    while (complexConditions.size() > 0) {
        auto cond = *complexConditions.begin();
        complexConditions.erase(complexConditions.begin());
        int lhsNum = findRelNum(cond.lhsAttr);
        int rhsNum = findRelNum(cond.rhsAttr);
        assert(lhsNum != rhsNum);
        QL_Iterator *iter = new QL_NestedLoopJoinIterator(iterators[lhsNum], attrInfo[lhsNum],
                                                          iterators[rhsNum], attrInfo[rhsNum]);
        queryPlans.push_back(iter);
        int joinedNum = disjointSet.join(lhsNum, rhsNum);
        AttrList joinedRelation = joinRelations(attrInfo[lhsNum], attrInfo[rhsNum]);
        updateAttrInfo(joinedNum, joinedRelation);
        std::vector<QL_Condition> batchConditions = findBatchConditions();
        batchConditions.push_back(cond);
        queryPlans.push_back(iter = new QL_SelectionIterator(iter, batchConditions));
        iterators[joinedNum] = iter;
    }

    // join rest of relations
    for (int i = 1; i < nRelations; ++i) {
        if (!disjointSet.connected(0, i)) {
            int lhsNum = disjointSet.find(0);
            int rhsNum = disjointSet.find(i);
            QL_Iterator *iter = new QL_NestedLoopJoinIterator(iterators[lhsNum], attrInfo[lhsNum],
                                                              iterators[rhsNum], attrInfo[rhsNum]);
            queryPlans.push_back(iter);
            int joinedNum = disjointSet.join(lhsNum, rhsNum);
            AttrList joinedRelation = joinRelations(attrInfo[lhsNum], attrInfo[rhsNum]);
            updateAttrInfo(joinedNum, joinedRelation);
            iterators[joinedNum] = iter;
        }
    }

    int finalNum = disjointSet.find(0);
    QL_Iterator *final = iterators[finalNum];
    updateAttrList(finalProjections);
    finalProjections = projectRelation(finalProjections);
    if (finalProjections != attrInfo[finalNum]) {
        AttrList projectFrom = findCorrespondingAttrs(finalNum, finalProjections);
        queryPlans.push_back(final = new QL_ProjectionIterator(final, projectFrom, finalProjections));
    }

    if (bQueryPlans) {
        final->Print();
    }

    Printer printer(finalProjections);
    printer.PrintHeader(std::cout);
    int retcode;
    RM_Record record;
    char *data;
    bool *isnull;
    while ((retcode = final->GetNextRec(record)) != RM_EOF) {
        if (retcode) return retcode;
        TRY(record.GetData(data));
        TRY(record.GetIsnull(isnull));
        printer.Print(std::cout, data, isnull);
    }
    printer.PrintFooter(std::cout);

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

RC QL_Manager::Insert(const char *relName, int nValues, const Value *values) {
    if (!strcmp(relName, "relcat") || !strcmp(relName, "attrcat")) return QL_FORBIDDEN;
    RelCatEntry relEntry;
    TRY(pSmm->GetRelEntry(relName, relEntry));

    int attrCount;
    bool kSort = true;
    std::vector<DataAttrInfo> attributes;
    TRY(pSmm->GetDataAttrInfo(relName, attrCount, attributes, kSort));
    if (nValues % attrCount != 0) {
        return QL_ATTR_COUNT_MISMATCH;
    }
    int recordsNum = nValues / attrCount;
    int nullableNum = 0;
    for (int i = 0; i < attrCount; ++i) {
        bool nullable = ((attributes[i].attrSpecs & ATTR_SPEC_NOTNULL) == 0);
        if (nullable) {
            ++nullableNum;
        }
        for (int j = 0; j < recordsNum; ++j) {
            // LOG(INFO) << "checking " << j * attrCount + i;
            if (!can_assign_to(attributes[i].attrType, values[j * attrCount + i].type, nullable)) {
                LOG(WARNING) << "type mismatch at i = " << i << ", j = " << j;
                LOG(WARNING) << "attrtype = " << attributes[i].attrType <<
                    ", valuetype = " << values[j * attrCount + i];
                return QL_VALUE_TYPES_MISMATCH;
            }
        }
    }

    LOG(INFO) << "check done";

    for (int j = 0; j < recordsNum; ++j) {
        const Value *this_values = values + (j * attrCount);

        RM_FileHandle fh;
        IX_IndexHandle indexHandle;
        TRY(pRmm->OpenFile(relName, fh));
        IX_IndexScan scan;
        bool duplicate = false;
        for (int i = 0; i < attrCount; ++i)
            if (attributes[i].attrSpecs & ATTR_SPEC_PRIMARYKEY) {
                TRY(pIxm->OpenIndex(relName, attributes[i].indexNo, indexHandle));
                RID rid;
                TRY(scan.OpenScan(indexHandle, EQ_OP, this_values[i].data));
                int retcode = scan.GetNextEntry(rid);
                TRY(scan.CloseScan());
                TRY(pIxm->CloseIndex(indexHandle));
                if (retcode != IX_EOF) {
                    if (retcode != 0) return retcode;
                    duplicate = true;
                }
                break;
            }
        if (duplicate) return QL_DUPLICATE_PRIMARY_KEY;

        ARR_PTR(data, char, relEntry.tupleLength);
        ARR_PTR(isnull, bool, nullableNum);
        int nullableIndex = 0;
        for (int i = 0; i < attrCount; ++i) {
            bool nullable = ((attributes[i].attrSpecs & ATTR_SPEC_NOTNULL) == 0);
            if (nullable) {
                isnull[nullableIndex++] = (this_values[i].type == VT_NULL);
            }
            if (this_values[i].type == VT_NULL) {
                continue;
            }
            auto &attr = attributes[i];
            void *value = this_values[i].data;
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
                    if (strlen(src) > attr.attrDisplayLength) return QL_STRING_VAL_TOO_LONG;
                    strcpy(dest, src);
                    break;
                }
            }
        }

        RID rid;

        TRY(fh.InsertRec(data, rid, isnull));
        for (int i = 0; i < attrCount; ++i) {
            if (attributes[i].indexNo != -1) {
                TRY(pIxm->OpenIndex(relName, attributes[i].indexNo, indexHandle));
                TRY(indexHandle.InsertEntry(data + attributes[i].offset, rid));
                // TRY(indexHandle.Traverse());
                TRY(pIxm->CloseIndex(indexHandle));
            }
        }
        TRY(pRmm->CloseFile(fh));

        ++relEntry.recordCount;
        TRY(pSmm->UpdateRelEntry(relName, relEntry));

    }

    return 0;
}

RC checkAttrBelongsToRel(const RelAttr &relAttr, const char *relName) {
    if (relAttr.relName == NULL || !strcmp(relName, relAttr.relName)) return 0;
    return QL_ATTR_NOTEXIST;
}

RC QL_Manager::CheckConditionsValid(const char *relName, int nConditions, const Condition *conditions,
                                    const std::map<std::string, DataAttrInfo> &attrMap,
                                    std::vector<QL_Condition> &retConditions) {
    // check conditions are valid
    for (int i = 0; i < nConditions; ++i) {
        TRY(checkAttrBelongsToRel(conditions[i].lhsAttr, relName));
        if (conditions[i].bRhsIsAttr)
            TRY(checkAttrBelongsToRel(conditions[i].rhsAttr, relName));
        auto iter = attrMap.find(conditions[i].lhsAttr.attrName);
        if (iter == attrMap.end()) return QL_ATTR_NOTEXIST;
        const DataAttrInfo &lhsAttr = iter->second;

        QL_Condition cond;
        cond.lhsAttr = lhsAttr;
        cond.op = conditions[i].op;
        cond.bRhsIsAttr = (bool)conditions[i].bRhsIsAttr;

        bool nullable = ((lhsAttr.attrSpecs & ATTR_SPEC_NOTNULL) == 0);
        if (conditions[i].bRhsIsAttr) {
            iter = attrMap.find(conditions[i].rhsAttr.attrName);
            if (iter == attrMap.end()) return QL_ATTR_NOTEXIST;
            const DataAttrInfo &rhsAttr = iter->second;
            if (lhsAttr.attrType != rhsAttr.attrType)
                return QL_ATTR_TYPES_MISMATCH;
            cond.rhsAttr = rhsAttr;
        } else {
            if (!can_assign_to(lhsAttr.attrType, conditions[i].rhsValue.type, nullable))
                return QL_VALUE_TYPES_MISMATCH;
            cond.rhsValue = conditions[i].rhsValue;
        }

        retConditions.push_back(cond);
    }
    VLOG(1) << "all conditions are valid";

    return 0;
}

RC QL_Manager::Delete(const char *relName, int nConditions, const Condition *conditions) {
    if (!strcmp(relName, "relcat") || !strcmp(relName, "attrcat")) return QL_FORBIDDEN;
    RelCatEntry relEntry;
    TRY(pSmm->GetRelEntry(relName, relEntry));

    int attrCount;
    std::vector<DataAttrInfo> attributes;
    TRY(pSmm->GetDataAttrInfo(relName, attrCount, attributes, true));
    std::map<std::string, DataAttrInfo> attrMap;
    for (auto info : attributes)
        attrMap[info.attrName] = info;

    std::vector<QL_Condition> conds;
    TRY(CheckConditionsValid(relName, nConditions, conditions, attrMap, conds));

    std::vector<IX_IndexHandle> indexHandles((unsigned long)attrCount);
    for (int i = 0; i < attrCount; ++i)
        if (attributes[i].indexNo != -1)
            TRY(pIxm->OpenIndex(relName, attributes[i].indexNo, indexHandles[i]));

    RM_FileHandle fileHandle;
    TRY(pRmm->OpenFile(relName, fileHandle));
    RM_FileScan scan;
    TRY(scan.OpenScan(fileHandle, INT, 4, 0, NO_OP, NULL));
    RM_Record record;
    RC retcode;
    int cnt = 0;
    while ((retcode = scan.GetNextRec(record)) != RM_EOF) {
        if (retcode) return retcode;
        char *data;
        bool *isnull;
        TRY(record.GetData(data));
        TRY(record.GetIsnull(isnull));
        bool shouldDelete = true;
        for (int i = 0; i < nConditions && shouldDelete; ++i)
            shouldDelete = checkSatisfy(data, isnull, conds[i]);
        if (shouldDelete) {
            ++cnt;
            RID rid;
            TRY(record.GetRid(rid));
            TRY(fileHandle.DeleteRec(rid));
            for (int i = 0; i < attrCount; ++i)
                if (attributes[i].indexNo != -1)
                    TRY(indexHandles[i].DeleteEntry(data + attributes[i].offset, rid));
        }
    }
    TRY(scan.CloseScan());
    for (int i = 0; i < attrCount; ++i)
        if (attributes[i].indexNo != -1) {
            // TRY(indexHandles[i].Traverse());
            TRY(pIxm->CloseIndex(indexHandles[i]));
        }
    TRY(pRmm->CloseFile(fileHandle));

    relEntry.recordCount -= cnt;
    TRY(pSmm->UpdateRelEntry(relName, relEntry));
    std::cout << cnt << " tuple(s) deleted." << std::endl;

    return 0;
}

RC QL_Manager::Update(const char *relName, const RelAttr &updAttr,
                      const int bIsValue, const RelAttr &rhsRelAttr, const Value &rhsValue,
                      int nConditions, const Condition *conditions) {
    if (!strcmp(relName, "relcat") || !strcmp(relName, "attrcat")) return QL_FORBIDDEN;
    RelCatEntry relEntry;
    TRY(pSmm->GetRelEntry(relName, relEntry));

    TRY(checkAttrBelongsToRel(updAttr, relName));
    if (!bIsValue)
        TRY(checkAttrBelongsToRel(rhsRelAttr, relName));

    int attrCount;
    std::vector<DataAttrInfo> attributes;
    TRY(pSmm->GetDataAttrInfo(relName, attrCount, attributes, true));
    std::map<std::string, DataAttrInfo> attrMap;
    for (auto info : attributes)
        attrMap[info.attrName] = info;

    std::vector<QL_Condition> conds;
    TRY(CheckConditionsValid(relName, nConditions, conditions, attrMap, conds));

    DataAttrInfo updAttrInfo = attrMap[updAttr.attrName];
    int valAttrOffset = bIsValue ? 0 : attrMap[rhsRelAttr.attrName].offset;
    bool nullable = !(updAttrInfo.attrSpecs & ATTR_SPEC_NOTNULL);
    if (!nullable && bIsValue && rhsValue.type == VT_NULL)
        return QL_ATTR_IS_NOTNULL;

    IX_IndexHandle indexHandle;
    if (updAttrInfo.indexNo != -1)
        TRY(pIxm->OpenIndex(relName, updAttrInfo.indexNo, indexHandle));

    RM_FileHandle fileHandle;
    TRY(pRmm->OpenFile(relName, fileHandle));
    RM_FileScan scan;
    TRY(scan.OpenScan(fileHandle, INT, 4, 0, NO_OP, NULL));
    RM_Record record;
    RC retcode;
    int cnt = 0;
    while ((retcode = scan.GetNextRec(record)) != RM_EOF) {
        if (retcode) return retcode;
        char *data;
        bool *isnull;
        TRY(record.GetData(data));
        TRY(record.GetIsnull(isnull));
        bool shouldUpdate = true;
        for (int i = 0; i < nConditions && shouldUpdate; ++i)
            shouldUpdate = checkSatisfy(data, isnull, conds[i]);
        if (shouldUpdate) {
            ++cnt;
            if (bIsValue && rhsValue.type == VT_NULL) {
                isnull[updAttrInfo.nullableIndex] = true;
            } else {
                if (nullable) isnull[updAttrInfo.nullableIndex] = false;
                void *value = bIsValue ? rhsValue.data : data + valAttrOffset;
                if (updAttrInfo.indexNo != -1) {
                    RID rid;
                    TRY(record.GetRid(rid));
                    TRY(indexHandle.DeleteEntry(data + updAttrInfo.offset, rid));
                    TRY(indexHandle.InsertEntry(value, rid));
                }
                switch (updAttrInfo.attrType) {
                    case INT:
                        *(int *)(data + updAttrInfo.offset) = *(int *)value;
                        break;
                    case FLOAT:
                        *(float *)(data + updAttrInfo.offset) = *(float *)value;
                        break;
                    case STRING:
                        strcpy(data + updAttrInfo.offset, (char *)value);
                        break;
                }
            }
            TRY(fileHandle.UpdateRec(record));
        }
    }
    TRY(scan.CloseScan());
    if (updAttrInfo.indexNo != -1)
        TRY(pIxm->CloseIndex(indexHandle));
    TRY(pRmm->CloseFile(fileHandle));

    std::cout << cnt << " tuple(s) updated." << std::endl;

    return 0;
}

bool checkSatisfy(char *lhsData, bool lhsIsnull, char *rhsData, bool rhsIsnull, const QL_Condition &condition) {
    switch (condition.op) {
        case NO_OP:
            return true;
        case ISNULL_OP:
            return lhsIsnull;
        case NOTNULL_OP:
            return !lhsIsnull;
        default:
            break;
    }
    if (lhsIsnull || rhsIsnull) return false;

    switch (condition.lhsAttr.attrType) {
        case INT: {
            int lhs = *(int *)lhsData;
            int rhs = *(int *)rhsData;
            switch (condition.op) {
                case EQ_OP:
                    return lhs == rhs;
                case NE_OP:
                    return lhs != rhs;
                case LT_OP:
                    return lhs < rhs;
                case GT_OP:
                    return lhs > rhs;
                case LE_OP:
                    return lhs <= rhs;
                case GE_OP:
                    return lhs >= rhs;
                default:
                    CHECK(false);
            }
        }
        case FLOAT: {
            float lhs = *(float *)lhsData;
            float rhs = *(float *)rhsData;
            switch (condition.op) {
                case EQ_OP:
                    return lhs == rhs;
                case NE_OP:
                    return lhs != rhs;
                case LT_OP:
                    return lhs < rhs;
                case GT_OP:
                    return lhs > rhs;
                case LE_OP:
                    return lhs <= rhs;
                case GE_OP:
                    return lhs >= rhs;
                default:
                    CHECK(false);
            }
        }
        case STRING: {
            char *lhs = lhsData;
            char *rhs = rhsData;
            switch (condition.op) {
                case EQ_OP:
                    return strcmp(lhs, rhs) == 0;
                case NE_OP:
                    return strcmp(lhs, rhs) != 0;
                case LT_OP:
                    return strcmp(lhs, rhs) < 0;
                case GT_OP:
                    return strcmp(lhs, rhs) > 0;
                case LE_OP:
                    return strcmp(lhs, rhs) <= 0;
                case GE_OP:
                    return strcmp(lhs, rhs) >= 0;
                default:
                    CHECK(false);
            }
        }
    }
    return false;
}

bool checkSatisfy(char *data, bool *isnull, const QL_Condition &condition) {
    if (condition.bRhsIsAttr) {
        return checkSatisfy(data + condition.lhsAttr.offset,
                            !(condition.lhsAttr.attrSpecs & ATTR_SPEC_NOTNULL) ? isnull[condition.lhsAttr.nullableIndex] : false,
                            data + condition.rhsAttr.offset,
                            !(condition.rhsAttr.attrSpecs & ATTR_SPEC_NOTNULL) ? isnull[condition.rhsAttr.nullableIndex] : false,
                            condition);
    } else {
        return checkSatisfy(data + condition.lhsAttr.offset,
                            !(condition.lhsAttr.attrSpecs & ATTR_SPEC_NOTNULL) ? isnull[condition.lhsAttr.nullableIndex] : false,
                            (char *)condition.rhsValue.data,
                            condition.rhsValue.type == VT_NULL,
                            condition);
    }
}
