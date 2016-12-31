//
// Created by Kanari on 2016/12/24.
//

#include "ix.h"
#include "ix_internal.h"

#include <string>
#include <sstream>
#include <stddef.h>

static std::string filename_gen(const char* fileName, int indexNo) {
    std::ostringstream oss;
    oss << fileName << "." << indexNo;
    return oss.str();
}

IX_Manager::IX_Manager(PF_Manager &pfm) /* : rmm(pfm) */ {
    this->pfm = &pfm;
}

IX_Manager::~IX_Manager() { }

RC IX_Manager::CreateIndex(const char *fileName, int indexNo, AttrType attrType, int attrLength) {
    int attrSize = upper_align<4>(attrLength);
    if (offsetof(IX_PageHeader, entries) + attrSize + 2 * offsetof(Entry, key) > PF_PAGE_SIZE) {
        return IX_ATTR_TOO_LARGE;
    }
    std::string indexFileName = filename_gen(fileName, indexNo);
    TRY(pfm->CreateFile(indexFileName.c_str()));
    PF_FileHandle fileHandle;
    PF_PageHandle pageHandle;
    TRY(pfm->OpenFile(indexFileName.c_str(), fileHandle));

    // create file header
    IX_FileHeader *fileHeader;
    TRY(fileHandle.AllocatePage(pageHandle));
    TRY(pageHandle.GetData(CVOID(fileHeader)));
    fileHeader->attrType = attrType;
    fileHeader->attrLength = attrLength;
    fileHeader->root = 1;
    fileHeader->firstFreePage = kLastFreePage;
    TRY(fileHandle.MarkDirty(0));
    TRY(fileHandle.UnpinPage(0));

    // create root node
    IX_PageHeader *root;
    TRY(fileHandle.AllocatePage(pageHandle));
    TRY(pageHandle.GetData(CVOID(root)));
    root->type = kLeafNode;
    root->childrenNum = 0;
    Entry* root_first_entry = (Entry*)(root->entries);
    root_first_entry->pageNum = kNullNode;
    TRY(fileHandle.MarkDirty(1));
    TRY(fileHandle.UnpinPage(1));

    TRY(pfm->CloseFile(fileHandle));

    // RM_Manager rmm(*pfm);
    // TRY(rmm.CreateFile((indexFileName + "r").c_str(), sizeof(RID)));
    return 0;
}

RC IX_Manager::DestroyIndex(const char *fileName, int indexNo) {
    std::string indexFileName = filename_gen(fileName, indexNo);
    TRY(pfm->DestroyFile(indexFileName.c_str()));

    // TRY(rmm.DestroyFile((indexFileName + "r").c_str()));
    return 0;
}

RC IX_Manager::OpenIndex(const char *fileName, int indexNo, IX_IndexHandle &indexHandle) {
    std::string indexFileName = filename_gen(fileName, indexNo);
    PF_FileHandle &fileHandle = indexHandle.pfHandle;
    PF_PageHandle pageHandle;
    IX_FileHeader *fileHeader;

    TRY(pfm->OpenFile(indexFileName.c_str(), fileHandle));
    TRY(fileHandle.GetFirstPage(pageHandle));
    TRY(pageHandle.GetData(CVOID(fileHeader)));
    indexHandle.attrType = fileHeader->attrType;
    indexHandle.attrLength = fileHeader->attrLength;
    indexHandle.root = fileHeader->root;
    indexHandle.firstFreePage = fileHeader->firstFreePage;
    indexHandle.isHeaderDirty = false;
    TRY(fileHandle.UnpinPage(0));
    // the initialization MUST come after information in the
    // header copied into the handle
    indexHandle.__initialize();

    // RM_FileHandle &rmHandle = indexHandle.rmHandle;
    // TRY(rmm.OpenFile((indexFileName + "r").c_str(), rmHandle));

    return 0;
}

RC IX_Manager::CloseIndex(IX_IndexHandle &indexHandle) {
    PF_FileHandle &fileHandle = indexHandle.pfHandle;

    if (indexHandle.isHeaderDirty) {
        PF_PageHandle pageHandle;
        IX_FileHeader *fileHeader;

        TRY(fileHandle.GetFirstPage(pageHandle));
        TRY(pageHandle.GetData(CVOID(fileHeader)));
        fileHeader->root = indexHandle.root;
        fileHeader->firstFreePage = indexHandle.firstFreePage;

        TRY(fileHandle.MarkDirty(0));
        TRY(fileHandle.UnpinPage(0));
    }

    TRY(pfm->CloseFile(fileHandle));

    // RM_FileHandle &rmHandle = indexHandle.rmHandle;
    // TRY(rmm.CloseFile(rmHandle));
    return 0;
}
