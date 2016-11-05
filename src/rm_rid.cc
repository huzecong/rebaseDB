#include "rm.h"
#include "rm_rid.h"

static const int kUninitializedRidNums = -1;

RID::RID() {
	pageNum = kUninitializedRidNums;
	slotNum = kUninitializedRidNums;
}

RID::RID(PageNum pageNum, SlotNum slotNum) {
	this->pageNum = pageNum;
	this->slotNum = slotNum;
}

RID::~RID() {}

RC RID::GetPageNum(PageNum &pageNum) const {
	pageNum = this->pageNum;
	return pageNum == kUninitializedRidNums ? RM_UNINITIALIZED_RID : 0;
}

RC RID::GetSlotNum(SlotNum &slotNum) const {
	slotNum = this->slotNum;
	return slotNum == kUninitializedRidNums ? RM_UNINITIALIZED_RID : 0;
}
