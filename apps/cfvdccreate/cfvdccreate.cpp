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
    OptionParser::Dimension3D_T dim;
    std::vector<size_t>         bs;
    std::vector<size_t>         cratios;
    string                      wname;
    int                         nthreads;
    std::vector<string>         vars;
    OptionParser::Boolean_T     force;
    OptionParser::Boolean_T     help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"dimension", 1, "512x512x512",
                                          "Data volume dimensions expressed in "
                                          "grid points (NXxNYxNZ)"},
                                         {"bs", 1, "64:64:64", "Internal storage blocking factor expressed in grid points (NX:NY:NZ)"},
                                         {"cratios", 1, "1:10:100:500",
                                          "Colon delimited list compression ratios. "
                                          "for 3D variables. The default is 1:10:100:500. The maximum "
                                          "compression ratio is wavelet and block size dependent."},
                                         {"wname", 1, "bior4.4",
                                          "Wavelet family used for compression "
                                          "Valid values are bior1.1, bior1.3, "
                                          "bior1.5, bior2.2, bior2.4 ,bior2.6, bior2.8, bior3.1, bior3.3, "
                                          "bior3.5, bior3.7, bior3.9, bior4.4"},
                                         {"nthreads", 1, "0",
                                          "Specify number of execution threads "
                                          "0 => use number of cores"},
                                         {"vars", 1, "",
                                          "Colon delimited list of 3D variable names (compressed) "
                                          "to be included in "
                                          "the VDC"},
                                         {"force", 0, "",
                                          "Create a new VDC master file even if a VDC data "
                                          "directory already exists. Results may be undefined if settings between "
                                          "the new master file and old data directory do not match."},
                                         {"help", 0, "", "Print this message and exit"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {{"dimension", Wasp::CvtToDimension3D, &opt.dim, sizeof(opt.dim)},
                                        {"bs", Wasp::CvtToSize_tVec, &opt.bs, sizeof(opt.bs)},
                                        {"cratios", Wasp::CvtToSize_tVec, &opt.cratios, sizeof(opt.cratios)},
                                        {"wname", Wasp::CvtToCPPStr, &opt.wname, sizeof(opt.wname)},
                                        {"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
                                        {"vars", Wasp::CvtToStrVec, &opt.vars, sizeof(opt.vars)},
                                        {"force", Wasp::CvtToBoolean, &opt.force, sizeof(opt.force)},
                                        {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
                                        {NULL}};

string ProgName;

// Construct a mask variable name
//
void maskvar(vector<string> dimnames, string &name)
{
    assert(dimnames.size() >= 1);
    name.clear();

    name = "mask";
    for (int i = 0; i < dimnames.size(); i++) { name += "_" + dimnames[i]; }
}

void DefineMaskVars(const DCCF &dccf, VDCNetCDF &vdc)
{
    // Find all coordinate combinations for data with missing values
    //
    vector<pair<string, vector<string>>> dimpairs;
    for (int d = 1; d < 4; d++) {
        vector<string> datanames = dccf.DC::GetDataVarNames(d, true);

        for (int i = 0; i < datanames.size(); i++) {
            DC::DataVar dvar;
            dccf.GetDataVarInfo(datanames[i], dvar);

            // skip if no missing value defined for this variable
            //
            if (!dvar.GetHasMissing()) continue;

            // Assume missing values locations don't vary over time!
            //
            vector<string> dimnames;
            bool           ok = dccf.GetVarDimNames(datanames[i], true, dimnames);
            assert(ok);

            string maskvar_name;
            maskvar(dimnames, maskvar_name);

            pair<string, vector<string>> p1 = make_pair(maskvar_name, dimnames);

            dimpairs.push_back(p1);
        }
    }
    sort(dimpairs.begin(), dimpairs.end());
    vector<pair<string, vector<string>>>::iterator last;
    last = unique(dimpairs.begin(), dimpairs.end());
    dimpairs.erase(last, dimpairs.end());

    for (int i = 0; i < dimpairs.size(); i++) {
        string         maskvar = dimpairs[i].first;
        vector<string> dimnames = dimpairs[i].second;

        //
        // 1D coordinates are not blocked
        //
        string         mywname;
        vector<size_t> mybs;
        bool           compress;
        if (dimnames.size() < 2) {
            mywname.clear();
            mybs.clear();
            compress = false;
        } else {
            mywname = "intbior2.2";
            mybs = opt.bs;
            compress = true;
        }

        // Try to compute "reasonable" 1D & 2D compression ratios from 3D
        // compression ratios
        //
        vector<size_t> cratios(1, 1);

        int rc = vdc.SetCompressionBlock(mybs, mywname, cratios);
        if (rc < 0) exit(1);

        rc = vdc.DefineDataVar(maskvar, dimnames, dimnames, "", DC::INT8, compress);

        if (rc < 0) { exit(1); }
    }
}

void defineMapProjection(const DCCF &dcwrf, VDCNetCDF &vdc)
{
    string proj4string;
    for (int d = 2; d < 4 && proj4string.empty(); d++) {
        vector<string> varnames = dcwrf.DC::GetDataVarNames(d, true);

        for (int i = 0; i < varnames.size(); i++) {
            string proj4string = dcwrf.GetMapProjection(varnames[i]);
            if (!proj4string.empty()) {
                vdc.SetMapProjection(proj4string);
                break;
            }
        }
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

    if (op.AppendOptions(set_opts) < 0) { return (1); }

    if (op.ParseOptions(&argc, argv, get_options) < 0) { return (1); }

    if (argc < 3) {
        cerr << "Usage: " << ProgName << " cf_files... master.nc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        return (1);
    }

    if (opt.help) {
        cerr << "Usage: " << ProgName << " cf_files... master.nc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        return (0);
    }

    argc--;
    argv++;
    vector<string> cffiles;
    for (int i = 0; i < argc - 1; i++) cffiles.push_back(argv[i]);
    string master = argv[argc - 1];

    VDCNetCDF vdc(opt.nthreads);

    if (vdc.DataDirExists(master) && !opt.force) {
        MyBase::SetErrMsg("Data directory exists and -force option not used. "
                          "Remove directory %s or use -force",
                          vdc.GetDataDir(master).c_str());
        return (1);
    }

    size_t chunksize = 1024 * 1024 * 4;
    int    rc = vdc.Initialize(master, VDC::W, chunksize);
    if (rc < 0) return (1);

    DCCF dccf;
    rc = dccf.Initialize(cffiles);
    if (rc < 0) { return (1); }

    vector<string> dimnames = dccf.GetDimensionNames();
    for (int i = 0; i < dimnames.size(); i++) {
        DC::Dimension dim;
        dccf.GetDimension(dimnames[i], dim);
        rc = vdc.DefineDimension(dim.GetName(), dim.GetLength());
        if (rc < 0) { return (1); }
    }

    // Make the default block dimension 64 for any missing dimensions
    //
    vector<size_t> bs = opt.bs;
    for (int i = bs.size(); i < 3; i++) bs.push_back(64);

    //
    // Define coordinate variables
    //
    for (int d = 0; d < 4; d++) {
        vector<string> coordnames = dccf.DC::GetCoordVarNames(d, true);

        //
        // Time coordinate and 1D coordinates are not blocked
        //
        vector<size_t> mybs;
        if (d < 2) {
            mybs.clear();
        } else {
            mybs = opt.bs;
        }

        vector<size_t> cratios(1, 1);

        rc = vdc.SetCompressionBlock(mybs, opt.wname, cratios);
        if (rc < 0) return (1);

        for (int i = 0; i < coordnames.size(); i++) {
            DC::CoordVar cvar;
            dccf.GetCoordVarInfo(coordnames[i], cvar);

            vector<string> sdimnames;
            string         time_dimname;

            bool ok = dccf.GetVarDimNames(coordnames[i], sdimnames, time_dimname);
            assert(ok);

            if (cvar.GetUniform()) {
                rc = vdc.DefineCoordVarUniform(cvar.GetName(), sdimnames, time_dimname, cvar.GetUnits(), cvar.GetAxis(), cvar.GetXType(), false);
            } else {
                rc = vdc.DefineCoordVar(cvar.GetName(), sdimnames, time_dimname, cvar.GetUnits(), cvar.GetAxis(), cvar.GetXType(), false);
            }

            if (rc < 0) { return (1); }
        }
    }

    DefineMaskVars(dccf, vdc);

    defineMapProjection(dccf, vdc);

    //
    // Define data variables
    //
    for (int d = 0; d < 4; d++) {
        vector<string> datanames = dccf.DC::GetDataVarNames(d, true);

        //
        // Time coordinate and 1D coordinates are not blocked
        //
        string         mywname;
        vector<size_t> mybs;
        bool           compress;
        if (d < 2) {
            mywname.clear();
            mybs.clear();
            compress = false;
        } else {
            mywname = opt.wname;
            mybs = opt.bs;
            compress = true;
        }

        // Try to compute "reasonable" 1D & 2D compression ratios from 3D
        // compression ratios
        //
        vector<size_t> cratios = opt.cratios;
        for (int i = 0; i < cratios.size(); i++) {
            size_t c = (size_t)pow((double)cratios[i], (double)((float)d / 3.0));
            cratios[i] = c;
        }

        rc = vdc.SetCompressionBlock(mybs, mywname, cratios);
        if (rc < 0) return (1);

        for (int i = 0; i < datanames.size(); i++) {
            DC::DataVar dvar;
            dccf.GetDataVarInfo(datanames[i], dvar);

            vector<string> dimnames;
            bool           ok = dccf.GetVarDimNames(datanames[i], false, dimnames);
            assert(ok);

            vector<string> coordvars;
            ok = dccf.GetVarCoordVars(datanames[i], false, coordvars);
            assert(ok);

            if (!dvar.GetHasMissing() || !compress) {
                rc = vdc.DefineDataVar(dvar.GetName(), dimnames, coordvars, dvar.GetUnits(), dvar.GetXType(), compress);
            } else {
                vector<string> sdimnames;
                bool           ok = dccf.GetVarDimNames(datanames[i], true, sdimnames);
                assert(ok);

                string maskvar_name;
                maskvar(sdimnames, maskvar_name);

                rc = vdc.DefineDataVar(dvar.GetName(), dimnames, coordvars, dvar.GetUnits(), dvar.GetXType(), dvar.GetMissingValue(), maskvar_name);
            }

            if (rc < 0) { return (1); }
        }
    }

    rc = vdc.EndDefine();
    if (rc < 0) {
        MyBase::SetErrMsg("Failed to write VDC master file : %s", master.c_str());
        return (1);
    }

    return (0);
}
