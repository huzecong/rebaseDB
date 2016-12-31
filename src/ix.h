//
// ix.h
//
//   Index Manager Component Interface
//

#ifndef IX_H
#define IX_H

// Please do not include any other files than the ones below in this file.

#include "redbase.h"
#include "rm_rid.h"
#include "pf.h"
#include "rm.h"

#include <memory>
#include <glog/logging.h>

class IX_IndexHandle;

//
// IX_Manager: provides IX index file management
//
class IX_Manager {
    PF_Manager *pfm;
    // RM_Manager rmm;

public:
    IX_Manager   (PF_Manager &pfm);              // Constructor
    ~IX_Manager  ();                             // Destructor
    RC CreateIndex  (const char *fileName,          // Create new index
                     int        indexNo,
                     AttrType   attrType,
                     int        attrLength);
    RC DestroyIndex (const char *fileName,          // Destroy index
                     int        indexNo);
    RC OpenIndex    (const char *fileName,          // Open index
                     int        indexNo,
                     IX_IndexHandle &indexHandle);
    RC CloseIndex   (IX_IndexHandle &indexHandle);  // Close index
};

//
// IX_IndexHandle: IX Index File interface
//
class IX_IndexHandle {
    friend class IX_Manager;
    friend class IX_IndexScan;

    PF_FileHandle pfHandle;
    // RM_FileHandle rmHandle;

    AttrType attrType;
    int attrLength;
    int root;
    int firstFreePage;

    bool isHeaderDirty;

    // use attrType and attrLength to calculate the
    // internal parameters
    void __initialize();

    int b; // branch factor
    int entrySize;

    int __cmp(void* lhs, void* rhs) const;
    inline void* __get_entry(void* base, int n) const {
        return (void*)((char*)base + entrySize * n);
    }

    RC new_node(int *nodeNum);
    RC delete_node(int nodeNum);

    RC insert_internal_entry(void *header, int index, void* key, int node);
    RC insert_entry(void *header, void* pData, const RID &rid);
    RC insert_internal(int nodeNum, int *splitNode, std::unique_ptr<char[]> *splitKey, void *pData, const RID &rid);
    RC insert_leaf(int nodeNum, int *splitNode, void *pData, const RID &rid);

public:
    IX_IndexHandle  ();                             // Constructor
    ~IX_IndexHandle ();                             // Destructor
    RC InsertEntry     (void *pData, const RID &rid);  // Insert new index entry
    RC DeleteEntry     (void *pData, const RID &rid);  // Delete index entry
    RC ForcePages      ();                             // Copy index to disk
};

//
// IX_IndexScan: condition-based scan of index entries
//
class IX_IndexScan {
    const IX_IndexHandle *indexHandle;
    CompOp compOp;
    void* value;

    bool scanOpened;
    int currentNodeNum;
    int currentEntryIndex;

    bool __check(void* key);

public:
    IX_IndexScan  ();                                 // Constructor
    ~IX_IndexScan ();                                 // Destructor
    RC OpenScan      (const IX_IndexHandle &indexHandle, // Initialize index scan
                      CompOp      compOp,
                      void        *value,
                      ClientHint  pinHint = NO_HINT);
    RC GetNextEntry  (RID &rid);                         // Get next matching entry
    RC CloseScan     ();                                 // Terminate index scan
};

//
// Print-error function
//
void IX_PrintError(RC rc);

#define IX_EOF                  (START_IX_WARN + 0)
#define IX_KEY_EXISTS           (START_IX_WARN + 1)
#define IX_SCAN_NOT_OPENED      (START_IX_WARN + 2)
#define IX_SCAN_NOT_CLOSED      (START_IX_WARN + 2)
#define IX_LASTWARN IX_SCAN_NOT_CLOSED


#define IX_ATTR_TOO_LARGE       (START_IX_ERR - 0)
#define IX_LASTERROR IX_ATTR_TOO_LARGE

#endif // IX_H
