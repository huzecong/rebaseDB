//
// Created by Kanari on 2016/12/24.
//

#include "ql.h"

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
            (vt == VT_FLOAT && rt == FLOAT) ||
            (vt == VT_STRING && rt == STRING);
}

RC QL_Manager::Select(int nSelAttrs, const RelAttr *selAttrs, int nRelations, const char *const *relations, int nConditions, const Condition *conditions) {
    return 0;
}

RC QL_Manager::Insert(const char *relName, int nValues, const Value *values) {
    RelCatEntry relEntry;
    TRY(pSmm->GetRelEntry(relName, relEntry));
    int attrCount;
    DataAttrInfo *attributes;
    bool kSort = true;
    TRY(pSmm->GetDataAttrInfo(relName, attrCount, attributes, kSort));
    if (nValues != attrCount) {
        return QL_VALUES_NUM_NOT_MATCH;
    }
    int nullableNum = 0;
    for (int i = 0; i < attrCount; ++i) {
        bool nullable = ((attributes[i].attrSpecs & ATTR_SPEC_NOTNULL) == 0);
        if (nullable) {
            ++nullableNum;
        }
        if (!can_assign_to(attributes[i].attrType, values[i].type, nullable)) {
            return QL_VALUES_TYPE_NOT_MATCH;
        }
    }

    RC errval = 0;

    char* data = new char[relEntry.tupleLength];
    bool* isnull = new bool[nullableNum];
    int nullableIndex = 0;
    for (int i = 0; i < attrCount && errval == 0; ++i) {
        bool nullable = ((attributes[i].attrSpecs & ATTR_SPEC_NOTNULL) == 0);
        if (nullable) {
            isnull[nullableIndex++] = (values[i].type == VT_NULL);
        }
        if (values[i].type == VT_NULL) {
            continue;
        }
        auto & attr = attributes[i];
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
                if (strlen(src) > attr.attrLength) {
                    errval = QL_STRING_VAL_TOO_LONG;
                    break;
                }
                strcpy(dest, src);
                break;
            }
        }
    }

    RM_FileHandle fh;
    RID rid;

    if (errval) {
        goto fail;
    }

    if ((errval = pRmm->OpenFile(relName, fh))) goto fail;
    if ((errval = fh.InsertRec(data, rid, isnull))) goto fail;
    if ((errval = pRmm->CloseFile(fh))) goto fail;

fail:
    delete [] data;
    delete [] isnull;

    return errval;
}

RC QL_Manager::Delete(const char *relName, int nConditions, const Condition *conditions) {
    return 0;
}

RC QL_Manager::Update(const char *relName, const RelAttr &updAttr, const int bIsValue, const RelAttr &rhsRelAttr, const Value &rhsValue, int nConditions,
                      const Condition *conditions) {
    return 0;
}
