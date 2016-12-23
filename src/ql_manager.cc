//
// Created by Kanari on 2016/12/24.
//

#include "ql.h"

QL_Manager::QL_Manager(SM_Manager &smm, IX_Manager &ixm, RM_Manager &rmm) {
	
}

QL_Manager::~QL_Manager() {
	
}

RC QL_Manager::Select(int nSelAttrs, const RelAttr *selAttrs, int nRelations, const char *const *relations, int nConditions, const Condition *conditions) {
	return 0;
}

RC QL_Manager::Insert(const char *relName, int nValues, const Value *values) {
	return 0;
}

RC QL_Manager::Delete(const char *relName, int nConditions, const Condition *conditions) {
	return 0;
}

RC QL_Manager::Update(const char *relName, const RelAttr &updAttr, const int bIsValue, const RelAttr &rhsRelAttr, const Value &rhsValue, int nConditions,
                      const Condition *conditions) {
	return 0;
}