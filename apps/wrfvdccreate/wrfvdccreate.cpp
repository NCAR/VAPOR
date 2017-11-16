#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <sstream>

#include <vapor/OptionParser.h>
#include <vapor/CFuncs.h>
#include <vapor/VDCNetCDF.h>
#include <vapor/DCWRF.h>

using namespace Wasp;
using namespace VAPoR;

struct opt_t {
    std::vector<size_t> bs;
    std::vector<size_t> cratios;
    string wname;
    int nthreads;
    std::vector<string> vars;
    OptionParser::Boolean_T force;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {
    {"bs", 1, "64:64:64",
     "Internal storage blocking factor expressed in grid points (NX:NY:NZ)"},
    {"cratios", 1, "1:10:100:500", "Colon delimited list compression ratios. "
                                   "for 3D variables. The default is 1:10:100:500. The maximum "
                                   "compression ratio is wavelet and block size dependent."},
    {"wname", 1, "bior4.4", "Wavelet family used for compression "
                            "Valid values are bior1.1, bior1.3, "
                            "bior1.5, bior2.2, bior2.4 ,bior2.6, bior2.8, bior3.1, bior3.3, "
                            "bior3.5, bior3.7, bior3.9, bior4.4"},
    {"nthreads", 1, "0", "Specify number of execution threads "
                         "0 => use number of cores"},
    {"vars", 1, "",
     "Colon delimited list of 3D variable names (compressed) "
     "to be included in "
     "the VDC"},
    {"force", 0, "", "Create a new VDC master file even if a VDC data "
                     "directory already exists. Results may be undefined if settings between "
                     "the new master file and old data directory do not match."},
    {"help", 0, "", "Print this message and exit"},
    {NULL}};

OptionParser::Option_T get_options[] = {
    {"bs", Wasp::CvtToSize_tVec, &opt.bs, sizeof(opt.bs)},
    {"cratios", Wasp::CvtToSize_tVec, &opt.cratios, sizeof(opt.cratios)},
    {"wname", Wasp::CvtToCPPStr, &opt.wname, sizeof(opt.wname)},
    {"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
    {"vars", Wasp::CvtToStrVec, &opt.vars, sizeof(opt.vars)},
    {"force", Wasp::CvtToBoolean, &opt.force, sizeof(opt.force)},
    {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
    {NULL}};

string ProgName;

void defineMapProjection(const DCWRF &dcwrf, VDCNetCDF &vdc) {

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

    if (argc < 3) {
        cerr << "Usage: " << ProgName << "wrffiles... master.nc" << endl;
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
    vector<string> wrffiles;
    for (int i = 0; i < argc - 1; i++)
        wrffiles.push_back(argv[i]);
    string master = argv[argc - 1];

    VDCNetCDF vdc(opt.nthreads);

    if (vdc.DataDirExists(master) && !opt.force) {
        MyBase::SetErrMsg(
            "Data directory exists and -force option not used. "
            "Remove directory %s or use -force",
            vdc.GetDataDir(master).c_str());
        exit(1);
    }

    size_t chunksize = 1024 * 1024 * 4;
    int rc = vdc.Initialize(master, vector<string>(), VDC::W, chunksize);
    if (rc < 0)
        exit(1);

    DCWRF dcwrf;
    rc = dcwrf.Initialize(wrffiles, vector<string>());
    if (rc < 0) {
        exit(1);
    }

    vector<string> dimnames = dcwrf.GetDimensionNames();
    for (int i = 0; i < dimnames.size(); i++) {
        DC::Dimension dim;
        dcwrf.GetDimension(dimnames[i], dim);
        rc = vdc.DefineDimension(dim.GetName(), dim.GetLength());
        if (rc < 0) {
            exit(1);
        }
    }

    // Make the default block dimension 64 for any missing dimensions
    //
    vector<size_t> bs = opt.bs;
    for (int i = bs.size(); i < 3; i++)
        bs.push_back(64);

    //
    // Define coordinate variables
    //
    for (int d = 0; d < 4; d++) {
        vector<string> coordnames = dcwrf.DC::GetCoordVarNames(d, true);

        //
        // Time coordinate and 1D coordinates are not blocked. Not sure
        // if this is really needed
        //
        vector<size_t> mybs;
        if (d < 2) {
            mybs.clear();
        } else {
            mybs = opt.bs;
        }

        vector<size_t> cratios(1, 1);

        rc = vdc.SetCompressionBlock(mybs, opt.wname, cratios);
        if (rc < 0)
            exit(1);

        for (int i = 0; i < coordnames.size(); i++) {
            DC::CoordVar cvar;
            dcwrf.GetCoordVarInfo(coordnames[i], cvar);

            vector<string> dimnames;
            bool ok = dcwrf.GetVarDimNames(coordnames[i], false, dimnames);
            assert(ok);

            string time_dim_name;
            if (dcwrf.IsTimeVarying(coordnames[i])) {
                time_dim_name = dimnames.back();
                dimnames.pop_back();
            }

            if (cvar.GetUniform()) {
                rc = vdc.DefineCoordVarUniform(
                    cvar.GetName(), dimnames, time_dim_name, cvar.GetUnits(),
                    cvar.GetAxis(), cvar.GetXType(), false);
            } else {
                rc = vdc.DefineCoordVar(
                    cvar.GetName(), dimnames, time_dim_name, cvar.GetUnits(),
                    cvar.GetAxis(), cvar.GetXType(), false);
            }

            if (rc < 0) {
                exit(1);
            }
        }
    }

    defineMapProjection(dcwrf, vdc);

    //
    // Define data variables
    //
    for (int d = 0; d < 4; d++) {
        vector<string> datanames = dcwrf.DC::GetDataVarNames(d, true);

        //
        // Time coordinate and 1D coordinates are not blocked
        //
        string mywname;
        vector<size_t> mybs;
        bool compress;
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
            size_t c = (size_t)pow(
                (double)cratios[i], (double)((float)d / 3.0));
            cratios[i] = c;
        }

        rc = vdc.SetCompressionBlock(mybs, mywname, cratios);
        if (rc < 0)
            exit(1);

        for (int i = 0; i < datanames.size(); i++) {
            DC::DataVar dvar;
            dcwrf.GetDataVarInfo(datanames[i], dvar);

            vector<string> dimnames;
            bool ok = dcwrf.GetVarDimNames(datanames[i], false, dimnames);
            assert(ok);

            vector<string> coordvars;
            ok = dcwrf.GetVarCoordVars(datanames[i], false, coordvars);
            assert(ok);

            rc = vdc.DefineDataVar(
                dvar.GetName(), dimnames, coordvars, dvar.GetUnits(),
                dvar.GetXType(), compress);

            if (rc < 0) {
                exit(1);
            }
        }
    }

    //
    // Copy attributes
    //
    {
        vector<string> varnames = dcwrf.GetDataVarNames();
        vector<string> coordnames = dcwrf.GetCoordVarNames();
        string globalvarname = "";
        vector<string> allnames;
        allnames.insert(allnames.end(), varnames.begin(), varnames.end());
        allnames.insert(allnames.end(), coordnames.begin(), coordnames.end());
        allnames.push_back(globalvarname);

        for (int i = 0; i < allnames.size(); i++) {
            vector<string> attnames = dcwrf.GetAttNames(allnames[i]);
            for (int j = 0; j < attnames.size(); j++) {
                vdc.CopyAtt(dcwrf, allnames[i], attnames[j]);
            }
        }
    }

    rc = vdc.EndDefine();
    if (rc < 0) {
        MyBase::SetErrMsg(
            "Failed to write VDC master file : %s", master.c_str());
        exit(1);
    }

    exit(0);
}
