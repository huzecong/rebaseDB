//
// sm.h
//   Data Manager Component Interface
//

#ifndef SM_H
#define SM_H

// Please do not include any other files than the ones below in this file.

#include <stdlib.h>
#include <string.h>
#include "redbase.h"  // Please don't change these lines
#include "rm.h"
#include "ix.h"
#include <string>
#include <map>
#include "parser.h"
#include "catalog.h"
#include "printer.h"

//
// SM_Manager: provides data management
//
class SM_Manager {
    friend class QL_Manager;

    RM_Manager *rmm;
    IX_Manager *ixm;

    RM_FileHandle relcat, attrcat;
public:
    SM_Manager    (IX_Manager &ixm_, RM_Manager &rmm_);
    ~SM_Manager   ();                             // Destructor

    RC OpenDb     (const char *dbName);           // Open the database
    RC CloseDb    ();                             // close the database

    RC CreateTable(const char *relName,           // create relation relName
                   int        attrCount,          //   number of attributes
                   AttrInfo   *attributes);       //   attribute data
    RC DropTable  (const char *relName);          // destroy a relation

    RC CreateIndex(const char *relName,           // create an index for
                   const char *attrName);         //   relName.attrName
    RC DropIndex  (const char *relName,           // destroy index on
                   const char *attrName);         //   relName.attrName

    RC Load       (const char *relName,           // load relName from
                   const char *fileName);         //   fileName

    RC Help       ();                             // Print relations in db
    RC Help       (const char *relName);          // print schema of relName

    RC Print      (const char *relName);          // print relName contents

    RC Set        (const char *paramName,         // set parameter to
                   const char *value);            //   value

    RC GetRelEntry(const char *relName, RelCatEntry &relEntry);
    RC GetAttrEntry(const char *relName, const char *attrName, AttrCatEntry &attrEntry);
    RC GetDataAttrInfo(const char *relName, int &attrCount, std::vector<DataAttrInfo> &attributes, bool sort = false);
    RC UpdateRelEntry(const char *relName, const RelCatEntry &relEntry);
    RC UpdateAttrEntry(const char *relName, const char *attrName, const AttrCatEntry &attrEntry);
private:
    RC GetRelCatEntry(const char *relName, RM_Record &rec);
    RC GetAttrCatEntry(const char *relName, const char *attrName, RM_Record &rec);
};

//
// Print-error function
//
void SM_PrintError(RC rc);


#define SM_REL_EXISTS            (START_SM_WARN + 0)
#define SM_REL_NOTEXIST          (START_SM_WARN + 1)
#define SM_ATTR_NOTEXIST         (START_SM_WARN + 2)
#define SM_INDEX_EXISTS          (START_SM_WARN + 3)
#define SM_INDEX_NOTEXIST        (START_SM_WARN + 4)
#define SM_FILE_FORMAT_INCORRECT (START_SM_WARN + 5)
#define SM_FILE_NOT_FOUND        (START_SM_WARN + 6)
#define SM_LASTWARN SM_FILE_NOT_FOUND


#define SM_CHDIR_FAILED    (START_SM_ERR - 0)
#define SM_CATALOG_CORRUPT (START_SM_ERR - 1)
#define SM_LASTERROR SM_CATALOG_CORRUPT

#endif // SM_H
