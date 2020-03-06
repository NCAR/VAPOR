#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include "vapor/VAssert.h"

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/DataMgr.h>
#include <vapor/FileUtils.h>
#include <vapor/utils.h>

using namespace Wasp;
using namespace VAPoR;

struct {
    std::string fileType;
    int memsize;
    int nthreads;
    OptionParser::Boolean_T nogeoxform;
    OptionParser::Boolean_T novertxform;
    OptionParser::Boolean_T makeBaseline;
    std::string baselineFile;
} opt;

OptionParser::OptDescRec_T  set_opts[] = {
    {"fileType",   1,  "vdc",  "data set type (vdc|wrf|cf|mpas)"},
    {"memsize", 1,  "2000","Cache size in MBs"},
    {"nthreads",    1,  "0",    "Specify number of execution threads "
        "0 => use number of cores"},
    {"nogeoxform",  0,  "", "Do not apply geographic transform (projection to PCS"},
    {"novertxform", 0,  "", "Do not apply to convert pressure, etc. to meters"},
    {"makeBaseline", 0, "", "Make output of this test the new baseline that will "
        "be used to compare future tests with"},
    {"baselineFile", 0, "dataMgrTests.txt", "Specifies the file that will be "
        "compared against the current test results"},
    {nullptr}
};

OptionParser::Option_T get_options[] = {
    {"fileType", Wasp::CvtToCPPStr, &opt.fileType, sizeof(opt.fileType)},
    {"memsize", Wasp::CvtToInt, &opt.memsize, sizeof(opt.memsize)},
    {"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
    {"nogeoxform", Wasp::CvtToBoolean, &opt.nogeoxform, sizeof(opt.nogeoxform)},
    {"novertxform", Wasp::CvtToBoolean, &opt.novertxform, sizeof(opt.novertxform)},
    {"makeBaseline", Wasp::CvtToBoolean, &opt.makeBaseline, sizeof(opt.makeBaseline)},
    {"baselineFile", Wasp::CvtToCPPStr, &opt.baselineFile, sizeof(opt.baselineFile)},
    {nullptr}
};

void InitializeOptions( 
    int &argc, 
    char **argv, 
    OptionParser &op,
    std::vector<std::string> &files,
    std::vector<std::string> &options,
) {
    std::string ProgName = FileUtils::LegacyBasename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (argc < 2) {
        cerr << "Usage: " << ProgName << " [options] vdcmaster " << endl;
        op.PrintOptionHelp(stderr);
        exit(1);
    }

    for (int i=1; i<argc; i++) {
        files.push_back(argv[i]);
    }

    if (! opt.nogeoxform) {
        options.push_back("-project_to_pcs");
    }
    if (! opt.novertxform) {
        options.push_back("-vertical_xform");
    }
}

int TestWRF( 
    std::vector< std::string > &files, 
    std::vector< std::string > &options 
) {
    DataMgr datamgr(opt.fileType, opt.memsize, opt.nthreads);
    int rc = datamgr.Initialize(files, options);
    if (rc<0) {
        cout << "Failed to intialize WRF DataMGR!!!" << endl;
        return -1;
    }

    return 0;
}

int main( int argc, char** argv ) {
    double t0 = Wasp::GetTime();

    MyBase::SetErrMsgFilePtr(stderr);

    OptionParser op;
    std::vector<string> files;
    std::vector<string> options;
    InitializeOptions( argc, argv, op, files, options );

    for (int i=0; i<files.size(); i++ ) {
        cout << files[i] << endl;
    }

    TestWRF( files, options );

    cout << "Elapsed time: " << Wasp::GetTime() - t0 << endl;
}
