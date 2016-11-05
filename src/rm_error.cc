//
// Created by Kanari on 2016/10/17.
//

#include <cerrno>
#include <cstdio>
#include <iostream>
#include "rm.h"

using namespace std;

//
// Error table
//
static char *RM_WarnMsg[] = {
	(char *)"file is not opened",
	(char *)"SlotNum is out of range",
	(char *)"record is already deleted",
	(char *)"no more records in scan",
	(char *)"Record is not properly initialized",
	(char *)"RID is not properly initialized",
	(char *)"scan is not opened",
	(char *)"last opened scan is not closed"
};

static char *RM_ErrorMsg[] = {
	(char *)"recordSize is too large for current pagefile system"
};

void RM_PrintError(RC rc) {
	// Check the return code is within proper limits
	if (rc >= START_RM_WARN && rc <= RM_LASTWARN)
		// Print warning
		cerr << "RM warning: " << RM_WarnMsg[rc - START_RM_WARN] << "\n";
		// Error codes are negative, so invert everything
	else if (-rc >= -START_RM_ERR && -rc < -RM_LASTERROR)
		// Print error
		cerr << "RM error: " << RM_ErrorMsg[-rc + START_RM_ERR] << "\n";
	else if (rc == 0)
		cerr << "RM_PrintError called with return code of 0\n";
	else
		PF_PrintError(rc);
}
