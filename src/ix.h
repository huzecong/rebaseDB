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

class IX_IndexHandle;

//
// IX_Manager: provides IX index file management
//
class IX_Manager {
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

#define IX_SOMEWARNING    (START_IX_WARN + 0)  // cannot find key
#define IX_LASTWARN IX_SOMEWARNING


#define IX_SOMEERROR      (START_IX_ERR - 0)  // key size too big
#define IX_LASTERROR IX_SOMEERROR

#endif // IX_H
