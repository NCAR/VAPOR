#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <sstream>

#include <vapor/OptionParser.h>
#include <vapor/CFuncs.h>
#include <vapor/VDCNetCDF.h>
#include <vapor/DCCF.h>
#include <vapor/FileUtils.h>

using namespace Wasp;
using namespace VAPoR;

struct opt_t {
    int                     nthreads;
    int                     numts;
    std::vector<string>     vars;
    std::vector<string>     xvars;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"nthreads", 1, "0",
                                          "Specify number of execution threads "
                                          "0 => use number of cores"},
                                         {"numts", 1, "-1", "Number of timesteps to be included in the VDC. Default (-1) includes all timesteps."},
                                         {"vars", 1, "",
                                          "Colon delimited list of variable names "
                                          "to be copied to the VDC"},
                                         {"xvars", 1, "",
                                          "Colon delimited list of variable names "
                                          "to exclude from copying the VDC"},
                                         {"help", 0, "", "Print this message and exit"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {{"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)}, {"numts", Wasp::CvtToInt, &opt.numts, sizeof(opt.numts)},
                                        {"vars", Wasp::CvtToStrVec, &opt.vars, sizeof(opt.vars)},          {"xvars", Wasp::CvtToStrVec, &opt.xvars, sizeof(opt.xvars)},
                                        {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},         {NULL}};

string ProgName;

SmartBuf dataBuffer;
SmartBuf maskBuffer;

// Product of elements in a vector
//
size_t vproduct(vector<size_t> a)
{
    size_t ntotal = 1;

    for (int i = 0; i < a.size(); i++) ntotal *= a[i];
    return (ntotal);
}

size_t gcd(size_t n1, size_t n2)
{
    size_t tmp;
    while (n2 != 0) {
        tmp = n1;
        n1 = n2;
        n2 = tmp % n2;
    }
    return n1;
}

size_t lcm(size_t n1, size_t n2) { return ((n1 * n2) / gcd(n1, n2)); }

int copyVarHelper(DC &dc, VDC &vdc, int fdr, int fdw, vector<size_t> &buffer_dims, vector<size_t> &src_hslice_dims, vector<size_t> &dst_hslice_dims, size_t src_nslice, size_t dst_nslice, double mv,
                  float *buffer)
{
    VAssert(buffer_dims.size() == src_hslice_dims.size());
    VAssert(buffer_dims.size() == dst_hslice_dims.size());

    size_t dim = buffer_dims.size() - 1;

    size_t src_slice_count = 0;
    size_t dst_slice_count = 0;
    while (src_slice_count < src_nslice) {
        float *bufptr = buffer;
        int    n = buffer_dims[dim] / src_hslice_dims[dim];

        int rCount;
        for (rCount = 0; rCount < n && src_slice_count < src_nslice; rCount++) {
            int rc = dc.ReadSlice(fdr, bufptr);
            if (rc < 0) return (-1);
            bufptr += vproduct(src_hslice_dims);

            src_slice_count++;
        }

        // In place replacmenet of missing value with 1-byte flag
        //
        size_t         sz = rCount * vproduct(src_hslice_dims);
        unsigned char *cptr = (unsigned char *)buffer;
        for (int j = 0; j < sz; j++) {
            if (buffer[j] == mv) {
                cptr[j] = 0;    // invalid data
            } else {
                cptr[j] = 1;    // valid data
            }
        }

        cptr = (unsigned char *)buffer;
        n = buffer_dims[dim] / dst_hslice_dims[dim];

        for (int i = 0; i < n && dst_slice_count < dst_nslice; i++) {
            int rc = vdc.WriteSlice(fdw, cptr);
            if (rc < 0) return (-1);

            cptr += vproduct(dst_hslice_dims);

            dst_slice_count++;
        }
    }
    return (0);
}

int CopyVar2d3dMask(DC &dc, VDC &vdc, size_t ts, string varname, int lod)
{
    // Only data variables can have masks
    //
    if (!vdc.IsDataVar(varname)) return (0);

    DC::DataVar varInfo;
    bool        status = vdc.GetDataVarInfo(varname, varInfo);
    if (!status) {
        MyBase::SetErrMsg("Invalid destination variable name : %s", varname.c_str());
        return (-1);
    }

    string maskvar = varInfo.GetMaskvar();

    // Do nothing if mask variable already exists on disk
    //
    if (maskvar.empty() || vdc.VariableExists(ts, maskvar, 0, lod)) return (0);

    status = dc.GetDataVarInfo(varname, varInfo);
    if (!status) {
        MyBase::SetErrMsg("Invalid source variable name : %s", varname.c_str());
        return (-1);
    }
    double mv = varInfo.GetMissingValue();

    // Get the dimensions of a hyper slice for the source and destination
    // varible
    //
    vector<size_t> src_hslice_dims;
    size_t         src_nslice;
    int            rc = dc.GetHyperSliceInfo(varname, -1, src_hslice_dims, src_nslice);
    if (rc < 0) return (rc);

    if (src_hslice_dims.size() < 2) return (0);

    vector<size_t> dst_hslice_dims;
    size_t         dst_nslice;
    rc = vdc.GetHyperSliceInfo(varname, -1, dst_hslice_dims, dst_nslice);
    if (rc < 0) return (rc);

    if (src_hslice_dims.size() != dst_hslice_dims.size()) {
        MyBase::SetErrMsg("Incompatible source and destination variable definitions");
        return (-1);
    }

    // n-1 fastest varying dimensions must be the same for both hyper-slices.
    // Slowest dimension may be different.
    //
    int    dim = src_hslice_dims.size() - 1;
    size_t src_dimlen = src_hslice_dims[dim];
    size_t dst_dimlen = dst_hslice_dims[dim];

    for (int i = 0; i < src_hslice_dims.size() - 1; i++) {
        if (src_hslice_dims[i] != dst_hslice_dims[i]) {
            MyBase::SetErrMsg("Incompatible source and destination variable definitions");
            return (-1);
        }
    }

    // Find the slice dimension for slowest varying dimension, the Least
    // Common Multiple for the source and destination
    //
    size_t slice_dim = lcm(src_dimlen, dst_dimlen);

    // Common (fastest-varying) dimensions for both variables, plus
    // the lcm of the slowest varying dimension for the source
    // and destination.
    //
    vector<size_t> buffer_dims = src_hslice_dims;
    buffer_dims.pop_back();    // Remove slowest varying dimension
    buffer_dims.push_back(slice_dim);

    int fdr = dc.OpenVariableRead(ts, varname, -1);
    if (fdr < 0) return (fdr);

    int fdw = vdc.OpenVariableWrite(ts, maskvar, lod);
    if (fdw < 0) return (fdw);

    size_t bufsize = vproduct(buffer_dims);
    float *buffer = (float *)dataBuffer.Alloc(bufsize * sizeof(*buffer));

    rc = copyVarHelper(dc, vdc, fdr, fdw, buffer_dims, src_hslice_dims, dst_hslice_dims, src_nslice, dst_nslice, mv, buffer);

    dc.CloseVariable(fdr);
    vdc.CloseVariable(fdw);

    return (rc);
}

// Return a new vector containing elements of v1 with any elements from
// v2 removed
//
vector<string> remove_vector(vector<string> v1, vector<string> v2)
{
    vector<string> newvec;
    for (auto it = v1.begin(); it != v1.end(); ++it) {
        if (find(v2.begin(), v2.end(), *it) == v2.end()) { newvec.push_back(*it); }
    }
    return (newvec);
}

int main(int argc, char **argv)
{
    OptionParser op;

    MyBase::SetErrMsgFilePtr(stderr);

    //
    // Parse command line arguments
    //
    ProgName = FileUtils::LegacyBasename(argv[0]);

    if (op.AppendOptions(set_opts) < 0) { return (1); }

    if (op.ParseOptions(&argc, argv, get_options) < 0) { return (1); }

    if (argc < 3) {
        cerr << "Usage: " << ProgName << " cffiles... master.vdc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        return (1);
    }

    if (opt.help) {
        cerr << "Usage: " << ProgName << " master.vdc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        return (0);
    }

    argc--;
    argv++;
    vector<string> cffiles;
    for (int i = 0; i < argc - 1; i++) cffiles.push_back(argv[i]);
    string master = argv[argc - 1];

    VDCNetCDF vdc(opt.nthreads);

    size_t         chunksize = 1024 * 1024 * 4;
    vector<size_t> bs;
    int            rc = vdc.Initialize(master, vector<string>(), VDC::A, bs, chunksize);
    if (rc < 0) return (1);

    DCCF dccf;
    rc = dccf.Initialize(cffiles, vector<string>());
    if (rc < 0) { return (1); }

    vector<string> varnames = dccf.GetCoordVarNames();
    for (int i = 0; i < varnames.size(); i++) {
        int nts = dccf.GetNumTimeSteps(varnames[i]);
        nts = opt.numts != -1 && nts > opt.numts ? opt.numts : nts;
        VAssert(nts >= 0);

        cout << "Copying variable " << varnames[i] << endl;

        for (int ts = 0; ts < nts; ts++) {
            cout << "  Time step " << ts << endl;
            int rc = vdc.CopyVar(dccf, ts, varnames[i], -1, -1);
            if (rc < 0) {
                MyBase::SetErrMsg("Failed to copy variable %s", varnames[i].c_str());
                return (1);
            }
        }
    }

    if (opt.vars.size()) {
        varnames = opt.vars;
    } else {
        varnames = dccf.GetDataVarNames();
    }

    varnames = remove_vector(varnames, opt.xvars);

    int estatus = 0;
    for (int i = 0; i < varnames.size(); i++) {
        int nts = dccf.GetNumTimeSteps(varnames[i]);
        nts = opt.numts != -1 && nts > opt.numts ? opt.numts : nts;
        VAssert(nts >= 0);

        cout << "Copying variable " << varnames[i] << endl;

        for (int ts = 0; ts < nts; ts++) {
            cout << "  Time step " << ts << endl;

            int rc = CopyVar2d3dMask(dccf, vdc, ts, varnames[i], -1);
            if (rc < 0) {
                MyBase::SetErrMsg("Failed to copy variable %s", varnames[i].c_str());
                continue;
                estatus = 1;
            }

            rc = vdc.CopyVar(dccf, ts, varnames[i], -1, -1);
            if (rc < 0) {
                MyBase::SetErrMsg("Failed to copy variable %s", varnames[i].c_str());
                estatus = 1;
            }
        }
    }

    return (estatus);
}
