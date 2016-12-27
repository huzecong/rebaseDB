//
// Created by Kanari on 2016/12/23.
//

#ifndef REBASE_SM_MANAGER_H
#define REBASE_SM_MANAGER_H

#include "sm.h"
#include "printer.h"

#include <unistd.h>
#include <stddef.h>
#include <algorithm>
#include <vector>
#include <cassert>
#include <memory>

static const int kCwdLen = 256;

SM_Manager::SM_Manager(IX_Manager &ixm_, RM_Manager &rmm_) {
    this->ixm = &ixm_;
    this->rmm = &rmm_;
}

SM_Manager::~SM_Manager() {}

RC SM_Manager::OpenDb(const char *dbName) {
    if (chdir(dbName) != 0) return SM_CHDIR_FAILED;
    TRY(rmm->OpenFile("relcat", relcat));
    TRY(rmm->OpenFile("attrcat", attrcat));
    return 0;
}

RC SM_Manager::CloseDb() {
    TRY(rmm->CloseFile(relcat));
    TRY(rmm->CloseFile(attrcat));
    if (chdir("..") != 0) return SM_CHDIR_FAILED;
    return 0;
}

RC SM_Manager::CreateTable(const char *relName, int attrCount, AttrInfo *attributes) {
    RM_FileScan scan;
    RM_Record rec;
    TRY(scan.OpenScan(relcat, STRING, MAXNAME + 1, offsetof(RelCatEntry, relName),
                      EQ_OP, (void *)relName));
    if (scan.GetNextRec(rec) != RM_EOF) return SM_REL_EXISTS;
    TRY(scan.CloseScan());

    RID rid;
    short offset = 0;
    std::vector<short> nullableOffsets;
    for (int i = 0; i < attrCount; ++i) {
        AttrCatEntry attrEntry;
        memset(&attrEntry, 0, sizeof attrEntry);
        strcpy(attrEntry.relName, relName);
        strcpy(attrEntry.attrName, attributes[i].attrName);
        attrEntry.offset = offset;
        attrEntry.attrType = attributes[i].attrType;
        attrEntry.attrLength = attributes[i].attrLength;
        attrEntry.attrSpecs = attributes[i].attrSpecs;
        if (!(attrEntry.attrSpecs & ATTR_SPEC_NOTNULL)) {
            nullableOffsets.push_back(offset);
        }
        if (attributes[i].attrType == INT) {
            offset += sizeof(int);
        } else if (attributes[i].attrType == STRING) {
            // + 1 for terminating '\0'
            offset += upper_align<4>(attrEntry.attrLength + 1);
        } else {
            assert(false);
        }
        attrEntry.indexNo = -1;
        TRY(attrcat.InsertRec((const char *)&attrEntry, rid));
    }

    RelCatEntry relEntry;
    memset(&relEntry, 0, sizeof(RelCatEntry));
    strcpy(relEntry.relName, relName);
    relEntry.tupleLength = offset;
    relEntry.attrCount = attrCount;
    relEntry.indexCount = 0;
    TRY(relcat.InsertRec((const char *)&relEntry, rid));

    TRY(relcat.ForcePages());
    TRY(attrcat.ForcePages());

    TRY(rmm->CreateFile(relName, relEntry.tupleLength,
                        (short)nullableOffsets.size(), &nullableOffsets[0]));

    return 0;
}

RC SM_Manager::DropTable(const char *relName) {
    AttrCatEntry *attrEntry;
    RM_FileScan scan;
    RM_Record rec;
    RID rid;

    TRY(rmm->DestroyFile(relName));

    TRY(scan.OpenScan(attrcat, STRING, MAXNAME + 1, offsetof(AttrCatEntry, relName),
                      EQ_OP, (void *)relName));
    RC retcode;
    while ((retcode = scan.GetNextRec(rec)) != RM_EOF) {
        if (retcode) return retcode;
        TRY(rec.GetData((char *&)attrEntry));
        if (attrEntry->indexNo != -1)
            TRY(ixm->DestroyIndex(relName, attrEntry->indexNo));
        TRY(rec.GetRid(rid));
        TRY(attrcat.DeleteRec(rid));
    }
    TRY(scan.CloseScan());

    TRY(scan.OpenScan(relcat, STRING, MAXNAME + 1, offsetof(RelCatEntry, relName),
                      EQ_OP, (void *)relName));
    TRY(scan.GetNextRec(rec));
    TRY(rec.GetRid(rid));
    TRY(relcat.DeleteRec(rid));
    TRY(scan.CloseScan());

    TRY(relcat.ForcePages());
    TRY(attrcat.ForcePages());

    return 0;
}

RC SM_Manager::CreateIndex(const char *relName, const char *attrName) {
    RM_Record relRec, attrRec;
    RelCatEntry *relEntry;
    AttrCatEntry *attrEntry;

    TRY(GetRelCatEntry(relName, relRec));
    TRY(relRec.GetData((char *&)relEntry));
    TRY(GetAttrCatEntry(relName, attrName, attrRec));
    TRY(attrRec.GetData((char *&)attrEntry));
    if (attrEntry->indexNo != -1) return SM_INDEX_EXISTS;

    int indexNo = relEntry->indexCount;
    IX_IndexHandle indexHandle;
    RM_FileHandle fileHandle;
    RM_FileScan scan;
    RM_Record rec;
    TRY(ixm->CreateIndex(relName, indexNo, attrEntry->attrType, attrEntry->attrLength));
    TRY(rmm->OpenFile(relName, fileHandle));
    TRY(ixm->OpenIndex(relName, indexNo, indexHandle));
    TRY(scan.OpenScan(fileHandle, INT, sizeof(int), 0, NO_OP, NULL));
    RC retcode;
    while ((retcode = scan.GetNextRec(rec)) != RM_EOF) {
        if (retcode) return retcode;
        RID rid;
        char *data;
        TRY(rec.GetRid(rid));
        TRY(rec.GetData(data));
        TRY(indexHandle.InsertEntry(data + attrEntry->offset, rid));
    }
    TRY(scan.CloseScan());
    TRY(ixm->CloseIndex(indexHandle));
    TRY(rmm->CloseFile(fileHandle));

    attrEntry->indexNo = indexNo;
    ++relEntry->indexCount;
    TRY(relcat.UpdateRec(relRec));
    TRY(attrcat.UpdateRec(attrRec));

    TRY(relcat.ForcePages());
    TRY(attrcat.ForcePages());

    return 0;
}

RC SM_Manager::DropIndex(const char *relName, const char *attrName) {
    RM_Record attrRec;
    AttrCatEntry *attrEntry;

    TRY(GetAttrCatEntry(relName, attrName, attrRec));
    TRY(attrRec.GetData((char *&)attrEntry));
    if (attrEntry->indexNo == -1) return SM_INDEX_NOTEXIST;

    TRY(ixm->DestroyIndex(relName, attrEntry->indexNo));
    attrEntry->indexNo = -1;
    TRY(relcat.UpdateRec(attrRec));

    TRY(relcat.ForcePages());
    TRY(attrcat.ForcePages());

    return 0;
}

RC SM_Manager::Load(const char *relName, const char *fileName) {
    RelCatEntry relEntry;
    TRY(GetRelEntry(relName, relEntry));
    int attrCount;
    std::unique_ptr<DataAttrInfo[]> _attributes;
    TRY(GetDataAttrInfo(relName, attrCount, _attributes, true));
    DataAttrInfo *attributes = _attributes.get();

    RM_FileHandle fileHandle;
    RID rid;
    ARR_PTR(data, char, relEntry.tupleLength);
    ARR_PTR(indexHandles, IX_IndexHandle, attrCount);
    for (int i = 0; i < attrCount; ++i)
        if (attributes[i].indexNo != -1)
            TRY(ixm->OpenIndex(relName, attributes[i].indexNo, indexHandles[i]));
    TRY(rmm->OpenFile(relName, fileHandle));
    FILE *file = fopen(fileName, "r");
    if (!file) return SM_FILE_NOT_FOUND;

    char buffer[MAXATTRS * MAXSTRINGLEN + 1];
    int cnt = 0;
    while (!feof(file)) {
        fscanf(file, "%[^\n]\n", buffer);
        memset(data, 0, (size_t)relEntry.tupleLength);
        int p = 0, q = 0, l = (int)strlen(buffer);
        for (int i = 0; i < attrCount; ++i) {
            for (q = p; q < l && buffer[q] != ','; ++q);
            if (q == l && i != attrCount - 1) {
                std::cerr << cnt + 1 << ":" << q << " " << "too few columns" << std::endl;
                return SM_FILE_FORMAT_INCORRECT;
            }
            buffer[q] = 0;
            switch (attributes[i].attrType) {
                case INT: {
                    char *end = NULL;
                    *(int *)(data + attributes[i].offset) = strtol(buffer + p, &end, 10);
                    if (end == data + attributes[i].offset) {
                        std::cerr << cnt + 1 << ":" << q << " " << "incorrect integer" << std::endl;
                        return SM_FILE_FORMAT_INCORRECT;
                    }
                    break;
                }
                case FLOAT: {
                    char *end = NULL;
                    *(float *)(data + attributes[i].offset) = strtof(buffer + p, &end);
                    if (end == data + attributes[i].offset) {
                        std::cerr << cnt + 1 << ":" << q << " " << "incorrect float" << std::endl;
                        return SM_FILE_FORMAT_INCORRECT;
                    }
                    break;
                }
                case STRING: {
                    if (q - p > attributes[i].attrLength) {
                        std::cerr << cnt + 1 << ":" << q << " " << "string too long" << std::endl;
                        return SM_FILE_FORMAT_INCORRECT;
                    }
                    strcpy(data + attributes[i].offset, buffer + p);
                    break;
                }
            }
            p = q + 1;
        }
        TRY(fileHandle.InsertRec(data, rid));
        for (int i = 0; i < attrCount; ++i) {
            if (attributes[i].indexNo != -1)
                TRY(indexHandles[i].InsertEntry(data + attributes[i].offset, rid));
        }
        ++cnt;
    }

    for (int i = 0; i < attrCount; ++i)
        if (attributes[i].indexNo != -1)
            TRY(ixm->CloseIndex(indexHandles[i]))
    TRY(rmm->CloseFile(fileHandle));

    std::cout << cnt << " values loaded." << std::endl;

    return 0;
}

RC SM_Manager::Help() {
    return 0;
}

RC SM_Manager::Help(const char *relName) {
    int attrCount;
    std::unique_ptr<DataAttrInfo[]> _attributes;
    TRY(GetDataAttrInfo("attrcat", attrCount, _attributes));
    DataAttrInfo *attributes = _attributes.get();
    Printer printer(attributes, attrCount);
    TRY(GetDataAttrInfo(relName, attrCount, _attributes, true));
    attributes = _attributes.get();

    printer.PrintHeader(std::cout);
    for (int i = 0; i < attrCount; ++i)
        printer.Print(std::cout, (char *)&attributes[i], NULL);

    printer.PrintFooter(std::cout);

    return 0;
}

RC SM_Manager::Print(const char *relName) {
    int attrCount;
    std::unique_ptr<DataAttrInfo[]> _attributes;
    TRY(GetDataAttrInfo(relName, attrCount, _attributes, true));
    DataAttrInfo *attributes = _attributes.get();

    Printer printer(attributes, attrCount);
    printer.PrintHeader(std::cout);

    RM_FileHandle fileHandle;
    RM_FileScan scan;
    RM_Record rec;
    TRY(rmm->OpenFile(relName, fileHandle));
    TRY(scan.OpenScan(fileHandle, INT, sizeof(int), 0, NO_OP, NULL));
    RC retcode;
    while ((retcode = scan.GetNextRec(rec)) != RM_EOF) {
        if (retcode) return retcode;
        char *data;
        bool *isnull;
        TRY(rec.GetData(data));
        TRY(rec.GetIsnull(isnull));
        printer.Print(std::cout, data, isnull);
    }
    TRY(scan.CloseScan());
    TRY(rmm->CloseFile(fileHandle));

    printer.PrintFooter(std::cout);

    return 0;
}

RC SM_Manager::Set(const char *paramName, const char *value) {
    return 0;
}

RC SM_Manager::GetRelEntry(const char *relName, RelCatEntry &relEntry) {
    RM_Record rec;
    char *data;

    TRY(GetRelCatEntry(relName, rec));
    TRY(rec.GetData(data));
    relEntry = *(RelCatEntry *)data;

    return 0;
}

RC SM_Manager::GetAttrEntry(const char *relName, const char *attrName, AttrCatEntry &attrEntry) {
    RM_Record rec;
    char *data;

    TRY(GetAttrCatEntry(relName, attrName, rec));
    TRY(rec.GetData(data));
    attrEntry = *(AttrCatEntry *)data;

    return 0;
}

RC SM_Manager::GetRelCatEntry(const char *relName, RM_Record &rec) {
    RM_FileScan scan;

    TRY(scan.OpenScan(relcat, STRING, MAXNAME + 1, offsetof(RelCatEntry, relName),
                      EQ_OP, (void *)relName));
    RC retcode = scan.GetNextRec(rec);
    if (retcode == RM_EOF) return SM_REL_NOTEXIST;
    if (retcode) return retcode;
    TRY(scan.CloseScan());

    return 0;
}

RC SM_Manager::GetAttrCatEntry(const char *relName, const char *attrName, RM_Record &rec) {
    AttrCatEntry *attrEntry;
    RM_FileScan scan;
    RID rid;

    TRY(scan.OpenScan(attrcat, STRING, MAXNAME + 1, offsetof(AttrCatEntry, relName),
                      EQ_OP, (void *)relName));
    RC retcode;
    bool found = false;
    while ((retcode = scan.GetNextRec(rec)) != RM_EOF) {
        if (retcode) return retcode;
        TRY(rec.GetData((char *&)attrEntry));
        if (!strncmp(attrEntry->attrName, attrName, MAXNAME)) {
            found = true;
            break;
        }
    }
    TRY(scan.CloseScan());
    if (!found) return SM_ATTR_NOTEXIST;

    return 0;
}

RC SM_Manager::GetDataAttrInfo(const char *relName, int &attrCount, std::unique_ptr<DataAttrInfo[]> &_attributes, bool sort) {
    RM_FileScan scan;
    RM_Record rec;
    RID rid;
    RelCatEntry relEntry;
    AttrCatEntry *attrEntry;

    TRY(GetRelEntry(relName, relEntry));
    attrCount = relEntry.attrCount;
    ARR_PTR(attributes, DataAttrInfo, attrCount);

    TRY(scan.OpenScan(attrcat, STRING, MAXNAME + 1, offsetof(AttrCatEntry, relName),
                      EQ_OP, (void *)relName));
    RC retcode;
    int i = 0;
    while ((retcode = scan.GetNextRec(rec)) != RM_EOF) {
        if (retcode) return retcode;
        TRY(rec.GetData((char *&)attrEntry));
        strcpy(attributes[i].relName, attrEntry->relName);
        strcpy(attributes[i].attrName, attrEntry->attrName);
        attributes[i].offset = attrEntry->offset;
        attributes[i].attrType = attrEntry->attrType;
        attributes[i].attrSpecs = attrEntry->attrSpecs;
        attributes[i].attrLength = attrEntry->attrLength;
        attributes[i].indexNo = attrEntry->indexNo;
        ++i;
    }
    TRY(scan.CloseScan());
    if (i != attrCount) return SM_CATALOG_CORRUPT;

    if (sort) {
        std::sort(attributes, attributes + attrCount,
                  [](const DataAttrInfo &a, const DataAttrInfo &b) { return a.offset < b.offset; });
    }

    _attributes.reset(nullptr);
    _attributes = std::move(__attributes__);

    return 0;
}

#endif //REBASE_SM_MANAGER_H
