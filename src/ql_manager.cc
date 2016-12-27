//
// Created by Kanari on 2016/12/24.
//

#include "ql.h"

#include <memory>

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
    bool kSort = true;
    std::unique_ptr<DataAttrInfo[]> _attributes;
    TRY(pSmm->GetDataAttrInfo(relName, attrCount, _attributes, kSort));
    DataAttrInfo *attributes = _attributes.get();
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
