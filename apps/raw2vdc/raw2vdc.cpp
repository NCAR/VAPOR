#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cerrno>
#include <stdio.h>
#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/VDCNetCDF.h>

using namespace Wasp;
using namespace VAPoR;

//
//	Command line argument stuff
//
struct opt_t {
    string                  varname;
    string                  type;
    int                     lod;
    int                     nthreads;
    int                     ts;
    OptionParser::Boolean_T swapbytes;
    OptionParser::Boolean_T debug;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"varname", 1, "var1", "Name of variable"},
                                         {"type", 1, "float32", "Primitive type of input data"},
                                         {"lod", 1, "-1",
                                          "Compression levels saved. 0 => coarsest, 1 => "
                                          "next refinement, etc. -1 => all levels defined by the netcdf file"},
                                         {"nthreads", 1, "0",
                                          "Specify number of execution threads "
                                          "0 => use number of cores"},
                                         {"ts", 1, "0", "Specify time step offset"},
                                         {"swapbytes", 0, "", "Swap bytes in data as they are read from disk"},
                                         {"debug", 0, "", "Enable diagnostic"},
                                         {"help", 0, "", "Print this message and exit"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {{"varname", Wasp::CvtToCPPStr, &opt.varname, sizeof(opt.varname)},
                                        {"type", Wasp::CvtToCPPStr, &opt.type, sizeof(opt.type)},
                                        {"lod", Wasp::CvtToInt, &opt.lod, sizeof(opt.lod)},
                                        {"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
                                        {"ts", Wasp::CvtToInt, &opt.ts, sizeof(opt.ts)},
                                        {"swapbytes", Wasp::CvtToBoolean, &opt.swapbytes, sizeof(opt.swapbytes)},
                                        {"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
                                        {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
                                        {NULL}};

size_t size_of_type(string type)
{
    if (type.compare("float32") == 0) return (4);
    if (type.compare("float64") == 0) return (8);
    if (type.compare("int8") == 0) return (1);
    return (1);
}

void swapbytes(void *vptr, int size, int n)
{
    unsigned char *ucptr = (unsigned char *)vptr;
    unsigned char  uc;
    int            i, j;

    for (j = 0; j < n; j++) {
        for (i = 0; i < size / 2; i++) {
            uc = ucptr[i];
            ucptr[i] = ucptr[size - i - 1];
            ucptr[size - i - 1] = uc;
        }
        ucptr += size;
    }
}

int read_data(FILE * fp,
              string type,    // element type
              bool swap, size_t n, float *slice)
{
    static SmartBuf smart_buf;

    size_t         element_sz = size_of_type(type);
    unsigned char *buffer = (unsigned char *)smart_buf.Alloc(n * element_sz);

    int rc = fread(buffer, element_sz, n, fp);
    if (rc != n) {
        if (rc < 0) {
            MyBase::SetErrMsg("Error reading input file : %M");
        } else {
            MyBase::SetErrMsg("Short read on input file");
        }
        return (-1);
    }

    // Swap bytes in place if needed
    //
    if (swap) { swapbytes(buffer, element_sz, n); }

    float *dptr = slice;
    if (type.compare("float32") == 0) {
        float *sptr = (float *)buffer;
        for (size_t i = 0; i < n; i++) { *dptr++ = (float)*sptr++; }
    } else if (type.compare("float64") == 0) {
        double *sptr = (double *)buffer;
        for (size_t i = 0; i < n; i++) { *dptr++ = (float)*sptr++; }
    } else if (type.compare("int8") == 0) {
        char *sptr = (char *)buffer;
        for (size_t i = 0; i < n; i++) { *dptr++ = (float)*sptr++; }
    }

    return (0);
}

const char *ProgName;

int main(int argc, char **argv)
{
    OptionParser op;

    MyBase::SetErrMsgFilePtr(stderr);

    //
    // Parse command line arguments
    //
    ProgName = Basename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) { exit(1); }

    if (op.ParseOptions(&argc, argv, get_options) < 0) { exit(1); }

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

    string master = argv[1];      // Path to VDC master file
    string datafile = argv[2];    // Path to raw data file

    if (opt.debug) MyBase::SetDiagMsgFilePtr(stderr);

    VDCNetCDF vdc(opt.nthreads);

    int rc = vdc.Initialize(master, VDC::A, 4 * 1024 * 1024);

    vector<size_t> sdims;
    if (!vdc.GetVarDimLens(opt.varname, true, sdims)) {
        MyBase::SetErrMsg("VDC corrupt, invalid dimensions");
        exit(1);
    }

    if (sdims.size() < 1) {
        MyBase::SetErrMsg("Variable must be 1D, 2D or 3D");
        exit(1);
    }

    size_t nelements = sdims[0];
    if (sdims.size() > 1) nelements *= sdims[1];

    float *slice = new float[nelements];

    FILE *fp = fopen(datafile.c_str(), "r");
    if (!fp) {
        MyBase::SetErrMsg("fopen(%s) : %M", datafile.c_str());
        exit(1);
    }

    rc = vdc.OpenVariableWrite(opt.ts, opt.varname, opt.lod);
    if (rc < 0) exit(1);

    size_t nz = sdims.size() > 2 ? sdims[2] : 1;

    for (size_t i = 0; i < nz; i++) {
        int rc = read_data(fp, opt.type, opt.swapbytes, nelements, slice);
        if (rc < 0) exit(1);

        rc = vdc.WriteSlice(slice);
        if (rc < 0) exit(1);
    }

    rc = vdc.CloseVariable();
    if (rc < 0) exit(1);

    fclose(fp);

    exit(0);
}
