//
// Created by Kanari on 2016/10/17.
//

#include "rm.h"

#include <cstring>
#include <cassert>

RM_FileScan::RM_FileScan() {
	scanOpened = false;
}

RM_FileScan::~RM_FileScan() {}

RC RM_FileScan::OpenScan(const RM_FileHandle &fileHandle, AttrType attrType, int attrLength, int attrOffset, CompOp compOp, void *value, ClientHint pinHint) {
	if (scanOpened) return RM_SCAN_NOT_CLOSED;

	this->fileHandle = &fileHandle;
	this->attrType = attrType;
	this->attrLength = attrLength;
	this->attrOffset = attrOffset;
	this->compOp = compOp;
	if (value == NULL) {
		this->value.stringVal = NULL;
	} else {
		switch (attrType) {
			case INT:
				this->value.intVal = *(int *)value;
				break;
			case FLOAT:
				this->value.floatVal = *(float *)value;
				break;
			case STRING:
				this->value.stringVal = new char[attrLength];
				memcpy(this->value.stringVal, value, (size_t)attrLength);
				break;
		}
	}

	char *data;
	PF_PageHandle pageHandle;
	scanOpened = true;
	TRY(fileHandle.pfHandle.GetFirstPage(pageHandle));
	TRY(pageHandle.GetData(data));
	recordSize = ((RM_FileHeader *)data)->recordSize;
	currentPageNum = 1;
	currentSlotNum = 0;
	TRY(fileHandle.pfHandle.UnpinPage(0));

	return 0;
}

RC RM_FileScan::GetNextRec(RM_Record &rec) {
	if (!scanOpened) return RM_SCAN_NOT_OPENED;

	char *data;
	PF_PageHandle pageHandle;
	bool found = false;
	TRY(fileHandle->pfHandle.GetThisPage(currentPageNum, pageHandle));
	while (true) {
		TRY(pageHandle.GetData(data));
		int cnt = ((RM_PageHeader *)data)->allocatedRecords;
		unsigned char *bitMap = ((RM_PageHeader *)data)->occupiedBitMap;
		for (; currentSlotNum < cnt; ++currentSlotNum) {
			if (getBitMap(bitMap, currentSlotNum) == 0) continue;
			char *pData = data + fileHandle->pageHeaderSize + recordSize * currentSlotNum;
			if (checkSatisfy(pData)) {
				found = true;
				break;
			}
		}
		TRY(fileHandle->pfHandle.UnpinPage(currentPageNum));
		if (found) break;
		int rc = fileHandle->pfHandle.GetNextPage(currentPageNum, pageHandle);
		if (rc == PF_EOF) return RM_EOF;
		else if (rc != 0) return rc;
		TRY(pageHandle.GetPageNum(currentPageNum));
		currentSlotNum = 0;
	}

	rec.rid = RID(currentPageNum, currentSlotNum);
	rec.SetData(data + fileHandle->pageHeaderSize + recordSize * currentSlotNum,
			(size_t)recordSize);
	++currentSlotNum;
	return 0;
}

RC RM_FileScan::CloseScan() {
	if (!scanOpened) return RM_SCAN_NOT_OPENED;
	scanOpened = false;
	if (attrType == STRING && value.stringVal != NULL)
		delete[] value.stringVal;
	return 0;
}

bool RM_FileScan::checkSatisfy(char *data) {
	switch (attrType) {
		case INT: {
			int curVal = *(int *)(data + attrOffset);
			switch (compOp) {
				case NO_OP:
					return true;
				case EQ_OP:
					return curVal == value.intVal;
				case NE_OP:
					return curVal != value.intVal;
				case LT_OP:
					return curVal < value.intVal;
				case GT_OP:
					return curVal > value.intVal;
				case LE_OP:
					return curVal <= value.intVal;
				case GE_OP:
					return curVal >= value.intVal;
			}
		}
		case FLOAT: {
			float curVal = *(float *)(data + attrOffset);
			switch (compOp) {
				case NO_OP:
					return true;
				case EQ_OP:
					return curVal == value.floatVal;
				case NE_OP:
					return curVal != value.floatVal;
				case LT_OP:
					return curVal < value.floatVal;
				case GT_OP:
					return curVal > value.floatVal;
				case LE_OP:
					return curVal <= value.floatVal;
				case GE_OP:
					return curVal >= value.floatVal;
			}
		}
		case STRING: {
			char *curVal = data + attrOffset;
			switch (compOp) {
				case NO_OP:
					return true;
				case EQ_OP:
					return strncmp(curVal, value.stringVal, (size_t)attrLength) == 0;
				case NE_OP:
					return strncmp(curVal, value.stringVal, (size_t)attrLength) != 0;
				case LT_OP:
					return strncmp(curVal, value.stringVal, (size_t)attrLength) < 0;
				case GT_OP:
					return strncmp(curVal, value.stringVal, (size_t)attrLength) > 0;
				case LE_OP:
					return strncmp(curVal, value.stringVal, (size_t)attrLength) <= 0;
				case GE_OP:
					return strncmp(curVal, value.stringVal, (size_t)attrLength) >= 0;
			}
		}
	}
	assert(0);
}
