//
// Created by Kanari on 2016/10/17.
//

#include "rm.h"

RM_Record::RM_Record() {
    pData = NULL;
    isnull = NULL;
}

RM_Record::~RM_Record() {
    if (pData != NULL) {
        delete [] pData;
    }
    if (isnull != NULL) {
        delete [] isnull;
    }
}

void RM_Record::SetData(char *data, size_t size) {
    if (pData != NULL) {
        delete [] pData;
    }
    pData = new char[size];
    memcpy(pData, data, size);
}

void RM_Record::SetIsnull(bool* isnull, short nullableNum) {
    if (this->isnull != NULL) {
        delete [] this->isnull;
    }
    this->isnull = new bool[nullableNum];
    memcpy(this->isnull, isnull, nullableNum * sizeof(bool));
}

RC RM_Record::GetData(char *&pData) const {
    pData = this->pData;
    return pData == NULL ? RM_UNINITIALIZED_RECORD : 0;
}

RC RM_Record::GetIsnull(bool *&isnull) const {
    isnull = this->isnull;
    return pData == NULL ? RM_UNINITIALIZED_RECORD : 0;
}

RC RM_Record::GetRid(RID &rid) const {
    rid = this->rid;
    return pData == NULL ? RM_UNINITIALIZED_RECORD : 0;
}
