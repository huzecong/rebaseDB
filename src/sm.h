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

//
// SM_Manager: provides data management
//
class SM_Manager {
	friend class QL_Manager;
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
};

//
// Print-error function
//
void SM_PrintError(RC rc);

#define SM_SOMEWARNING    (START_SM_WARN + 0)  // cannot find key
#define SM_LASTWARN SM_SOMEWARNING


#define SM_SOMEERROR      (START_SM_ERR - 0)  // key size too big
#define SM_LASTERROR SM_SOMEERROR

#endif // SM_H
