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

// Copy a variable mask with 2 or 3 spatial dimensions
//
int CopyVar2d3dMask(DC &dc, VDC &vdc, const vector<size_t> &dimlens, size_t ts, string varname, int lod)
{
    assert(dimlens.size() == 2 || dimlens.size() == 3);

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

    int rc = dc.OpenVariableRead(ts, varname, -1, -1);
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to open variable %s for reading\n", varname.c_str());
        return (-1);
    }

    rc = vdc.OpenVariableWrite(ts, maskvar, lod);
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to open variable %s for writing\n", maskvar.c_str());
        return (-1);
    }

    size_t nz = dimlens.size() > 2 ? dimlens[dimlens.size() - 1] : 1;

    size_t         sz = dimlens[0] * dimlens[1];
    float *        buf = (float *)dataBuffer.Alloc(sz * sizeof(*buf));
    unsigned char *mask_buf = (unsigned char *)dataBuffer.Alloc(sz * sizeof(*mask_buf));

    dc.GetDataVarInfo(varname, dvar);
    double mv = dvar.GetMissingValue();

    // Generate a mask variable by comparing the data variable values
    // against the missing value
    //
    for (size_t i = 0; i < nz; i++) {
        rc = dc.ReadSlice(buf);
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

        int rc = vdc.WriteSlice(mask_buf);
        if (rc < 0) {
            MyBase::SetErrMsg("Failed to write variable %s\n", maskvar.c_str());
            return (-1);
        }
    }

    (void)dc.CloseVariable();
    rc = vdc.CloseVariable();
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to write variable %s\n", maskvar.c_str());
        return (-1);
    }

    return (0);
}

// Copy a variable with 2 or 3 spatial dimensions
//
int CopyVar2d3d(DC &dc, VDC &vdc, const vector<size_t> &dimlens, size_t ts, string varname, int lod)
{
    assert(dimlens.size() == 2 || dimlens.size() == 3);

    int rc = CopyVar2d3dMask(dc, vdc, dimlens, ts, varname, lod);
    if (rc < 0) return (-1);

    rc = dc.OpenVariableRead(ts, varname, -1, -1);
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to open variable %s for reading\n", varname.c_str());
        return (-1);
    }

    rc = vdc.OpenVariableWrite(ts, varname, lod);
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to open variable %s for writing\n", varname.c_str());
        return (-1);
    }

    size_t nz = dimlens.size() > 2 ? dimlens[dimlens.size() - 1] : 1;

    size_t sz = dimlens[0] * dimlens[1];
    float *buf = (float *)dataBuffer.Alloc(sz * sizeof(*buf));

    for (size_t i = 0; i < nz; i++) {
        rc = dc.ReadSlice(buf);
        if (rc < 0) {
            MyBase::SetErrMsg("Failed to read variable %s\n", varname.c_str());
            return (-1);
        }

        int rc = vdc.WriteSlice(buf);
        if (rc < 0) {
            MyBase::SetErrMsg("Failed to write variable %s\n", varname.c_str());
            return (-1);
        }
    }

    (void)dc.CloseVariable();
    rc = vdc.CloseVariable();
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to write variable %s\n", varname.c_str());
        return (-1);
    }

    return (0);
}

// Copy a variable with 0 or 1 spatial dimensions
//
int CopyVar0d1d(DC &dc, VDC &vdc, const vector<size_t> &dimlens, size_t ts, string varname, int lod)
{
    assert(dimlens.size() == 0 || dimlens.size() == 1);

    size_t sz = 1;
    for (int i = 0; i < dimlens.size(); i++) { sz *= dimlens[i]; }

    float *buf = (float *)dataBuffer.Alloc(sz * sizeof(*buf));

    int rc = dc.GetVar(ts, varname, -1, -1, buf);
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to read variable %s\n", varname.c_str());
        return (-1);
    }

    rc = vdc.PutVar(ts, varname, lod, buf);
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to write variable %s\n", varname.c_str());
        return (-1);
    }

    return (0);
}

int CopyVar(DC &dc, VDC &vdc, size_t ts, string varname, int lod)
{
    vector<size_t> dimlens;
    bool           ok = dc.GetVarDimLens(varname, false, dimlens);
    assert(ok == true);

    if (dc.IsTimeVarying(varname)) { dimlens.pop_back(); }
    assert(dimlens.size() > 0 && dimlens.size() < 4);

    // Call appropriate copy method based on number of spatial dimensions
    //
    if (dimlens.size() == 0 || dimlens.size() == 1) {
        return (CopyVar0d1d(dc, vdc, dimlens, ts, varname, lod));
    } else {
        return (CopyVar2d3d(dc, vdc, dimlens, ts, varname, lod));
    }
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

    size_t chunksize = 1024 * 1024 * 4;
    int    rc = vdc.Initialize(master, VDC::A, chunksize);
    if (rc < 0) exit(1);

    DCCF dccf;
    rc = dccf.Initialize(cffiles);
    if (rc < 0) { exit(1); }

    // Necessary????
    //
    //	rc = vdc.EndDefine();
    //	if (rc<0) {
    //		MyBase::SetErrMsg(
    //			"Failed to write VDC master file : %s", master.c_str()
    ////		);
    //		exit(1);
    //	}

    vector<string> varnames = dccf.GetCoordVarNames();
    for (int i = 0; i < varnames.size(); i++) {
        int nts = dccf.GetNumTimeSteps(varnames[i]);
        nts = opt.numts != -1 && nts > opt.numts ? opt.numts : nts;
        assert(nts >= 0);

        cout << "Copying variable " << varnames[i] << endl;

        for (int ts = 0; ts < nts; ts++) {
            cout << "  Time step " << ts << endl;
            int rc = CopyVar(dccf, vdc, ts, varnames[i], -1);
            if (rc < 0) exit(1);
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

            int rc = CopyVar(dccf, vdc, ts, varnames[i], -1);
            if (rc < 0) exit(1);
        }
    }

    return (0);
}
