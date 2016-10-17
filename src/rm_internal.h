//
// Created by Kanari on 2016/10/16.
//

#ifndef RM_INTERNAL_H
#define RM_INTERNAL_H

static const int kLastFreePage = -1;
static const int kLastFreeRecord = -2;

struct RM_PageHeader {
	short firstFreeRecord;
	short allocatedRecords;
	int nextFreePage;
	unsigned char occupiedBitMap[1];
};

struct RM_FileHeader {
	short recordSize;
	short recordsPerPage;
	int firstFreePage;
};

#endif //REBASE_RM_INTERNAL_H
