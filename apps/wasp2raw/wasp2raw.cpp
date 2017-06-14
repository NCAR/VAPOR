#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cerrno>
#include <stdio.h>
#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/WASP.h>

using namespace Wasp;
using namespace VAPoR;

//
//	Command line argument stuff
//
struct opt_t {
    string varname;
    string type;
    int lod;
    int level;
    int nthreads;
    vector<int> start;
    vector<int> count;
    OptionParser::Boolean_T debug;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {
    {"varname", 1, "var1", "Name of variable"},
    {"type", 1, "float32", "Primitive type of output data"},
    {"lod", 1, "-1", "Compression levels saved. 0 => coarsest, 1 => "
                     "next refinement, etc. -1 => all levels defined by the netcdf file"},
    {"level", 1, "-1", "Multiresolution refinement level. Zero implies coarsest "
                       "resolution"},
    {"start", 1, "", "Colon-delimited NetCDF style start coordinate vector"},
    {"count", 1, "", "Colon-delimited NetCDF style count coordinate vector"},
    {"nthreads", 1, "0", "Number of execution threads. 0 => use number of cores"},
    {"debug", 0, "", "Enable diagnostic"},
    {"help", 0, "", "Print this message and exit"},
    {NULL}};

OptionParser::Option_T get_options[] = {
    {"varname", Wasp::CvtToCPPStr, &opt.varname, sizeof(opt.varname)},
    {"type", Wasp::CvtToCPPStr, &opt.type, sizeof(opt.type)},
    {"lod", Wasp::CvtToInt, &opt.lod, sizeof(opt.lod)},
    {"level", Wasp::CvtToInt, &opt.level, sizeof(opt.level)},
    {"start", Wasp::CvtToIntVec, &opt.start, sizeof(opt.start)},
    {"count", Wasp::CvtToIntVec, &opt.count, sizeof(opt.count)},
    {"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
    {"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
    {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
    {NULL}};

const char *ProgName;

template <class T>
void CopyVar(string ncdffile, string datafile, T dummy) {

    WASP wasp(opt.nthreads);

    int rc = wasp.Open(ncdffile, NC_WRITE);

    vector<size_t> dims;
    vector<size_t> bs;
    rc = wasp.InqVarDimlens(opt.varname, opt.level, dims, bs);
    if (rc < 0)
        exit(1);

    rc = wasp.OpenVarRead(opt.varname, opt.level, opt.lod);
    if (rc < 0)
        exit(1);

    vector<size_t> start(dims.size(), 0);
    if (opt.start.size()) {
        if (opt.start.size() != dims.size()) {
            MyBase::SetErrMsg("Invalid start specification");
            exit(1);
        }
        for (int i = 0; i < opt.start.size(); i++) {
            start[i] = opt.start[i];
        }
    }

    vector<size_t> count = dims;
    if (opt.count.size()) {
        if (opt.count.size() != dims.size()) {
            MyBase::SetErrMsg("Invalid count specification");
            exit(1);
        }
        for (int i = 0; i < opt.count.size(); i++) {
            count[i] = opt.count[i];
        }
    }

    size_t nelements = 1;
    for (int i = 0; i < count.size(); i++)
        nelements *= count[i];

    T *data = new T[nelements];

    rc = wasp.GetVara(start, count, data);
    if (rc < 0)
        exit(1);

    rc = wasp.CloseVar();
    if (rc < 0)
        exit(1);

    rc = wasp.Close();
    if (rc < 0)
        exit(1);

    FILE *fp = fopen(datafile.c_str(), "w");
    if (!fp) {
        MyBase::SetErrMsg("fopen(%s) : %M", datafile.c_str());
        exit(1);
    }

    rc = fwrite(data, sizeof(*data), nelements, fp);
    if (rc != nelements) {
        MyBase::SetErrMsg("fread() : %M");
        exit(1);
    }
}

int main(int argc, char **argv) {

    OptionParser op;

    MyBase::SetErrMsgFilePtr(stderr);

    //
    // Parse command line arguments
    //
    ProgName = Basename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) {
        exit(1);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        exit(1);
    }

    if (opt.help) {
        cerr << "Usage: " << ProgName << " [options] netcdffile datafile" << endl;
        op.PrintOptionHelp(stderr);
        exit(0);
    }

    if (argc != 3) {
        cerr << "Usage: " << ProgName << " [options] netcdffile datafile" << endl;
        op.PrintOptionHelp(stderr);
        exit(1);
    }

    string ncdffile = argv[1]; // Path to a vdf file
    string datafile = argv[2]; // Path to raw data file

    if (opt.debug)
        MyBase::SetDiagMsgFilePtr(stderr);

    if (opt.type == "float32") {
        float dummy = 0.0;
        CopyVar(ncdffile, datafile, dummy);
    } else if (opt.type == "float64") {
        double dummy = 0.0;
        CopyVar(ncdffile, datafile, dummy);
    } else if (opt.type == "int32") {
        int dummy = 0;
        CopyVar(ncdffile, datafile, dummy);
    } else if (opt.type == "int16") {
        int16_t dummy = 0;
        CopyVar(ncdffile, datafile, dummy);
    } else {
        cerr << "Invalid type " << opt.type << endl;
    }

    return (0);
}
