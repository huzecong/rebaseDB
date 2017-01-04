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

DEFINE_string(c, "-",
        "batch mode rather than interactive mode.\n" \
        "    reads commands from <file>\n" \
        "    use '-' (without quotes) to use interactive mode.");

DECLARE_bool(n);

using namespace std;

PF_Manager pfm;
RM_Manager rmm(pfm);
IX_Manager ixm(pfm);
SM_Manager smm(ixm, rmm);
QL_Manager qlm(smm, ixm, rmm);

extern FILE* yyin;
extern bool output_prompt;

//
// main
//
int main(int argc, char *argv[]) {
    FLAGS_logtostderr = 1;
    gflags::ParseCommandLineFlags(&argc, &argv, true);
    google::InitGoogleLogging(argv[0]);

    if (FLAGS_c != "-") {
        yyin = fopen(FLAGS_c.c_str(), "r");
        CHECK(yyin != NULL) << "cannot open " << FLAGS_c;
        output_prompt = false;
    } else {
        output_prompt = true;
    }

    RBparse(pfm, smm, qlm);

    if (yyin != stdin) {
        fclose(yyin);
    }

    cout << "Bye.\n";
}
