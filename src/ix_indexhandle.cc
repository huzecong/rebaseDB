//
// Created by Kanari on 2016/12/24.
//

#include "ix.h"

IX_IndexHandle::IX_IndexHandle() {
	
}

IX_IndexHandle::~IX_IndexHandle() {
	
}

RC IX_IndexHandle::InsertEntry(void *pData, const RID &rid) {
	return 0;
}

RC IX_IndexHandle::DeleteEntry(void *pData, const RID &rid) {
	return 0;
}

RC IX_IndexHandle::ForcePages() {
	return 0;
}
