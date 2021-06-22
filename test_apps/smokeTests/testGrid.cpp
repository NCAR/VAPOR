// This test exercises the RegularGrid, StretchedGrid, LayeredGrid,
// and CurvilinearGrid classes.  These grids are created according to
// a user-changable discretization, coordinate range, value range,
// and block size.
//
// The grids are then assigned values according to
// three data patterns: a constant field, a linear ramp, and a triangle
// signal.
//
// After values are assigned, values are gathered and compared
// from the functions Grid::GetValue( {i, j, k} ) and Grid::AccessIJK(i,j,k).
// RMS error is computed, along with counts for mismatches, and queries
// that yield missing values.
//
// This process is repeated once for linear interpolation, and once for
// nearest-neighbor interpolation.
//
// Functions under test:
//  LayeredGrid::LayeredGrid(
//    const std::vector <size_t> &dims,
//    const std::vector <size_t> &bs,
//    const std::vector <float *> &blks,
//    const std::vector <double> &xcoords,
//    const std::vector <double> &ycoords,
//    const RegularGrid &rg
//  )
//  RegularGrid::RegularGrid(
//    const std::vector <size_t> &dims,
//    const std::vector <size_t> &bs,
//    const std::vector <float *> &blks,
//    const std::vector <double> &minu,
//    const std::vector <double> &maxu
//  )
//  StretchedGrid::StretchedGrid(
//    const std::vector <size_t> &dims,
//    const std::vector <size_t> &bs,
//    const std::vector <float *> &blks,
//    const std::vector <double> &xcoords,
//    const std::vector <double> &ycoords,
//    const std::vector <double> &zcoords
//  )
//  CurvilinearGrid::CurvilinearGrid(
//    const std::vector <size_t> &dims,
//    const std::vector <size_t> &bs,
//    const std::vector <float *> &blks,
//    const RegularGrid &xrg,
//    const RegularGrid &yrg,
//    const RegularGrid &zrg,
//    std::shared_ptr <const QuadTreeRectangle<float, size_t> > qtr
//  )
//  Grid::GetDimensions()
//  Grid::SetValueIJK(size_t i, size_t j, size_t k, float v)
//  Grid::GetUserExtents(
//    std::vector <double> &minu, std::vector <double> &maxu
//  )
//  Grid::GetValueAtIndex(const size_t indices[3])
//  Grid::GetUserCoordinates(
//    const size_t indices[],
//    double coords[]
//  )
//  Grid::GetValue(const std::vector <double> &coords)
//  Grid::GetMissingValue()
//  Grid::SetInterpolationOrder(int order)

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cstdio>
#include <algorithm>
#include "vapor/VAssert.h"

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/FileUtils.h>
#include <vapor/OpenMPSupport.h>
#include "gridTools.h"

#include <vapor/ConstantGrid.h>

#include <float.h>
#include <cmath>

using namespace Wasp;
using namespace VAPoR;

namespace {
size_t X = 0;
size_t Y = 1;
size_t Z = 2;

std::vector<float *> Heap;
}    // namespace

struct opt_t {
    std::vector<std::string> grids;
    std::vector<std::string> arrangements;
    std::vector<size_t>      dims;
    std::vector<size_t>      bs;
    std::vector<float>       extents;
    double                   minValue;
    double                   maxValue;
    OptionParser::Boolean_T  help;
} opt;

OptionParser::OptDescRec_T set_opts[] = {{"grids", 1, "Regular:Stretched:Layered:Curvilinear:Unstructured2D", "Colon delimited list of grids to test"},
                                         {"arrangements", 1, "Constant:Ramp:RampOnAxis:Triangle",
                                          "Colon delimited list of "
                                          "data arrangements to test synthetic grids with"},
                                         {"dims", 1, "8:8:8",
                                          "Data volume dimensions expressed in "
                                          "grid points (NX:NY:NZ)"},
                                         {"bs", 1, "64:64:64", "Internal storage blocking factor expressed in grid points (NX:NY:NZ)"},
                                         {"extents", 1, "0:0:0:1:1:1",
                                          "Colon delimited 6-element vector "
                                          "specifying domain extents of synthetic grids in user coordinates "
                                          "(X0:Y0:Z0:X1:Y1:Z1)"},
                                         {"minValue", 1, "0", "The minimum data value to be assigned to cells in synthetic grids"},
                                         {"maxValue", 1, "1", "The maximum data value to be assigned to cells in synthetic grids"},
                                         {"help", 0, "", "Print this message and exit"},
                                         {nullptr}};

OptionParser::Option_T get_options[] = {{"grids", Wasp::CvtToStrVec, &opt.grids, sizeof(opt.grids)},
                                        {"arrangements", Wasp::CvtToStrVec, &opt.arrangements, sizeof(opt.arrangements)},
                                        {"dims", Wasp::CvtToSize_tVec, &opt.dims, sizeof(opt.dims)},
                                        {"bs", Wasp::CvtToSize_tVec, &opt.bs, sizeof(opt.bs)},
                                        {"extents", Wasp::CvtToFloatVec, &opt.extents, sizeof(opt.extents)},
                                        {"minValue", Wasp::CvtToDouble, &opt.minValue, sizeof(opt.minValue)},
                                        {"maxValue", Wasp::CvtToDouble, &opt.maxValue, sizeof(opt.maxValue)},
                                        {"help", Wasp::CvtToBoolean, &opt.help, sizeof(opt.help)},
                                        {nullptr}};

void InitializeOptions(int argc, char **argv, OptionParser &op)
{
    std::string ProgName = FileUtils::LegacyBasename(argv[0]);

    MyBase::SetErrMsgFilePtr(stderr);

    if (op.AppendOptions(set_opts) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(EXIT_FAILURE);
    }

    if (op.ParseOptions(&argc, argv, get_options) < 0) {
        cerr << ProgName << " : " << op.GetErrMsg();
        exit(EXIT_FAILURE);
    }

    if (opt.extents.size() != 6) {
        cerr << "The -extents flag must contain 6 elements if used" << endl;
        op.PrintOptionHelp(stderr, 80, false);
        exit(EXIT_FAILURE);
    }

    if (opt.dims.size() != 3) {
        cerr << "The -dims flag must contain 3 elements if used" << endl;
        op.PrintOptionHelp(stderr);
        exit(EXIT_FAILURE);
    }

    if (opt.help) {
        op.PrintOptionHelp(stderr);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    double t0 = Wasp::GetTime();

    OptionParser op;
    InitializeOptions(argc, argv, op);

    std::cout << std::fixed << std::showpoint;
    std::cout << std::setprecision(4);

    std::vector<double> minu = {opt.extents[X], opt.extents[Y], opt.extents[Z]};
    std::vector<double> maxu = {opt.extents[X + 3], opt.extents[Y + 3], opt.extents[Z + 3]};

    std::vector<size_t> bs2d = {opt.bs[X], opt.bs[Y]};
    std::vector<double> minu2d = {minu[X], minu[Y]};
    std::vector<double> maxu2d = {maxu[X], maxu[Y]};

    std::vector<size_t> dims2d = {opt.dims[X], opt.dims[Y]};

    int  rc = EXIT_SUCCESS;
    bool regularRC = true;
    bool stretchedRC = true;
    bool layeredRC = true;
    bool curvilinearRC = true;
    bool unstructured2DRC = true;

#pragma omp parallel
    {
        if (omp_get_thread_num() == 0) cout << "Num threads: " << omp_get_num_threads() << endl;
        ;
    }

    // Test Regular Grid
    if (std::find(opt.grids.begin(), opt.grids.end(), "Regular") != opt.grids.end()) {
        double               t0 = Wasp::GetTime();
        std::vector<float *> rgBlks = AllocateBlocks(opt.bs, opt.dims);
        RegularGrid *        regularGrid = new RegularGrid(opt.dims, opt.bs, rgBlks, minu, maxu);
        double               t1 = Wasp::GetTime() - t0;
        cout << "RegularGrid() time: " << t1 << endl;
        regularRC = RunTests(regularGrid, opt.arrangements, opt.minValue, opt.maxValue);
        delete regularGrid;
    }

    // Test Stretched Grid
    if (std::find(opt.grids.begin(), opt.grids.end(), "Stretched") != opt.grids.end()) {
        double         t0 = Wasp::GetTime();
        StretchedGrid *stretchedGrid = MakeStretchedGrid(opt.dims, opt.bs, minu, maxu);
        double         t1 = Wasp::GetTime() - t0;
        cout << "MakeStretchedGrid() time: " << t1 << endl;
        stretchedRC = RunTests(stretchedGrid, opt.arrangements, opt.minValue, opt.maxValue);
        delete stretchedGrid;
    }

    // Test Layered Grid
    if (std::find(opt.grids.begin(), opt.grids.end(), "Layered") != opt.grids.end()) {
        double       t0 = Wasp::GetTime();
        LayeredGrid *layeredGrid = MakeLayeredGrid(opt.dims, opt.bs, minu, maxu);
        double       t1 = Wasp::GetTime() - t0;
        cout << "MakeLayeredGrid() time: " << t1 << endl;
        layeredRC = RunTests(layeredGrid, opt.arrangements, opt.minValue, opt.maxValue);
        delete layeredGrid;
    }

    // Test Curvilinear Grid
    if (std::find(opt.grids.begin(), opt.grids.end(), "Curvilinear") != opt.grids.end()) {
        double           t0 = Wasp::GetTime();
        CurvilinearGrid *curvilinearGrid;
        curvilinearGrid = MakeCurvilinearTerrainGrid(opt.bs, minu, maxu, opt.dims);
        double t1 = Wasp::GetTime() - t0;
        cout << "MakeCurvilinearTerrainGrid() time: " << t1 << endl;
        curvilinearRC = RunTests(curvilinearGrid, opt.arrangements, opt.minValue, opt.maxValue);
        delete curvilinearGrid;
    }

    // Test Unstructured Grid 2D
    if (std::find(opt.grids.begin(), opt.grids.end(), "Unstructured2D") != opt.grids.end() && opt.dims[0] > 1 && opt.dims[1] > 1) {
        double              t0 = Wasp::GetTime();
        UnstructuredGrid2D *g = MakeUnstructuredGrid2D(opt.dims, opt.bs, minu, maxu);
        double              t1 = Wasp::GetTime() - t0;
        cout << "MakeUnstructuredGrid2D() time: " << t1 << endl;
        unstructured2DRC = RunTests(g, opt.arrangements, opt.minValue, opt.maxValue);
        delete g;
    }

    if (regularRC == false) {
        cerr << "Errors occurred while testing Grid::RegularGrid." << endl;
        rc = EXIT_FAILURE;
    }
    if (stretchedRC == false) {
        cerr << "Errors occurred while testing Grid::StretchedGrid." << endl;
        rc = EXIT_FAILURE;
    }
    if (layeredRC == false) {
        cerr << "Errors occurred while testing Grid::LayeredGrid." << endl;
        rc = EXIT_FAILURE;
    }
    if (curvilinearRC == false) {
        cerr << "Errors occurred while testing Grid::CurvilinearGrid." << endl;
        rc = EXIT_FAILURE;
    }
    if (unstructured2DRC == false) {
        cerr << "Errors occurred while testing Grid::UnstructuredGrid2D." << endl;
        rc = EXIT_FAILURE;
    }

    double t1 = Wasp::GetTime();
    cout << "Elapsed time: " << t1 - t0 << endl;

    DeleteHeap();

    return rc;
}
