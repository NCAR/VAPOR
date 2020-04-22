// This test exercises the following DataMgr functions, and prints their
// results.  These results can be captured and compared to previous results
// as part of an automated test for reviewing Pull Requests.
//
// Functions under test:
//
//  DataMGr::DataMgr(string format, size_t mem_size, int nthreads = 0)
//  DataMgr::GetDimensionNames()
//  DataMgr::GetDimension( string dimname, DC:Dimension &dimension)
//  DataMgr::GetMeshNames()
//  DataMgr::GetMesh(string meshname, DC::Mesh &mesh)
//  DataMgr::GetDataVarNames(int ndim)
//  DataMgr::GetCoordVarNames()
//  DataMgr::GetTimeCoordinates()
//  DataMgr::GetTimeCoordVarName()
//  DataMgr::GetNumTimeSteps()
//  DataMgr::GetDataVarInfo(string varname, VAPoR::DC::DataVar &datavar)
//  DataMgr::GetNumRefLevels(string varname)
//  DataMgr::GetCRatios(string varname)
//  DataMgr::GetVariable(
//    size_t ts, string varname, int level, int lod, bool lock=false
//  )
//  DataMgr::GetVariableExtents(
//    size_t ts, string varname, int level, int lod,
//    std::vector <double> &min , std::vector <double> &max
//  )
//  DataMgr::GetDimLens(string varname, std::vector <size_t> &dims)
//  DataMgr::GetNumDimensions( string varname )
//  DataMgr::GetVarTopologyDim( sting varname )
//  DataMgr::GetMapProjection()

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
    std::string             fileType;
    int                     memsize;
    int                     nthreads;
    OptionParser::Boolean_T nogeoxform;
    OptionParser::Boolean_T novertxform;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"fileType", 1, "vdc", "data set type (vdc|wrf|cf|mpas)"},
                                         {"memsize", 1, "2000", "Cache size in MBs"},
                                         {"nthreads", 1, "0",
                                          "Specify number of execution threads "
                                          "0 => use number of cores"},
                                         {"nogeoxform", 0, "", "Do not apply geographic transform (projection to PCS"},
                                         {"novertxform", 0, "", "Do not apply to convert pressure, etc. to meters"},
                                         {nullptr}};

OptionParser::Option_T get_options[] = {{"fileType", Wasp::CvtToCPPStr, &opt.fileType, sizeof(opt.fileType)},
                                        {"memsize", Wasp::CvtToInt, &opt.memsize, sizeof(opt.memsize)},
                                        {"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
                                        {"nogeoxform", Wasp::CvtToBoolean, &opt.nogeoxform, sizeof(opt.nogeoxform)},
                                        {"novertxform", Wasp::CvtToBoolean, &opt.novertxform, sizeof(opt.novertxform)},
                                        {nullptr}};

void InitializeOptions(int &argc, char **argv, OptionParser &op, std::vector<std::string> &files, std::vector<std::string> &options)
{
    std::string ProgName = FileUtils::LegacyBasename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(EXIT_FAILURE);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(EXIT_FAILURE);
    }

    if (argc < 2) {
        cerr << "Usage: " << ProgName << " [options] dataFile " << endl;
        op.PrintOptionHelp(stderr);
        exit(EXIT_FAILURE);
    }

    for (int i = 1; i < argc; i++) { files.push_back(argv[i]); }

    if (!opt.nogeoxform) { options.push_back("-project_to_pcs"); }
    if (!opt.novertxform) { options.push_back("-vertical_xform"); }
}

int main(int argc, char **argv)
{
    double t0 = Wasp::GetTime();

    MyBase::SetErrMsgFilePtr(stderr);

    OptionParser        op;
    std::vector<string> files;
    std::vector<string> options;
    InitializeOptions(argc, argv, op, files, options);

    for (int i = 0; i < files.size(); i++) {
        std::string fileName = files[i];
        fileName = fileName.substr(fileName.find_last_of("\\/") + 1);
        cout << fileName << endl;
    }

    int rc = TestDataMgr(opt.fileType, opt.memsize, opt.nthreads, files, options);

    cout << "Elapsed time: " << Wasp::GetTime() - t0 << endl;

    return rc;
}
