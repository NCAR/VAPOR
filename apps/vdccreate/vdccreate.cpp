#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <sstream>

#include <vapor/OptionParser.h>
#include <vapor/CFuncs.h>
#include <vapor/VDCNetCDF.h>

using namespace Wasp;
using namespace VAPoR;

struct opt_t {
    OptionParser::Dimension3D_T dim;
    std::vector<size_t> bs;
    std::vector<size_t> cratios;
    string wname;
    int numts;
    int nthreads;
    std::vector<string> vars3d;
    std::vector<string> vars2dxy;
    std::vector<string> vars2dxz;
    std::vector<string> vars2dyz;
    std::vector<string> ncvars3d;
    std::vector<string> ncvars2dxy;
    std::vector<string> ncvars2dxz;
    std::vector<string> ncvars2dyz;
    std::vector<float> extents;
    OptionParser::Boolean_T force;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {
    {"dimension", 1, "512x512x512", "Data volume dimensions expressed in "
                                    "grid points (NXxNYxNZ)"},
    {"bs", 1, "64:64:64",
     "Internal storage blocking factor expressed in grid points (NX:NY:NZ)"},
    {"cratios", 1, "500:100:10:1", "Colon delimited list compression ratios. "
                                   "The default is 500:100:10:1. The maximum compression ratio "
                                   "is wavelet and block size dependent."},
    {"wname", 1, "bior4.4", "Wavelet family used for compression "
                            "Valid values are bior1.1, bior1.3, "
                            "bior1.5, bior2.2, bior2.4 ,bior2.6, bior2.8, bior3.1, bior3.3, "
                            "bior3.5, bior3.7, bior3.9, bior4.4"},
    {"numts", 1, "1", "Number of timesteps in the data set"},
    {"nthreads", 1, "0", "Specify number of execution threads "
                         "0 => use number of cores"},
    {"vars3d", 1, "",
     "Colon delimited list of 3D variable names (compressed) "
     "to be included in "
     "the VDC"},
    {"vars2dxy", 1, "", "Colon delimited list of 2D XY-plane variable "
                        "names (compressed) "
                        "to be included in the VDC"},
    {"vars2dxz", 1, "", "Colon delimited list of 3D XZ-plane variable "
                        "names (compressed) "
                        "to be included in the VDC"},
    {"vars2dyz", 1, "", "Colon delimited list of 3D YZ-plane variable "
                        "names (compressed) "
                        "to be included in the VDC"},
    {"ncvars3d", 1, "",
     "Colon delimited list of 3D variable names (not compressed) "
     "to be included in "
     "the VDC"},
    {"ncvars2dxy", 1, "", "Colon delimited list of 2D XY-plane variable "
                          "names (not compressed) "
                          "to be included in the VDC"},
    {"ncvars2dxz", 1, "", "Colon delimited list of 3D XZ-plane variable "
                          "names (not compressed) "
                          "to be included in the VDC"},
    {"ncvars2dyz", 1, "", "Colon delimited list of 3D YZ-plane variable "
                          "names (not compressed) "
                          "to be included in the VDC"},
    {"extents", 1, "", "Colon delimited 6-element vector "
                       "specifying domain extents in user coordinates (X0:Y0:Z0:X1:Y1:Z1)"},
    {"force", 0, "", "Create a new VDC master file even if a VDC data "
                     "directory already exists. Results may be undefined if settings between "
                     "the new master file and old data directory do not match."},
    {"help", 0, "", "Print this message and exit"},
    {NULL}};

OptionParser::Option_T get_options[] = {
    {"dimension", Wasp::CvtToDimension3D, &opt.dim, sizeof(opt.dim)},
    {"bs", Wasp::CvtToSize_tVec, &opt.bs, sizeof(opt.bs)},
    {"cratios", Wasp::CvtToSize_tVec, &opt.cratios, sizeof(opt.cratios)},
    {"wname", Wasp::CvtToCPPStr, &opt.wname, sizeof(opt.wname)},
    {"numts", Wasp::CvtToInt, &opt.numts, sizeof(opt.numts)},
    {"nthreads", Wasp::CvtToInt, &opt.nthreads, sizeof(opt.nthreads)},
    {"vars3d", Wasp::CvtToStrVec, &opt.vars3d, sizeof(opt.vars3d)},
    {"vars2dxy", Wasp::CvtToStrVec, &opt.vars2dxy, sizeof(opt.vars2dxy)},
    {"vars2dxz", Wasp::CvtToStrVec, &opt.vars2dxz, sizeof(opt.vars2dxz)},
    {"vars2dyz", Wasp::CvtToStrVec, &opt.vars2dyz, sizeof(opt.vars2dyz)},
    {"ncvars3d", Wasp::CvtToStrVec, &opt.ncvars3d, sizeof(opt.ncvars3d)},
    {"ncvars2dxy", Wasp::CvtToStrVec, &opt.ncvars2dxy, sizeof(opt.ncvars2dxy)},
    {"ncvars2dxz", Wasp::CvtToStrVec, &opt.ncvars2dxz, sizeof(opt.ncvars2dxz)},
    {"ncvars2dyz", Wasp::CvtToStrVec, &opt.ncvars2dyz, sizeof(opt.ncvars2dyz)},
    {"extents", Wasp::CvtToFloatVec, &opt.extents, sizeof(opt.extents)},

    {"force", Wasp::CvtToBoolean, &opt.force, sizeof(opt.force)},
    {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
    {NULL}};

string ProgName;

void set_coord(
    VDCNetCDF &vdc,
    string dimname, size_t dimlen, float min, float max) {
    assert(dimlen >= 1);

    float *buf = new float[dimlen];

    float delta = dimlen > 1 ? (max - min) / (float)(dimlen - 1) : 0.0;

    for (int i = 0; i < dimlen; i++) {
        buf[i] = min + i * delta;
    }

    int rc = vdc.PutVar(0, dimname, -1, buf);
    if (rc < 0)
        exit(1);
}

void set_coords(
    VDCNetCDF &vdc,
    const vector<float> &extents,
    const vector<string> &dimnames,
    const vector<size_t> &dimlens) {
    if (!extents.size())
        return;

    assert(extents.size() == 6);
    assert(dimnames.size() == 4);
    assert(dimlens.size() == 4);

    set_coord(vdc, dimnames[0], dimlens[0], extents[0], extents[3]);
    set_coord(vdc, dimnames[1], dimlens[1], extents[1], extents[4]);
    set_coord(vdc, dimnames[2], dimlens[2], extents[2], extents[5]);
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

    if (argc != 2) {
        cerr << "Usage: " << ProgName << " [options] master.nc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        exit(1);
    }
    if (opt.extents.size() && opt.extents.size() != 6) {
        cerr << "Usage: " << ProgName << " master.nc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        exit(1);
    }
    string master = argv[1];

    if (opt.help) {
        cerr << "Usage: " << ProgName << " master.nc" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        exit(0);
    }

    VDCNetCDF vdc(opt.nthreads);

    if (vdc.DataDirExists(master) && !opt.force) {
        MyBase::SetErrMsg(
            "Data directory exists and -force option not used. "
            "Remove directory %s or use -force",
            vdc.GetDataDir(master).c_str());
        exit(1);
    }

    size_t chunksize = 1024 * 1024 * 4;
    int rc = vdc.Initialize(master, VDC::W, chunksize);
    if (rc < 0)
        exit(1);

    vector<string> dimnames;
    dimnames.push_back("Nx");
    dimnames.push_back("Ny");
    dimnames.push_back("Nz");
    dimnames.push_back("Nt");

    vector<size_t> dimlens;
    dimlens.push_back(opt.dim.nx);
    dimlens.push_back(opt.dim.ny);
    dimlens.push_back(opt.dim.nz);
    dimlens.push_back(opt.numts);

    vector<size_t> bs(1, 1);
    vector<size_t> cratios(1, 1);
    //	bs.push_back(opt.bs[0]);
    rc = vdc.SetCompressionBlock(bs, "", cratios);
    rc = vdc.DefineDimension(dimnames[0], dimlens[0], 0);

    //	bs.clear();
    //	bs.push_back(opt.bs[1]);
    rc = vdc.SetCompressionBlock(bs, "", cratios);
    rc = vdc.DefineDimension(dimnames[1], dimlens[1], 1);

    //	bs.clear();
    //	bs.push_back(opt.bs[2]);
    rc = vdc.SetCompressionBlock(bs, "", cratios);
    rc = vdc.DefineDimension(dimnames[2], dimlens[2], 2);

    //	bs.clear();
    rc = vdc.SetCompressionBlock(bs, "", cratios);
    rc = vdc.DefineDimension(dimnames[3], dimlens[3], 3);

    bs = opt.bs;
    rc = vdc.SetCompressionBlock(opt.bs, opt.wname, opt.cratios);
    if (rc < 0)
        exit(1);

    for (int i = 0; i < opt.vars3d.size(); i++) {
        rc = vdc.DefineDataVar(
            opt.vars3d[i], dimnames, dimnames, "", VDC::FLOAT, true);
    }
    for (int i = 0; i < opt.ncvars3d.size(); i++) {
        rc = vdc.DefineDataVar(
            opt.ncvars3d[i], dimnames, dimnames, "", VDC::FLOAT, false);
    }

    vector<string> dimnames2dxy;
    dimnames2dxy.push_back(dimnames[0]);
    dimnames2dxy.push_back(dimnames[1]);
    dimnames2dxy.push_back(dimnames[3]);

    //
    // Try to compute "reasonable" 2D compression ratios from 3D
    // compression ratios
    //
    vector<size_t> cratios2D = opt.cratios;
    for (int i = 0; i < cratios2D.size(); i++) {
        size_t c = (size_t)pow((double)cratios2D[i], (double)(2.0 / 3.0));
        cratios2D[i] = c;
    }

    rc = vdc.SetCompressionBlock(opt.bs, opt.wname, cratios2D);
    if (rc < 0)
        exit(1);

    for (int i = 0; i < opt.vars2dxy.size(); i++) {
        rc = vdc.DefineDataVar(
            opt.vars2dxy[i], dimnames2dxy, dimnames2dxy, "", VDC::FLOAT, true);
    }
    for (int i = 0; i < opt.ncvars2dxy.size(); i++) {
        rc = vdc.DefineDataVar(
            opt.ncvars2dxy[i], dimnames2dxy, dimnames2dxy, "", VDC::FLOAT, false);
    }

    vector<string> dimnames2dxz;
    dimnames2dxz.push_back(dimnames[0]);
    dimnames2dxz.push_back(dimnames[2]);
    dimnames2dxz.push_back(dimnames[3]);
    for (int i = 0; i < opt.vars2dxz.size(); i++) {
        rc = vdc.DefineDataVar(
            opt.vars2dxz[i], dimnames2dxz, dimnames2dxz, "", VDC::FLOAT, true);
    }
    for (int i = 0; i < opt.ncvars2dxz.size(); i++) {
        rc = vdc.DefineDataVar(
            opt.ncvars2dxz[i], dimnames2dxz, dimnames2dxz, "", VDC::FLOAT, false);
    }

    vector<string> dimnames2dyz;
    dimnames2dyz.push_back(dimnames[1]);
    dimnames2dyz.push_back(dimnames[2]);
    dimnames2dyz.push_back(dimnames[3]);
    for (int i = 0; i < opt.vars2dyz.size(); i++) {
        rc = vdc.DefineDataVar(
            opt.vars2dyz[i], dimnames2dyz, dimnames2dyz, "", VDC::FLOAT, true);
    }
    for (int i = 0; i < opt.ncvars2dyz.size(); i++) {
        rc = vdc.DefineDataVar(
            opt.ncvars2dyz[i], dimnames2dyz, dimnames2dyz, "", VDC::FLOAT, false);
    }

    vdc.EndDefine();

    set_coords(vdc, opt.extents, dimnames, dimlens);
}
