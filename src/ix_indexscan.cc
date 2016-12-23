//
// Created by Kanari on 2016/12/24.
//

#include "ix.h"

IX_IndexScan::IX_IndexScan() {
	
}

IX_IndexScan::~IX_IndexScan() {
	
}

RC IX_IndexScan::OpenScan(const IX_IndexHandle &indexHandle, CompOp compOp, void *value, ClientHint pinHint) {
	return 0;
}

RC IX_IndexScan::GetNextEntry(RID &rid) {
	return 0;
}

RC IX_IndexScan::CloseScan() {
	return 0;
}
