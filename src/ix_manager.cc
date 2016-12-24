//
// Created by Kanari on 2016/12/24.
//

#include "ix.h"

IX_Manager::IX_Manager(PF_Manager &pfm) {
    
}

IX_Manager::~IX_Manager() {
    
}

RC IX_Manager::CreateIndex(const char *fileName, int indexNo, AttrType attrType, int attrLength) {
    return 0;
}

RC IX_Manager::DestroyIndex(const char *fileName, int indexNo) {
    return 0;
}

RC IX_Manager::OpenIndex(const char *fileName, int indexNo, IX_IndexHandle &indexHandle) {
    return 0;
}

RC IX_Manager::CloseIndex(IX_IndexHandle &indexHandle) {
    return 0;
}
