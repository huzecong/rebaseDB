//
// redbase.cc
//
// Author: Jason McHugh (mchughj@cs.stanford.edu)
//
// This shell is provided for the student.

#include <iostream>
#include "redbase.h"
#include "rm.h"
#include "sm.h"
#include "ql.h"

#include <glog/logging.h>

using namespace std;

PF_Manager pfm;
RM_Manager rmm(pfm);
IX_Manager ixm(pfm);
SM_Manager smm(ixm, rmm);
QL_Manager qlm(smm, ixm, rmm);


//
// main
//
int main(int argc, char *argv[]) {
    FLAGS_logtostderr = 1;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    RBparse(pfm, smm, qlm);

    cout << "Bye.\n";
}
