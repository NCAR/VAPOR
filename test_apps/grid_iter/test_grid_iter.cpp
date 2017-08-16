#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <cassert>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/RegularGrid.h>
#include <vapor/LayeredGrid.h>
#include <vapor/CurvilinearGrid.h>
#include <vapor/KDTreeRG.h>

using namespace Wasp;
using namespace VAPoR;

struct {
    std::vector<size_t> bs;
    std::vector<double> minu;
    std::vector<double> maxu;
    std::vector<double> roimin;
    std::vector<double> roimax;
    std::vector<size_t> dims;
    std::vector<size_t> periodic;
    string type;
    OptionParser::Boolean_T debug;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {
    {"bs", 1, "64:64:64", "Colon delimited 3-element vector "
                          "specifying block size"},
    {"minu", 1, "0:0:0", "Colon delimited 3-element vector "
                         "specifying minimum user coordinates"},
    {"maxu", 1, "1.0:1.0:1.0", "Colon delimited 3-element vector "
                               "specifying maximum user coordinates"},
    {"roimin", 1, "0.0:0.0:0.0", "Colon delimited 3-element vector "
                                 "specifying min bbox coordinates for a region of interest"},
    {"roimax", 1, "1.0:1.0:1.0", "Colon delimited 3-element vector "
                                 "specifying max bbox coordinates for a region of interest"},
    {"dims", 1, "512:512:128", "Colon delimited 3-element vector "
                               "specifying grid dimensions"},
    {"periodic", 1, "0:0:0", "Colon delimited 3-element vector "
                             "of booleans specifying boundary periodicity"},
    {"type", 1, "regular", "Grid type. One of (regular, "
                           "layered, curvlinear"},
    {"debug", 0, "", "Print diagnostics"},
    {"help", 0, "", "Print this message and exit"},
    {NULL}};

OptionParser::Option_T get_options[] = {
    {"bs", Wasp::CvtToSize_tVec, &opt.bs, sizeof(opt.bs)},
    {"minu", Wasp::CvtToDoubleVec, &opt.minu, sizeof(opt.minu)},
    {"maxu", Wasp::CvtToDoubleVec, &opt.maxu, sizeof(opt.maxu)},
    {"roimin", Wasp::CvtToDoubleVec, &opt.roimin, sizeof(opt.roimin)},
    {"roimax", Wasp::CvtToDoubleVec, &opt.roimax, sizeof(opt.roimax)},
    {"dims", Wasp::CvtToSize_tVec, &opt.dims, sizeof(opt.dims)},
    {"periodic", Wasp::CvtToSize_tVec, &opt.periodic, sizeof(opt.periodic)},
    {"type", Wasp::CvtToCPPStr, &opt.type, sizeof(opt.type)},
    {"debug", Wasp::CvtToBoolean, &opt.debug, sizeof(opt.debug)},
    {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
    {NULL}};

const char *ProgName;

vector<float *> alloc_blocks(
    const vector<size_t> &bs,
    const vector<size_t> &dims) {

    size_t block_size = 1;
    size_t nblocks = 1;

    for (int i = 0; i < bs.size(); i++) {

        block_size *= bs[i];

        assert(dims[i] > 0);
        size_t nb = ((dims[i] - 1) / bs[i]) + 1;

        nblocks *= nb;
    }

    float *buf = new float[nblocks * block_size];

    vector<float *> blks;
    for (int i = 0; i < nblocks; i++) {
        blks.push_back(buf + i * block_size);
    }

    return (blks);
}

VAPoR::RegularGrid *make_regular_grid() {
    assert(opt.bs.size() == opt.minu.size());
    assert(opt.bs.size() == opt.maxu.size());
    assert(opt.bs.size() == opt.dims.size());
    assert(opt.bs.size() == opt.periodic.size());

    vector<float *> blks = alloc_blocks(opt.bs, opt.dims);

    RegularGrid *rg = new RegularGrid(
        opt.dims, opt.bs, blks, opt.minu, opt.maxu);

    return (rg);
}

VAPoR::LayeredGrid *make_layered_grid() {
    assert(opt.bs.size() == 3);
    assert(opt.bs.size() == opt.minu.size());
    assert(opt.bs.size() == opt.maxu.size());
    assert(opt.bs.size() == opt.dims.size());
    assert(opt.bs.size() == opt.periodic.size());

    vector<float *> zblks = alloc_blocks(opt.bs, opt.dims);

    RegularGrid *rg = new RegularGrid(
        opt.dims, opt.bs, zblks, vector<double>(3, 0.0), vector<double>(3, 1.0));

    for (size_t k = 0; k < rg->GetDimensions()[2]; k++) {
        for (size_t j = 0; j < rg->GetDimensions()[1]; j++) {
            for (size_t i = 0; i < rg->GetDimensions()[0]; i++) {

                double z = (opt.maxu[2] - opt.minu[2]) / (opt.dims[2] - 1) * k + opt.minu[2];
                rg->SetValueIJK(i, j, k, (float)z);
            }
        }
    }

    vector<float *> blks = alloc_blocks(opt.bs, opt.dims);

    vector<double> minu2d = {opt.minu[0], opt.minu[1]};
    vector<double> maxu2d = {opt.maxu[0], opt.maxu[1]};
    LayeredGrid *lg = new LayeredGrid(
        opt.dims, opt.bs, blks, minu2d, maxu2d, *rg);

    return (lg);
}

VAPoR::CurvilinearGrid *make_curvilinear_grid() {
    assert(opt.bs.size() == 3);
    assert(opt.bs.size() == opt.minu.size());
    assert(opt.bs.size() == opt.maxu.size());
    assert(opt.bs.size() == opt.dims.size());
    assert(opt.bs.size() == opt.periodic.size());

    vector<size_t> bs2d = {opt.bs[0], opt.bs[1]};
    vector<size_t> dims2d = {opt.dims[0], opt.dims[1]};

    vector<float *> xblks = alloc_blocks(bs2d, dims2d);

    RegularGrid *xrg = new RegularGrid(
        dims2d, bs2d, xblks, vector<double>(3, 0.0), vector<double>(3, 1.0));

    RegularGrid *yrg = new RegularGrid(
        dims2d, bs2d, xblks, vector<double>(3, 0.0), vector<double>(3, 1.0));

    for (size_t j = 0; j < dims2d[1]; j++) {
        for (size_t i = 0; i < dims2d[0]; i++) {

            double x = (opt.maxu[0] - opt.minu[0]) / (dims2d[0] - 1) * i + opt.minu[0];
            double y = (opt.maxu[1] - opt.minu[1]) / (dims2d[1] - 1) * j + opt.minu[1];
            xrg->SetValueIJK(i, j, 0, x);
            yrg->SetValueIJK(i, j, 0, y);
        }
    }

    KDTreeRG *kdtree = new KDTreeRG(*xrg, *yrg);

    vector<double> zcoords;
    for (int k = 0; k < opt.dims[2]; k++) {
        double z = (opt.maxu[2] - opt.minu[2]) / (opt.dims[2] - 1) * k + opt.minu[2];
        zcoords.push_back(z);
    }

    vector<float *> blks = alloc_blocks(opt.bs, opt.dims);

    vector<double> minu2d = {opt.minu[0], opt.minu[1]};
    vector<double> maxu2d = {opt.maxu[0], opt.maxu[1]};
    CurvilinearGrid *cg = new CurvilinearGrid(
        opt.dims, opt.bs, blks, *xrg, *yrg, zcoords, kdtree);

    return (cg);
}

void init_grid(StructuredGrid *sg) {

    // Initialize data to linear ramp
    //
    size_t kmax = opt.dims.size() >= 1 ? opt.dims[2] : 1;
    size_t jmax = opt.dims.size() >= 1 ? opt.dims[1] : 1;
    size_t imax = opt.dims.size() >= 1 ? opt.dims[0] : 1;

    for (size_t k = 0; k < kmax; k++) {
        for (size_t j = 0; j < jmax; j++) {
            for (size_t i = 0; i < imax; i++) {

                sg->SetValueIJK(i, j, k, (float)k + 1);
            }
        }
    }
}

int main(int argc, char **argv) {

    OptionParser op;
    string s;

    ProgName = Basename(argv[0]);

    MyBase::SetErrMsgFilePtr(stderr);

    if (op.AppendOptions(set_opts) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(1);
    }

    if (opt.help) {
        cerr << "Usage: " << ProgName << " [options] metafiles " << endl;
        op.PrintOptionHelp(stderr);
        exit(0);
    }

    if (opt.debug) {
        MyBase::SetDiagMsgFilePtr(stderr);
    }

    double t0 = Wasp::GetTime();

    StructuredGrid *sg = NULL;
    if (opt.type == "layered") {
        cout << "Layered grid" << endl;
        sg = make_layered_grid();
    } else if (opt.type == "curvilinear") {
        cout << "Curvilinear grid" << endl;
        sg = make_curvilinear_grid();
    } else {
        cout << "Regular grid" << endl;
        sg = make_regular_grid();
    }

    if (!sg)
        return (1);

    init_grid(sg);

    cout << "Creation time : " << Wasp::GetTime() - t0 << endl;
    cout << *sg;
    cout << endl;

    t0 = Wasp::GetTime();

    RegularGrid::Iterator itr;
    double accum = 0.0;
    size_t count = 0;
    for (itr = sg->begin(opt.roimin, opt.roimax); itr != sg->end(); ++itr) {
        accum += *itr;
        count++;
    }
    cout << "Iteration time : " << Wasp::GetTime() - t0 << endl;
    cout << "Sum and count: " << accum << " " << count << endl;

    delete sg;

    return (0);
}
