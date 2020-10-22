#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include "vapor/VAssert.h"

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/RegularGrid.h>
#include <vapor/LayeredGrid.h>
#include <vapor/CurvilinearGrid.h>
#include <vapor/StretchedGrid.h>
#include <vapor/FileUtils.h>

using namespace Wasp;
using namespace VAPoR;

struct {
    std::vector<size_t>     bs;
    std::vector<double>     minu;
    std::vector<double>     maxu;
    std::vector<double>     roimin;
    std::vector<double>     roimax;
    std::vector<size_t>     dims;
    std::vector<size_t>     periodic;
    string                  type;
    OptionParser::Boolean_T debug;
    OptionParser::Boolean_T help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"bs", 1, "64:64:64",
                                          "Colon delimited 3-element vector "
                                          "specifying block size"},
                                         {"minu", 1, "0:0:0",
                                          "Colon delimited 3-element vector "
                                          "specifying minimum user coordinates"},
                                         {"maxu", 1, "1.0:1.0:1.0",
                                          "Colon delimited 3-element vector "
                                          "specifying maximum user coordinates"},
                                         {"roimin", 1, "0.0:0.0:0.0",
                                          "Colon delimited 3-element vector "
                                          "specifying min bbox coordinates for a region of interest"},
                                         {"roimax", 1, "1.0:1.0:1.0",
                                          "Colon delimited 3-element vector "
                                          "specifying max bbox coordinates for a region of interest"},
                                         {"dims", 1, "512:512:128",
                                          "Colon delimited 3-element vector "
                                          "specifying grid dimensions"},
                                         {"periodic", 1, "0:0:0",
                                          "Colon delimited 3-element vector "
                                          "of booleans specifying boundary periodicity"},
                                         {"type", 1, "regular",
                                          "Grid type. One of (regular, "
                                          "layered, curvlinear, stretched"},
                                         {"debug", 0, "", "Print diagnostics"},
                                         {"help", 0, "", "Print this message and exit"},
                                         {NULL}};

OptionParser::Option_T get_options[] = {{"bs", Wasp::CvtToSize_tVec, &opt.bs, sizeof(opt.bs)},
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

namespace {
vector<float *> Heap;
};

template<class T> void out_container(T first, T last)
{
    for (T itr = first; itr != last; ++itr) { cout << *itr << " "; }
    cout << endl;
}

const char *ProgName;

vector<float *> alloc_blocks(const vector<size_t> &bs, const vector<size_t> &dims)
{
    size_t block_size = 1;
    size_t nblocks = 1;

    for (int i = 0; i < bs.size(); i++) {
        block_size *= bs[i];

        VAssert(dims[i] > 0);
        size_t nb = ((dims[i] - 1) / bs[i]) + 1;

        nblocks *= nb;
    }

    float *buf = new float[nblocks * block_size];
    Heap.push_back(buf);

    vector<float *> blks;
    for (int i = 0; i < nblocks; i++) { blks.push_back(buf + i * block_size); }

    return (blks);
}

VAPoR::RegularGrid *make_regular_grid()
{
    VAssert(opt.bs.size() == opt.minu.size());
    VAssert(opt.bs.size() == opt.maxu.size());
    VAssert(opt.bs.size() == opt.dims.size());
    VAssert(opt.bs.size() == opt.periodic.size());

    vector<float *> blks = alloc_blocks(opt.bs, opt.dims);

    RegularGrid *rg = new RegularGrid(opt.dims, opt.bs, blks, opt.minu, opt.maxu);

    return (rg);
}

VAPoR::StretchedGrid *make_stretched_grid()
{
    VAssert(opt.bs.size() == opt.minu.size());
    VAssert(opt.bs.size() == opt.maxu.size());
    VAssert(opt.bs.size() == opt.dims.size());
    VAssert(opt.bs.size() == opt.periodic.size());

    vector<float *> blks = alloc_blocks(opt.bs, opt.dims);

    vector<double>           xcoords, ycoords, zcoords;
    vector<vector<double> *> coords = {&xcoords, &ycoords, &zcoords};
    double                   delta;

    for (int i = 0; i < opt.minu.size(); i++) {
        delta = opt.dims[i] == 1 ? 0.0 : (opt.maxu[i] - opt.minu[i]) / (opt.dims[i] - 1);
        for (int j = 0; i < opt.dims[j]; j++) { coords[i]->push_back(opt.minu[j] + j * delta); }
    }

    StretchedGrid *rg = new StretchedGrid(opt.dims, opt.bs, blks, xcoords, ycoords, zcoords);

    return (rg);
}

VAPoR::LayeredGrid *make_layered_grid()
{
    VAssert(opt.bs.size() == 3);
    VAssert(opt.bs.size() == opt.minu.size());
    VAssert(opt.bs.size() == opt.maxu.size());
    VAssert(opt.bs.size() == opt.dims.size());
    VAssert(opt.bs.size() == opt.periodic.size());

    vector<float *> zblks = alloc_blocks(opt.bs, opt.dims);

    RegularGrid *rg = new RegularGrid(opt.dims, opt.bs, zblks, vector<double>(3, 0.0), vector<double>(3, 1.0));

    for (size_t k = 0; k < rg->GetDimensions()[2]; k++) {
        for (size_t j = 0; j < rg->GetDimensions()[1]; j++) {
            for (size_t i = 0; i < rg->GetDimensions()[0]; i++) {
                double z = (opt.maxu[2] - opt.minu[2]) / (opt.dims[2] - 1) * k + opt.minu[2];
                rg->SetValueIJK(i, j, k, (float)z);
            }
        }
    }

    vector<float *> blks = alloc_blocks(opt.bs, opt.dims);

    double         deltax = opt.maxu[0] - opt.minu[0] / (opt.dims[0] - 1);
    vector<double> xcoords;
    for (int i = 0; i < opt.dims[0]; i++) { xcoords.push_back(opt.minu[0] + (i * deltax)); }

    double         deltay = opt.maxu[1] - opt.minu[1] / (opt.dims[1] - 1);
    vector<double> ycoords;
    for (int i = 0; i < opt.dims[1]; i++) { ycoords.push_back(opt.minu[1] + (i * deltay)); }

    LayeredGrid *lg = new LayeredGrid(opt.dims, opt.bs, blks, xcoords, ycoords, *rg);

    return (lg);
}

VAPoR::CurvilinearGrid *make_curvilinear_grid()
{
    VAssert(opt.bs.size() == 3);
    VAssert(opt.bs.size() == opt.minu.size());
    VAssert(opt.bs.size() == opt.maxu.size());
    VAssert(opt.bs.size() == opt.dims.size());
    VAssert(opt.bs.size() == opt.periodic.size());

    vector<size_t> bs2d = {opt.bs[0], opt.bs[1]};
    vector<size_t> dims2d = {opt.dims[0], opt.dims[1]};

    vector<float *> xblks = alloc_blocks(bs2d, dims2d);

    RegularGrid *xrg = new RegularGrid(dims2d, bs2d, xblks, vector<double>(2, 0.0), vector<double>(2, 1.0));

    vector<float *> yblks = alloc_blocks(bs2d, dims2d);

    RegularGrid *yrg = new RegularGrid(dims2d, bs2d, yblks, vector<double>(2, 0.0), vector<double>(2, 1.0));

    for (size_t j = 0; j < dims2d[1]; j++) {
        for (size_t i = 0; i < dims2d[0]; i++) {
            double x = (opt.maxu[0] - opt.minu[0]) / (dims2d[0] - 1) * i + opt.minu[0];
            double y = (opt.maxu[1] - opt.minu[1]) / (dims2d[1] - 1) * j + opt.minu[1];
            xrg->SetValueIJK(i, j, 0, x);
            yrg->SetValueIJK(i, j, 0, y);
        }
    }

    vector<double> zcoords;
    for (int k = 0; k < opt.dims[2]; k++) {
        double z = (opt.maxu[2] - opt.minu[2]) / (opt.dims[2] - 1) * k + opt.minu[2];
        zcoords.push_back(z);
    }

    vector<float *> blks = alloc_blocks(opt.bs, opt.dims);

    vector<double>   minu2d = {opt.minu[0], opt.minu[1]};
    vector<double>   maxu2d = {opt.maxu[0], opt.maxu[1]};
    CurvilinearGrid *cg = new CurvilinearGrid(opt.dims, opt.bs, blks, *xrg, *yrg, zcoords, NULL);

    return (cg);
}

VAPoR::CurvilinearGrid *make_curvilinear_terrain_grid()
{
    VAssert(opt.bs.size() == 3);
    VAssert(opt.bs.size() == opt.minu.size());
    VAssert(opt.bs.size() == opt.maxu.size());
    VAssert(opt.bs.size() == opt.dims.size());
    VAssert(opt.bs.size() == opt.periodic.size());

    vector<size_t> bs2d = {opt.bs[0], opt.bs[1]};
    vector<size_t> dims2d = {opt.dims[0], opt.dims[1]};
    vector<size_t> bs = {opt.bs[0], opt.bs[1], opt.bs[2]};
    vector<size_t> dims = {opt.dims[0], opt.dims[1], opt.dims[2]};

    vector<float *> xblks = alloc_blocks(bs2d, dims2d);

    RegularGrid *xrg = new RegularGrid(dims2d, bs2d, xblks, vector<double>(2, 0.0), vector<double>(2, 1.0));

    vector<float *> yblks = alloc_blocks(bs2d, dims2d);

    RegularGrid *yrg = new RegularGrid(dims2d, bs2d, yblks, vector<double>(2, 0.0), vector<double>(2, 1.0));

    for (size_t j = 0; j < dims2d[1]; j++) {
        for (size_t i = 0; i < dims2d[0]; i++) {
            double x = (opt.maxu[0] - opt.minu[0]) / (dims2d[0] - 1) * i + opt.minu[0];
            double y = (opt.maxu[1] - opt.minu[1]) / (dims2d[1] - 1) * j + opt.minu[1];
            xrg->SetValueIJK(i, j, 0, x);
            yrg->SetValueIJK(i, j, 0, y);
        }
    }

    vector<float *> zblks = alloc_blocks(bs, dims);

    RegularGrid *zrg = new RegularGrid(dims, bs, zblks, vector<double>(3, 0.0), vector<double>(3, 1.0));

    for (size_t k = 0; k < dims[2]; k++) {
        double z = (opt.maxu[2] - opt.minu[2]) / (dims[2] - 1) * k + opt.minu[2];

        for (size_t j = 0; j < dims[1]; j++) {
            for (size_t i = 0; i < dims[0]; i++) { zrg->SetValueIJK(i, j, k, z); }
        }
    }

    vector<float *> blks = alloc_blocks(opt.bs, opt.dims);

    vector<double>   minu2d = {opt.minu[0], opt.minu[1]};
    vector<double>   maxu2d = {opt.maxu[0], opt.maxu[1]};
    CurvilinearGrid *cg = new CurvilinearGrid(opt.dims, opt.bs, blks, *xrg, *yrg, *zrg, NULL);

    return (cg);
}

void init_grid(StructuredGrid *sg)
{
    // Initialize data to linear ramp
    //
    size_t kmax = opt.dims.size() >= 1 ? opt.dims[2] : 1;
    size_t jmax = opt.dims.size() >= 1 ? opt.dims[1] : 1;
    size_t imax = opt.dims.size() >= 1 ? opt.dims[0] : 1;

    size_t idx = 0;
    for (size_t k = 0; k < kmax; k++) {
        for (size_t j = 0; j < jmax; j++) {
            for (size_t i = 0; i < imax; i++) {
                sg->SetValueIJK(i, j, k, (float)idx);
                idx++;
            }
        }
    }
}

void test_iterator(const StructuredGrid *sg)
{
    cout << "Value Iterator Test ----->" << endl;

    double t0 = Wasp::GetTime();

    Grid::ConstIterator itr;
    Grid::ConstIterator enditr = sg->cend();
    double              accum = 0.0;
    size_t              count = 0;
    //    for (itr = sg->cbegin(opt.roimin, opt.roimax); itr!=enditr; ++itr) {
    for (itr = sg->cbegin(); itr != enditr; ++itr) {
        accum += *itr;
        count++;
    }
    cout << "Iteration time : " << Wasp::GetTime() - t0 << endl;
    cout << "Sum and count: " << accum << " " << count << endl;
    cout << endl;
}

void test_operator_pg_iterator(const StructuredGrid *sg)
{
    cout << "Operator += Test ----->" << endl;

    double t0 = Wasp::GetTime();

    const size_t stride = 8;

    Grid::ConstIterator itr1 = sg->cbegin();
    Grid::ConstIterator enditr1 = sg->cend();
    double              accum1 = 0.0;
    size_t              count1 = 0;
    size_t              index = 0;
    for (; itr1 != enditr1; ++itr1) {
        if ((index % stride) == 0) {
            accum1 += *itr1;
            count1++;
        }
        index++;
    }
    double t1 = Wasp::GetTime();

    cout << "Iteration time (inc by 1) : " << t1 - t0 << endl;
    cout << "Sum and count: " << accum1 << " " << count1 << endl;

    double              t2 = Wasp::GetTime();
    Grid::ConstIterator itr2 = sg->cbegin();
    Grid::ConstIterator enditr2 = sg->cend();
    double              accum2 = 0.0;
    size_t              count2 = 0;
    for (; itr2 != enditr2;) {
        accum2 += *itr2;
        count2++;
        itr2 += stride;
    }
    double t3 = Wasp::GetTime();

    cout << "Iteration time (+=) : " << t3 - t2 << endl;
    cout << "Sum and count: " << accum2 << " " << count2 << endl;

    itr1 = sg->cbegin();
    enditr1 = sg->cend();
    itr2 = sg->cbegin();
    enditr2 = sg->cend();
    index = 0;
    bool mismatch = false;
    for (; itr1 != enditr1 && itr2 != enditr2; ++itr1) {
        if ((index % stride) == 0) {
            if (*itr1 != *itr2) {
                mismatch = true;
                break;
            }
            itr2 += stride;
        }
        index++;
    }
    if (!mismatch) {
        cout << "operator+= operator++ match" << endl;
    } else {
        cout << "FAIL : operator+= operator++ mismatch" << endl;
    }
    cout << endl;
}

#ifdef VAPOR3_0_0_ALPHA
void test_cell_iterator(const StructuredGrid *sg)
{
    cout << "Cell Iterator Test ----->" << endl;
    double t0 = Wasp::GetTime();

    StructuredGrid::ConstCellIterator itr;
    size_t                            count = 0;
    for (itr = sg->ConstCellBegin(); itr != sg->ConstCellEnd(); ++itr) {
        count++;
        cout << "Cell : ";
        out_container((*itr).cbegin(), (*itr).cend());
        vector<vector<size_t>> nodes;
        sg->GetCellNodes(*itr, nodes);
        cout << "\tNodes : " << endl;
        for (int i = 0; i < nodes.size(); i++) {
            cout << "\t";
            out_container(nodes[i].cbegin(), nodes[i].cend());
        }
        cout << endl;

        //		const vector <double> &coord = *(itr.GetCoordItr());
        //		cout << coord[0] << " " << coord[1] << " " << coord[2] << endl;
    }
    cout << "Iteration time : " << Wasp::GetTime() - t0 << endl;
    cout << "Cell count : " << count << endl;
    cout << endl;
}
#endif

void test_node_iterator(const StructuredGrid *sg)
{
    cout << "Node Iterator Test ----->" << endl;

    double t0 = Wasp::GetTime();

    Grid::ConstNodeIterator itr;
    Grid::ConstNodeIterator enditr = sg->ConstNodeEnd();

    if (opt.roimin == opt.minu && opt.roimax == opt.maxu) {
        itr = sg->ConstNodeBegin();
    } else {
        itr = sg->ConstNodeBegin(opt.roimin, opt.roimax);
    }
    size_t count = 0;
    //    for ( ; itr!=sg->ConstNodeEnd(); ++itr)
    for (; itr != enditr; ++itr) {
        count++;
        //		out_container((*itr).cbegin(), (*itr).cend());
        //		cout << endl;
    }
    cout << "Iteration time : " << Wasp::GetTime() - t0 << endl;
    cout << "count: " << count << endl;
    cout << endl;
}

void test_cell_iterator(const StructuredGrid *sg)
{
    cout << "Cell Iterator Test ----->" << endl;

    double t0 = Wasp::GetTime();

    Grid::ConstCellIterator itr;
    Grid::ConstCellIterator enditr = sg->ConstCellEnd();

    if (opt.roimin == opt.minu && opt.roimax == opt.maxu) {
        itr = sg->ConstCellBegin();
    } else {
        itr = sg->ConstCellBegin(opt.roimin, opt.roimax);
    }

    size_t count = 0;
    //    for (itr = sg->ConstCellBegin(); itr!=sg->ConstCellEnd(); ++itr)
    for (; itr != enditr; ++itr) {
        count++;
        //		out_container((*itr).cbegin(), (*itr).cend());
        //		cout << endl;
    }
    cout << "Iteration time : " << Wasp::GetTime() - t0 << endl;
    cout << "count: " << count << endl;
    cout << endl;
}

void test_coord_iterator(const StructuredGrid *sg)
{
    cout << "Coord Iterator Test ----->" << endl;

    double t0 = Wasp::GetTime();

    Grid::ConstCoordItr itr;
    Grid::ConstCoordItr enditr = sg->ConstCoordEnd();
    size_t              count = 0;
    for (itr = sg->ConstCoordBegin(); itr != enditr; ++itr) { count++; }
    cout << "Iteration time : " << Wasp::GetTime() - t0 << endl;
    cout << "count: " << count << endl;
    cout << endl;
}

void test_coord_accessor(const StructuredGrid *sg)
{
    cout << "Coord Accessor Test ----->" << endl;

    double t0 = Wasp::GetTime();

    size_t count = 0;

    vector<size_t> index(3);
    vector<double> coord;
    for (size_t k = 0; k < sg->GetDimensions()[2]; k++) {
        for (size_t j = 0; j < sg->GetDimensions()[1]; j++) {
            for (size_t i = 0; i < sg->GetDimensions()[0]; i++) {
                index[0] = i;
                index[1] = j;
                index[2] = k;

                sg->GetUserCoordinates(index, coord);
                count++;
            }
        }
    }
    cout << "Iteration time : " << Wasp::GetTime() - t0 << endl;
    cout << "count: " << count << endl;
    cout << endl;
}

void test_getvalue(StructuredGrid *sg)
{
    cout << "GetValue Test ----->" << endl;

    float rangev[2];
    sg->GetRange(rangev);

    sg->SetInterpolationOrder(1);

    float range = rangev[1] - rangev[0];
    if (range == 0.0) return;

    double t0 = Wasp::GetTime();

    Grid::ConstIterator itr;
    Grid::ConstIterator enditr = sg->cend();
    Grid::ConstCoordItr coord_itr = sg->ConstCoordBegin();
    Grid::ConstCoordItr coord_enditr = sg->ConstCoordEnd();
    float               maxErr = 0.0;
    for (itr = sg->cbegin(); itr != enditr; ++itr, ++coord_itr) {
        float                 v1 = *itr;
        const vector<double> &coord = *coord_itr;

        float v2 = sg->GetValue(coord);

        float err = abs(v1 - v2) / range;
        if (err > maxErr) { maxErr = err; }
    }
    cout << "Iteration time : " << Wasp::GetTime() - t0 << endl;
    cout << "Lmax error : " << maxErr << endl;
    cout << endl;
}

void test_roi_iterator()
{
    cout << "ROI Test ----->" << endl;

    vector<size_t>  dims = {128, 128, 128};
    vector<size_t>  bs = {64, 64, 64};
    vector<float *> blks = alloc_blocks(bs, dims);

    vector<double> minu = {0.0, 0.0, 0.0};
    vector<double> maxu = {(double)dims[0] - 1, (double)dims[1] - 1, (double)dims[2] - 1};

    RegularGrid *rg = new RegularGrid(dims, bs, blks, minu, maxu);

    vector<double> delta;
    vector<double> roiminu, roimaxu;
    for (int i = 0; i < minu.size(); i++) {
        delta.push_back(maxu[i] - minu[i] / (dims[i] - 1));
        roiminu.push_back(minu[i] + (delta[i] / 0.5));
        roimaxu.push_back(maxu[i] - (delta[i] / 0.5));
    }

    size_t idx = 0;
    for (size_t k = 1; k < dims[2] - 1; k++) {
        for (size_t j = 1; j < dims[1] - 1; j++) {
            for (size_t i = 1; i < dims[0] - 1; i++) {
                rg->SetValueIJK(i, j, k, (float)idx);
                idx++;
            }
        }
    }

    Grid::ConstIterator itr = rg->cbegin(roiminu, roimaxu);
    Grid::ConstIterator enditr = rg->cend();
    idx = 0;
    size_t         v = 0;
    vector<size_t> index;
    bool           pass = true;
    for (; itr != enditr; ++itr) {
        v = (size_t)*itr;

        if (idx != v) {
            pass = false;
            cerr << "FAIL test_roi_iterator : " << v << "not equal " << idx << endl;
        }
    }

    if (pass) { cout << "ROI test passed" << endl; }
    cout << endl;
}

int main(int argc, char **argv)
{
    OptionParser op;
    string       s;

    ProgName = FileUtils::LegacyBasename(argv[0]);

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

    if (opt.debug) { MyBase::SetDiagMsgFilePtr(stderr); }

    double t0 = Wasp::GetTime();

    StructuredGrid *sg = NULL;
    if (opt.type == "layered") {
        cout << "Layered grid" << endl;
        sg = make_layered_grid();
    } else if (opt.type == "curvilinear") {
        cout << "Curvilinear grid" << endl;
        sg = make_curvilinear_grid();
    } else if (opt.type == "curvilinear_terrain") {
        cout << "Curvilinear terrain grid" << endl;
        sg = make_curvilinear_terrain_grid();
    } else if (opt.type == "stretched") {
        cout << "Stretched grid" << endl;
        sg = make_stretched_grid();
    } else {
        cout << "Regular grid" << endl;
        sg = make_regular_grid();
    }

    if (!sg) return (1);

    init_grid(sg);

    cout << "Creation time : " << Wasp::GetTime() - t0 << endl;
    cout << *sg;
    vector<double> minu, maxu;
    sg->GetUserExtents(minu, maxu);
    cout << "Grid Extents: " << endl;
    for (int i = 0; i < minu.size(); i++) { cout << "\t" << minu[i] << " " << maxu[i] << endl; }
    cout << endl;

    test_roi_iterator();

    test_iterator(sg);

    test_operator_pg_iterator(sg);

    test_node_iterator(sg);

    test_cell_iterator(sg);

    test_coord_iterator(sg);

    test_coord_accessor(sg);

    test_getvalue(sg);

    delete sg;

    for (int i = 0; i < Heap.size(); i++) { delete[] Heap[i]; }

    return (0);
}
