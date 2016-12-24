//
// rm.h
//
//   Record Manager component interface
//
// This file does not include the interface for the RID class.  This is
// found in rm_rid.h
//

#ifndef RM_H
#define RM_H

// Please DO NOT include any files other than redbase.h and pf.h in this
// file.  When you submit your code, the test program will be compiled
// with your rm.h and your redbase.h, along with the standard pf.h that
// was given to you.  Your rm.h, your redbase.h, and the standard pf.h
// should therefore be self-contained (i.e., should not depend upon
// declarations in any other file).

// Do not change the following includes
#include "redbase.h"
#include "rm_rid.h"
#include "pf.h"
#include "rm_internal.h"

#include <glog/logging.h>
#include <cstring>

//
// RM_Record: RM Record interface
//
class RM_Record {
    friend class RM_FileHandle;
    friend class RM_FileScan;

    char *pData;
    RID rid;
public:
    RM_Record ();
    RM_Record(const RM_Record&) = delete;
    ~RM_Record();

    RM_Record& operator=(const RM_Record&) = delete;

    void SetData(char *data, size_t size);
    
    // Return the data corresponding to the record.  Sets *pData to the
    // record contents.
    RC GetData(char *&pData) const;

    // Return the RID associated with the record
    RC GetRid (RID &rid) const;
};

//
// RM_FileHandle: RM File interface
//
class RM_FileHandle {
    friend class RM_Manager;
    friend class RM_FileScan;

    PF_FileHandle pfHandle;
    short recordSize;
    short recordsPerPage;
    int firstFreePage;
    short pageHeaderSize;
    bool isHeaderDirty;
public:
    RM_FileHandle ();
    ~RM_FileHandle();

    // Given a RID, return the record
    RC GetRec     (const RID &rid, RM_Record &rec) const;

    RC InsertRec  (const char *pData, RID &rid);       // Insert a new record

    RC DeleteRec  (const RID &rid);                    // Delete a record
    RC UpdateRec  (const RM_Record &rec);              // Update a record

    // Forces a page (along with any contents stored in this class)
    // from the buffer pool to disk.  Default value forces all pages.
    RC ForcePages (PageNum pageNum = ALL_PAGES);
};

//
// RM_FileScan: condition-based scan of records in the file
//
class RM_FileScan {
    const RM_FileHandle *fileHandle;
    AttrType attrType;
    int attrLength;
    int attrOffset;
    CompOp compOp;

    union {
        int intVal;
        float floatVal;
        char *stringVal;
    } value;

    bool scanOpened;
    PageNum currentPageNum;
    SlotNum currentSlotNum;
    short recordSize;

    bool checkSatisfy(char *data);
public:
    RM_FileScan  ();
    ~RM_FileScan ();

    RC OpenScan  (const RM_FileHandle &fileHandle,
                  AttrType   attrType,
                  int        attrLength,
                  int        attrOffset,
                  CompOp     compOp,
                  void       *value,
                  ClientHint pinHint = NO_HINT); // Initialize a file scan
    RC GetNextRec(RM_Record &rec);               // Get next matching record
    RC CloseScan ();                             // Close the scan
};

//
// RM_Manager: provides RM file management
//
class RM_Manager {
    PF_Manager *pfm;
public:
    RM_Manager    (PF_Manager &pfm);
    ~RM_Manager   ();

    RC CreateFile (const char *fileName, int recordSize);
    RC DestroyFile(const char *fileName);
    RC OpenFile   (const char *fileName, RM_FileHandle &fileHandle);

    RC CloseFile  (RM_FileHandle &fileHandle);
};

//
// Print-error function
//
void RM_PrintError(RC rc);

#define RM_FILE_NOT_OPENED      (START_RM_WARN + 0) // file is not opened
#define RM_SLOTNUM_OUT_OF_RANGE (START_RM_WARN + 1) // SlotNum < 0 || >= records/page
#define RM_RECORD_DELETED       (START_RM_WARN + 2) // record already deleted
#define RM_EOF                  (START_RM_WARN + 3)
#define RM_UNINITIALIZED_RECORD (START_RM_WARN + 4)
#define RM_UNINITIALIZED_RID    (START_RM_WARN + 5)
#define RM_SCAN_NOT_OPENED      (START_RM_WARN + 6)
#define RM_SCAN_NOT_CLOSED      (START_RM_WARN + 7)
#define RM_LASTWARN             RM_SCAN_NOT_CLOSED

#define RM_RECORDSIZE_TOO_LARGE (START_RM_ERR - 0) // record size larger than PF_PAGE_SIZE
#define RM_LASTERROR            RM_RECORDSIZE_TOO_LARGE

#endif
