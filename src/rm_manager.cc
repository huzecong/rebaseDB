//
// Created by Kanari on 2016/10/16.
//

#include "pf.h"
#include "rm.h"

#include "stddef.h"
#include <cstring>

/* RM Manager */
RM_Manager::RM_Manager(PF_Manager &pfm) {
    this->pfm = &pfm;
}

RM_Manager::~RM_Manager() {}

RC RM_Manager::CreateFile(const char *fileName, int recordSize,
        short nullableNum, short *nullableOffsets) {
    if (recordSize > PF_PAGE_SIZE) {
        return RM_RECORDSIZE_TOO_LARGE;
    }
    if (sizeof(RM_PageHeader) + nullableNum * sizeof(short) > PF_PAGE_SIZE) {
        return RM_RECORDSIZE_TOO_LARGE;
    }
    pfm->CreateFile(fileName);
    // initialize header
    PF_FileHandle fileHandle;
    PF_PageHandle pageHandle;
    RM_FileHeader *fileHeader;
    RM_PageHeader *pageHeader;
    TRY(pfm->OpenFile(fileName, fileHandle));
    TRY(fileHandle.AllocatePage(pageHandle));
    TRY(pageHandle.GetData(CVOID(fileHeader)));

    // total size = sizeof PageHeader + bitmap[ = records * (1 + nullable)] +
    //   records * recordSize
    short recordsPerPage = (PF_PAGE_SIZE - sizeof(RM_PageHeader)) /
            (recordSize + nullableNum + 1);
    if (upper_align<4>(recordsPerPage * (nullableNum + 1)) +
            sizeof(RM_PageHeader) + recordSize * recordsPerPage > PF_PAGE_SIZE)
        --recordsPerPage;
    fileHeader->recordSize = recordSize;
    fileHeader->recordsPerPage = recordsPerPage;
    fileHeader->nullableNum = nullableNum;
    fileHeader->firstFreePage = kLastFreePage;
    for (int i = 0; i < nullableNum; ++i) {
        fileHeader->nullableOffsets[i] = nullableOffsets[i];
    }

    TRY(fileHandle.MarkDirty(0));
    TRY(fileHandle.UnpinPage(0));

    TRY(fileHandle.AllocatePage(pageHandle));
    TRY(pageHandle.GetData(CVOID(pageHeader)));

    *pageHeader = {kLastFreeRecord, 0, kLastFreePage};
    memset(pageHeader + offsetof(RM_PageHeader, bitmap), 0,
            recordsPerPage * (nullableNum + 1));

    TRY(fileHandle.MarkDirty(1));
    TRY(fileHandle.UnpinPage(1));

    TRY(pfm->CloseFile(fileHandle));
    return 0;
}

// must ensure file is not open
RC RM_Manager::DestroyFile(const char *fileName) {
    return pfm->DestroyFile(fileName);
}

RC RM_Manager::OpenFile(const char *fileName, RM_FileHandle &fileHandle) {
    PF_FileHandle pfHandle;
    TRY(pfm->OpenFile(fileName, pfHandle));
    fileHandle.pfHandle = pfHandle;
    PF_PageHandle pageHandle;
    RM_FileHeader *data;
    TRY(pfHandle.GetFirstPage(pageHandle));
    TRY(pageHandle.GetData(CVOID(data)));

    fileHandle.recordSize = data->recordSize;
    fileHandle.recordsPerPage = data->recordsPerPage;
    fileHandle.nullableNum = data->nullableNum;
    if (data->nullableNum < 0) {
        return RM_BAD_NULLABLE_NUM;
    } else if (data->nullableNum > 0) {
        fileHandle.nullableOffsets = new short[data->nullableNum];
    } else {
        fileHandle.nullableOffsets = NULL;
    }
    for (int i = 0; i < data->nullableNum; ++i) {
        fileHandle.nullableOffsets[i] = data->nullableOffsets[i];
    }
    fileHandle.firstFreePage = data->firstFreePage;
    fileHandle.isHeaderDirty = false;
    fileHandle.pageHeaderSize = sizeof(RM_PageHeader) + upper_align<4>(
            data->recordsPerPage * (1 + data->nullableNum));

    TRY(pfHandle.UnpinPage(0));
    return 0;
}

RC RM_Manager::CloseFile(RM_FileHandle &fileHandle) {
    if (fileHandle.isHeaderDirty) {
        PF_PageHandle pageHandle;
        RM_FileHeader *header;
        TRY(fileHandle.pfHandle.GetFirstPage(pageHandle));
        TRY(pageHandle.GetData(CVOID(header)));

        header->recordSize = fileHandle.recordSize;
        header->recordsPerPage = fileHandle.recordsPerPage;
        header->nullableNum = fileHandle.nullableNum;
        header->firstFreePage = fileHandle.firstFreePage;
        for (int i = 0; i < fileHandle.nullableNum; ++i) {
            header->nullableOffsets[i] = fileHandle.nullableOffsets[i];
        }
        delete [] fileHandle.nullableOffsets;

        TRY(fileHandle.pfHandle.MarkDirty(0));
        TRY(fileHandle.pfHandle.UnpinPage(0));
    }
    TRY(pfm->CloseFile(fileHandle.pfHandle));
    return 0;
}
