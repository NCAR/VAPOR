#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/DataMgr.h>
#include <vapor/FileUtils.h>
#include <vapor/utils.h>

#include "vapor/VAssert.h"
#include "gridTools.h"
#include "dataMgrTools.h"

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
    std::vector<std::string> &options
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

/*void PrintDimensions( DataMgr &dataMgr ) {
    vector <string> dimnames;
    dimnames = dataMgr.GetDimensionNames();
    cout << "Dimensions:" << endl;
    for (int i=0; i<dimnames.size(); i++) {
        DC::Dimension dimension;
        dataMgr.GetDimension(dimnames[i], dimension);
        cout << "\t" << dimension.GetName() << " = " << dimension.GetLength() << endl;
        cout << "\t Time Varying: " << dimension.IsTimeVarying() << endl;
    }
    cout << endl;
}

void PrintMeshes( DataMgr &dataMgr, bool verbose=false ) {
    vector <string> meshnames;
    cout << "Meshes:" << endl;
    meshnames = dataMgr.GetMeshNames();
    for (int i=0; i<meshnames.size(); i++) {
        cout << "\t" << meshnames[i] << endl;
        if (verbose) {
            DC::Mesh mesh;
            dataMgr.GetMesh(meshnames[i], mesh);
            cout << mesh;
        }
    }
    cout << endl;
}

void PrintCoordVariables( DataMgr &dataMgr ) {
    std::vector<std::string> coordVars = dataMgr.GetCoordVarNames();
    for (int i=0; i<coordVars.size(); i++) {
        cout << coordVars[i] << endl;
    }
    cout << endl;
}

void PrintTimeCoordinates( DataMgr &dataMgr ) {
    std::vector<double> timeCoords = dataMgr.GetTimeCoordinates();
    cout << "Time Coordinates:" << endl;
    for (int i=0; i<timeCoords.size(); i++) {
        cout << "\t" << timeCoords[i] << endl;
    }
    cout << endl;
}

void PrintVariables( 
    DataMgr &dataMgr, 
    bool verbose  = false,
    bool testVars = false
 ) {
    vector <string> vars;

    for (int d=1; d<4; d++) {
        cout << d << "D variables: " << endl;;
        vars = dataMgr.GetDataVarNames(d);
        for (int i=0; i<vars.size(); i++) {
            cout << "  " << vars[i] << endl;
            if (verbose) {
                DC::DataVar datavar;
                dataMgr.GetDataVarInfo(vars[i], datavar);
                cout << datavar;// << endl;
            }
            //if (testVars) {
            if (i==0) {
                std::vector< double > minExt, maxExt;
                dataMgr.GetVariableExtents( 0, vars[i], -1, -1, minExt, maxExt );
                // Reduce extents to test
                for (int i=0; i<minExt.size(); i++) {  
                    minExt[i] /= 32.0;
                    maxExt[i] /= 32.0;
                }

                Grid* grid = dataMgr.GetVariable( 0, vars[i], -1, -1, minExt, maxExt );
                //double rms              =0;
                //size_t numMissingValues =0;
                //size_t disagreements    =0;
                double rms;
                size_t numMissingValues;
                size_t disagreements;
                //CompareIndexToCoords( grid, rms, numMissingValues, disagreements, false );
                //CompareIndexToCoords( grid, rms, numMissingValues, disagreements );
                Foo(grid, rms, numMissingValues, disagreements);
                cout << "Grid test for variable " << vars[i] << ":" << endl;
                //PrintStats( rms, numMissingValues, disagreements );
            }
            cout << endl;
        }
        cout << endl;
    }
}

int TestWRF( 
    std::vector< std::string > &files, 
    std::vector< std::string > &options 
) {
    DataMgr dataMgr(opt.fileType, opt.memsize, opt.nthreads);
    int rc = dataMgr.Initialize(files, options);
    if (rc<0) {
        cout << "Failed to intialize WRF DataMGR" << endl;
        return -1;
    }

    PrintDimensions( dataMgr );
    PrintMeshes( dataMgr );
    PrintVariables( dataMgr);//, true );
    //PrintVariables( dataMgr, true );
    PrintCoordVariables( dataMgr );
    PrintTimeCoordinates( dataMgr );

    return 0;
}*/

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

    TestWRF( opt.fileType, opt.memsize, opt.nthreads, files, options );

    cout << "Elapsed time: " << Wasp::GetTime() - t0 << endl;
}
