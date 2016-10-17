//
// Created by Kanari on 2016/10/17.
//

#include "rm.h"

RM_Record::RM_Record() {
	pData = NULL;
}

RM_Record::~RM_Record() {
	if (pData != NULL)
		delete[] pData;
}

RC RM_Record::GetData(char *&pData) const {
	pData = this->pData;
	return pData == NULL ? RM_UNINITIALIZED_RECORD : 0;
}

RC RM_Record::GetRid(RID &rid) const {
	rid = this->rid;
	return pData == NULL ? RM_UNINITIALIZED_RECORD : 0;
}

