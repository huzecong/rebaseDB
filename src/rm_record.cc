//
// Created by Kanari on 2016/10/17.
//

#include "rm.h"

RM_Record::~RM_Record() {
	delete[] pData;
}