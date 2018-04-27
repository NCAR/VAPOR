#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <sstream>

#include <vapor/OptionParser.h>
#include <vapor/CFuncs.h>
#include <vapor/VDCNetCDF.h>
#include <vapor/DCCF.h>

using namespace Wasp;
using namespace VAPoR;

struct opt_t {
    int                     nthreads;
    int                     numts;
    std::vector<string>     vars;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"nthreads", 1, "0",
                                          "Specify number of execution threads "
                                          "0 => use number of cores"},
                                         {"numts", 1, "-1", "Number of timesteps to be included in the VDC. Default (-1) includes all timesteps."},
                                         {"vars", 1, "",
                                          "Colon delimited list of 3D variable names (compressed) "
                                          "to be included in "
                                          "the VDC"},
                                         {"help", 0, "", "Print this message and exit"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {{"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
                                        {"numts", Wasp::CvtToInt, &opt.numts, sizeof(opt.numts)},
                                        {"vars", Wasp::CvtToStrVec, &opt.vars, sizeof(opt.vars)},
                                        {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
                                        {NULL}};

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

#ifdef DEAD
// Copy a variable mask with 2 or 3 spatial dimensions
//
int CopyVar2d3dMask(DC &dc, VDC &vdc, size_t ts, string varname, int lod)
{
    vector<size_t> dimlens;
    bool           ok = dc.GetVarDimLens(varname, true, dimlens);
    assert(ok == true);

    if (dimlens.size() < 2) return (0);

    string maskvar;

    // Only data variables can have masks
    //
    if (!vdc.IsDataVar(varname)) return (0);

    DC::DataVar dvar;
    vdc.GetDataVarInfo(varname, dvar);
    maskvar = dvar.GetMaskvar();

    // Do nothing if mask variable already exists on disk
    //
    if (maskvar.empty() || vdc.VariableExists(ts, maskvar, 0, lod)) return (0);

    int fdr = dc.OpenVariableRead(ts, varname, -1, -1);
    if (fdr < 0) {
        MyBase::SetErrMsg("Failed to open variable %s for reading\n", varname.c_str());
        return (-1);
    }

    int fdw = vdc.OpenVariableWrite(ts, maskvar, lod);
    if (fdw < 0) {
        MyBase::SetErrMsg("Failed to open variable %s for writing\n", maskvar.c_str());
        return (-1);
    }

    size_t nz = dimlens.size() > 2 ? dimlens[dimlens.size() - 1] : 1;

    size_t         sz = dimlens[0] * dimlens[1];
    float *        buf = (float *)dataBuffer.Alloc(sz * sizeof(*buf));
    unsigned char *mask_buf = (unsigned char *)maskBuffer.Alloc(sz * sizeof(*mask_buf));

    dc.GetDataVarInfo(varname, dvar);
    double mv = dvar.GetMissingValue();

    // Generate a mask variable by comparing the data variable values
    // against the missing value
    //
    for (size_t i = 0; i < nz; i++) {
        int rc = dc.ReadSlice(fdr, buf);
        if (rc < 0) {
            MyBase::SetErrMsg("Failed to read variable %s\n", varname.c_str());
            return (-1);
        }

        for (int j = 0; j < sz; j++) {
            if (buf[j] == mv)
                mask_buf[j] = 0;    // invalid data
            else
                mask_buf[j] = 1;    // valid data
        }

        rc = vdc.WriteSlice(fdw, mask_buf);
        if (rc < 0) {
            MyBase::SetErrMsg("Failed to write variable %s\n", maskvar.c_str());
            return (-1);
        }
    }

    (void)dc.CloseVariable(fdr);
    int rc = vdc.CloseVariable(fdw);
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to write variable %s\n", maskvar.c_str());
        return (-1);
    }

    return (0);
}

#endif

int copyVarHelper(DC &dc, VDC &vdc, int fdr, int fdw, vector<size_t> &buffer_dims, vector<size_t> &src_hslice_dims, vector<size_t> &dst_hslice_dims, size_t src_nslice, size_t dst_nslice, double mv,
                  float *buffer)
{
    assert(buffer_dims.size() == src_hslice_dims.size());
    assert(buffer_dims.size() == dst_hslice_dims.size());

    size_t dim = buffer_dims.size() - 1;

    size_t src_slice_count = 0;
    size_t dst_slice_count = 0;
    while (src_slice_count < src_nslice) {
        float *bufptr = buffer;
        int    n = buffer_dims[dim] / src_hslice_dims[dim];

        for (int i = 0; i < n && src_slice_count < src_nslice; i++) {
            int rc = dc.ReadSlice(fdr, bufptr);
            if (rc < 0) return (-1);
            bufptr += vproduct(src_hslice_dims);

            src_slice_count++;
        }

        // In place replacmenet of missing value with 1-byte flag
        //
        size_t         sz = n * vproduct(src_hslice_dims);
        unsigned char *cptr = (unsigned char *)buffer;
        for (int j = 0; j < sz; j++) {
            if (buffer[j] == mv)
                cptr[j] = 0;    // invalid data
            else
                cptr[j] = 1;    // valid data
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

    if (argc < 3) {
        cerr << "Usage: " << ProgName << " cffiles... master.nc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        exit(1);
    }

    if (opt.help) {
        cerr << "Usage: " << ProgName << " master.nc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        exit(0);
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
    if (rc < 0) exit(1);

    DCCF dccf;
    rc = dccf.Initialize(cffiles, vector<string>());
    if (rc < 0) { exit(1); }

    vector<string> varnames = dccf.GetCoordVarNames();
    int            status = 0;
    for (int i = 0; i < varnames.size(); i++) {
        int nts = dccf.GetNumTimeSteps(varnames[i]);
        nts = opt.numts != -1 && nts > opt.numts ? opt.numts : nts;
        assert(nts >= 0);

        cout << "Copying variable " << varnames[i] << endl;

        for (int ts = 0; ts < nts; ts++) {
            cout << "  Time step " << ts << endl;
            int rc = vdc.CopyVar(dccf, ts, varnames[i], -1, -1);
            if (rc < 0) {
                MyBase::SetErrMsg("Failed to copy variable %s", varnames[i].c_str());
                status = 1;
            }
        }
    }

    if (opt.vars.size())
        varnames = opt.vars;
    else
        varnames = dccf.GetDataVarNames();

    for (int i = 0; i < varnames.size(); i++) {
        int nts = dccf.GetNumTimeSteps(varnames[i]);
        nts = opt.numts != -1 && nts > opt.numts ? opt.numts : nts;
        assert(nts >= 0);

        cout << "Copying variable " << varnames[i] << endl;

        for (int ts = 0; ts < nts; ts++) {
            cout << "  Time step " << ts << endl;

            int rc = CopyVar2d3dMask(dccf, vdc, ts, varnames[i], -1);
            if (rc < 0) {
                MyBase::SetErrMsg("Failed to copy variable %s", varnames[i].c_str());
                status = 1;
            }

            rc = vdc.CopyVar(dccf, ts, varnames[i], -1, -1);
            if (rc < 0) {
                MyBase::SetErrMsg("Failed to copy variable %s", varnames[i].c_str());
                status = 1;
            }
            if (rc < 0) exit(1);
        }
    }

    return (status);
}
