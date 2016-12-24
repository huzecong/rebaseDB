//
// dbcreate.cc
//
// Author: Jason McHugh (mchughj@cs.stanford.edu)
//
// This shell is provided for the student.

#include "rm.h"
#include "sm.h"

#include <unistd.h>
#include <stddef.h>
#include <iostream>

using namespace std;

//int isDir(const char *file){
//    struct stat buf;
//    stat(file, &buf);
//    return S_ISREG(buf.st_mode);
//}

//
// main
//
int main(int argc, char *argv[]) {
    char *dbname;
    char command[255] = "mkdir ";
    RC rc;

    // Look for 2 arguments. The first is always the name of the program
    // that was executed, and the second should be the name of the
    // database.
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " dbname \n";
        exit(1);
    }

    // The database name is the second argument
    dbname = argv[1];
    if (strlen(argv[1]) > (sizeof(command) - strlen(command) - 1)) {
        cerr << argv[1] << " length exceeds maximum allowed, cannot create database\n";
        exit(1);
    }

    // Create a subdirectory for the database
    int error = system(strcat(command, dbname));

    if (error) {
        cerr << "system call to create directory exited with error code " << error << endl;
        exit(1);
    }

    if (chdir(dbname) < 0) {
        cerr << argv[0] << " chdir error to " << dbname << "\n";
        exit(1);
    }

    // Create the system catalogs...

    PF_Manager pfm;
    RM_Manager rmm(pfm);

    // Calculate the size of entries in relcat:
    int relCatRecSize = sizeof(RelCatEntry);
    int attrCatRecSize = sizeof(AttrCatEntry);

    if ((rc = rmm.CreateFile("relcat", relCatRecSize))) {
        cerr << "Trouble creating relcat. Exiting" << endl;
        exit(1);
    }

    if ((rc = rmm.CreateFile("attrcat", attrCatRecSize))) {
        cerr << "Trouble creating attrcat. Exiting" << endl;
        exit(1);
    }

    // Adding relation metadata of relcat and attrcat
    const char *(relName[MAXNAME + 1]) = {"relcat", "attrcat"};
    const char *(attrName[MAXNAME + 1]) = {
            "relName", "tupleLength", "attrCount", "indexCount",
            "relName", "attrName", "offset", "attrType", "attrLength", "indexNo"
    };

    RM_FileHandle handle;
    RID rid;
    rmm.OpenFile("relcat", handle);
    RelCatEntry relEntry;

    memcpy(relEntry.relName, relName[0], MAXNAME + 1);
    relEntry.tupleLength = sizeof(RelCatEntry);
    relEntry.attrCount = 4;
    relEntry.indexCount = 0;
    handle.InsertRec((const char *)&relEntry, rid);

    memcpy(relEntry.relName, relName[1], MAXNAME + 1);
    relEntry.tupleLength = sizeof(AttrCatEntry);
    relEntry.attrCount = 6;
    relEntry.indexCount = 0;
    handle.InsertRec((const char *)&relEntry, rid);

    rmm.CloseFile(handle);

    // Adding attribute metadata of relcat and attrcat
    rmm.OpenFile("attrcat", handle);
    AttrCatEntry attrEntry;
    attrEntry.indexNo = -1;

    memcpy(attrEntry.relName, relName[0], MAXNAME + 1);
    memcpy(attrEntry.attrName, attrName[0], MAXNAME + 1);
    attrEntry.offset = offsetof(RelCatEntry, relName);
    attrEntry.attrType = STRING;
    attrEntry.attrLength = MAXNAME + 1;
    handle.InsertRec((const char *)&attrEntry, rid);
    memcpy(attrEntry.attrName, attrName[1], MAXNAME + 1);
    attrEntry.offset = offsetof(RelCatEntry, tupleLength);
    attrEntry.attrType = INT;
    attrEntry.attrLength = sizeof(int);
    handle.InsertRec((const char *)&attrEntry, rid);
    memcpy(attrEntry.attrName, attrName[2], MAXNAME + 1);
    attrEntry.offset = offsetof(RelCatEntry, attrCount);
    handle.InsertRec((const char *)&attrEntry, rid);
    memcpy(attrEntry.attrName, attrName[3], MAXNAME + 1);
    attrEntry.offset = offsetof(RelCatEntry, indexCount);
    handle.InsertRec((const char *)&attrEntry, rid);

    memcpy(attrEntry.relName, relName[1], MAXNAME + 1);
    memcpy(attrEntry.attrName, attrName[4], MAXNAME + 1);
    attrEntry.offset = offsetof(AttrCatEntry, relName);
    attrEntry.attrType = STRING;
    attrEntry.attrLength = MAXNAME + 1;
    handle.InsertRec((const char *)&attrEntry, rid);
    memcpy(attrEntry.attrName, attrName[5], MAXNAME + 1);
    attrEntry.offset = offsetof(AttrCatEntry, attrName);
    handle.InsertRec((const char *)&attrEntry, rid);
    memcpy(attrEntry.attrName, attrName[6], MAXNAME + 1);
    attrEntry.offset = offsetof(AttrCatEntry, offset);
    attrEntry.attrType = INT;
    attrEntry.attrLength = sizeof(int);
    handle.InsertRec((const char *)&attrEntry, rid);
    memcpy(attrEntry.attrName, attrName[7], MAXNAME + 1);
    attrEntry.offset = offsetof(AttrCatEntry, attrType);
    handle.InsertRec((const char *)&attrEntry, rid);
    memcpy(attrEntry.attrName, attrName[8], MAXNAME + 1);
    attrEntry.offset = offsetof(AttrCatEntry, attrLength);
    handle.InsertRec((const char *)&attrEntry, rid);
    memcpy(attrEntry.attrName, attrName[9], MAXNAME + 1);
    attrEntry.offset = offsetof(AttrCatEntry, indexNo);
    handle.InsertRec((const char *)&attrEntry, rid);

    rmm.CloseFile(handle);

    return (0);
}
