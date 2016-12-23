//
// File:        sm_error.cc
// Description: SM_PrintError function
//

#include <cerrno>
#include <cstdio>
#include <iostream>
#include "sm.h"

using namespace std;

//
// Error table
//
static const char *SM_WarnMsg[] = {
		"relation already exists",
		"relation does not exist",
		"attribute does not exist for given relation",
        "index already exists for given attribute",
        "index does not exist for given attribute",
        "file to load has incorrect format",
        "file not found",
        "length of string-typed attribute should not exceed MAXSTRINGLEN=255"
};

static const char *SM_ErrorMsg[] = {
		"chdir command execution failed",
        "database catalog file is corrupt",
};

//
// SM_PrintError
//
// Desc: Send a message corresponding to a SM return code to cerr
// In:   rc - return code for which a message is desired
//
void SM_PrintError(RC rc) {
	// Check the return code is within proper limits
	if (rc >= START_SM_WARN && rc <= SM_LASTWARN)
		// Print warning
		cerr << "SM warning: " << SM_WarnMsg[rc - START_SM_WARN] << "\n";
		// Error codes are negative, so invert everything
	else if ((-rc >= -START_SM_ERR) && -rc <= -SM_LASTERROR) {
		// Print error
		cerr << "SM error: " << SM_ErrorMsg[-rc + START_SM_ERR] << "\n";
	} else if (rc == 0)
		cerr << "SM_PrintError called with return code of 0\n";
	else {
		// Print error
		cerr << "rc was " << rc << endl;
		cerr << "START_SM_ERR was " << START_SM_ERR << endl;
		cerr << "SM_LASTERROR was " << SM_LASTERROR << endl;
		cerr << "SM error: " << rc << " is out of bounds\n";
	}
}
