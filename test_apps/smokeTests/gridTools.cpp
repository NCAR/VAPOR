#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include <cstdio>
#include <string>
#include <algorithm>

#include <vapor/CFuncs.h>
#include <vapor/OptionParser.h>
#include <vapor/DataMgr.h>
#include <vapor/FileUtils.h>
#include <vapor/utils.h>

#include "vapor/VAssert.h"
#include "gridTools.h"

#include <vapor/ConstantGrid.h>

#include <float.h>
#include <cmath>

using namespace Wasp;
using namespace VAPoR;

enum interpOrder { nearestNeighbor = 0, linear = 1 };

namespace {
size_t X = 0;
size_t Y = 1;
size_t Z = 2;

std::vector<void *> Heap;
}    // namespace

void DeleteHeap()
{
    for (size_t i = 0; i < Heap.size(); i++) ::operator delete(Heap[i]);
}

template<typename T> vector<T *> AllocateBlocksType(const vector<size_t> &bs, const vector<size_t> &dims)
{
    size_t block_size = 1;
    size_t nblocks = 1;

    for (size_t i = 0; i < bs.size(); i++) {
        block_size *= bs[i];

        VAssert(dims[i] > 0);
        size_t nb = ((dims[i] - 1) / bs[i]) + 1;

        nblocks *= nb;
    }

    T *buf = new T[nblocks * block_size];

    Heap.push_back(buf);

    std::vector<T *> blks;
    for (size_t i = 0; i < nblocks; i++) { blks.push_back(buf + i * block_size); }
    return (blks);
}

vector<float *> AllocateBlocks(const vector<size_t> &bs, const vector<size_t> &dims) { return (AllocateBlocksType<float>(bs, dims)); }

void MakeTriangle(Grid *grid, float minVal, float maxVal)
{
    auto tmp = grid->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(grid->GetNumDimensions());
    size_t x = dims[X];
    size_t y = dims.size() > 1 ? dims[Y] : 1;
    size_t z = dims.size() > 2 ? dims[Z] : 1;

    float value = minVal;
    for (size_t k = 0; k < z; k++) {
        for (size_t j = 0; j < y; j++) {
            for (size_t i = 0; i < x; i++) {
                value = value == minVal ? maxVal : minVal;
                grid->SetValueIJK(i, j, k, value);
            }
        }
    }
}

void MakeConstantField(Grid *grid, float value)
{
    auto tmp = grid->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(grid->GetNumDimensions());
    size_t x = dims[X];
    size_t y = dims.size() > 1 ? dims[Y] : 1;
    size_t z = dims.size() > 2 ? dims[Z] : 1;

    for (size_t k = 0; k < z; k++) {
        for (size_t j = 0; j < y; j++) {
            for (size_t i = 0; i < x; i++) { grid->SetValueIJK(i, j, k, value); }
        }
    }
}

void MakeRamp(Grid *grid, float minVal, float maxVal)
{
    auto tmp = grid->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(grid->GetNumDimensions());
    size_t x = dims[X];
    size_t y = dims.size() > 1 ? dims[Y] : 1;
    size_t z = dims.size() > 2 ? dims[Z] : 1;

    float increment = (maxVal - minVal) / ((x * y * z - 1) == 0 ? 1 : (x * y * z - 1));

    float value = minVal;
    for (size_t k = 0; k < z; k++) {
        for (size_t j = 0; j < y; j++) {
            for (size_t i = 0; i < x; i++) {
                grid->SetValueIJK(i, j, k, value);
                value += increment;
            }
        }
    }
}

void MakeRampOnAxis(Grid *grid, float minVal, float maxVal, size_t axis = X)
{
    auto tmp = grid->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(grid->GetNumDimensions());
    size_t x = dims[X];
    size_t y = dims.size() > 1 ? dims[Y] : 1;
    size_t z = dims.size() > 2 ? dims[Z] : 1;

    float xIncrement = axis == X ? (maxVal - minVal) / (dims[X] - 1) : 0;
    float yIncrement = axis == Y ? (maxVal - minVal) / (dims[Y] - 1) : 0;
    float zIncrement = axis == Z ? (maxVal - minVal) / (dims[Z] - 1) : 0;

    float value = minVal;
    for (size_t k = 0; k < z; k++) {
        value = axis == Y ? minVal : value;    // reset value if we're ramping on Y
        for (size_t j = 0; j < y; j++) {
            value = axis == X ? minVal : value;    // reset value if we're ramping on X
            for (size_t i = 0; i < x; i++) {
                grid->SetValueIJK(i, j, k, value);
                value += xIncrement;
            }
            value += yIncrement;
        }
        value += zIncrement;
    }
}

// This function iterates across all nodes in a grid; making comparisons between the
// data values returned by the functions GetValueAtIndex() and GetValue().
bool CompareIndexToCoords(VAPoR::Grid *grid,
                          double &     rms,                 // Root Mean Square error
                          size_t &     numMissingValues,    // Counter for receiving MissingValue upon query
                          size_t &     disagreements        // Counter for when GetValueAtIndex() and GetValue() disagree
)
{
    bool rc = true;

    rms = 0.f;
    disagreements = 0;
    numMissingValues = 0;

    auto tmp = grid->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(grid->GetNumDimensions());
    size_t x = dims[X];
    size_t y = dims.size() > 1 ? dims[Y] : 1;
    size_t z = dims.size() > 2 ? dims[Z] : 1;

    double peak = 0.f;
    double sum = 0;
    for (size_t k = 0; k < z; k++) {
        for (size_t j = 0; j < y; j++) {
            for (size_t i = 0; i < x; i++) {
                DimsType indices = {i, j, k};
                double     trueValue = grid->GetValueAtIndex(indices);

                CoordType coords;
                grid->GetUserCoordinates(indices, coords);
                float sampleValue = grid->GetValue(coords);

                if (sampleValue == grid->GetMissingValue()) {
                    numMissingValues++;
                    continue;
                }

                double error = abs(sampleValue - trueValue);

                if (!Wasp::NearlyEqual(error, 0.0)) { disagreements++; }

                if (error > peak) peak = error;
                sum += error * error;
            }
        }
    }

    rms = sqrt(sum / (x * y * z));

    if (rms != 0 || disagreements > 0) rc = false;
    return rc;
}

bool TestConstNodeIterator(const Grid *g, size_t &count, size_t &expectedCount, size_t &disagreements, double &time)
{
    bool rc = true;
    count = 0;
    expectedCount = 1;
    disagreements = 0;
    double t0 = Wasp::GetTime();

    Grid::ConstNodeIterator itr;
    Grid::ConstNodeIterator enditr = g->ConstNodeEnd();

    itr = g->ConstNodeBegin();

    auto tmp = g->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(g->GetNumDimensions());
    for (auto dim : dims) expectedCount *= dim;

    for (; itr != enditr; ++itr) {
        std::vector<size_t> ijk = Wasp::VectorizeCoords(count, dims);
        DimsType          ijk3;
        std::copy_n(ijk.begin(), ijk3.size(), ijk3.begin());

        DimsType itr3;
        std::copy_n((*itr).begin(), itr3.size(), itr3.begin());

        double itrData = g->GetValueAtIndex(itr3);
        double gridData = g->GetValueAtIndex(ijk3);

        if (!Wasp::NearlyEqual(itrData, gridData)) { disagreements++; }

        count++;
    }

    time = Wasp::GetTime() - t0;

    if (expectedCount != count || disagreements > 0) { rc = false; }
    return rc;
}

bool TestIterator(Grid *g, size_t &count, size_t &expectedCount, size_t &disagreements, double &time)
{
    bool rc = true;
    count = 0;
    expectedCount = 1;
    disagreements = 0;
    double t0 = Wasp::GetTime();

    Grid::Iterator itr;
    Grid::Iterator enditr = g->end();

    itr = g->begin();

    auto tmp = g->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(g->GetNumDimensions());
    for (auto dim : dims) expectedCount *= dim;

    for (; itr != enditr; ++itr) {
        std::vector<size_t> ijk = Wasp::VectorizeCoords(count, dims);

        if (!Wasp::NearlyEqual(*itr, g->GetValueAtIndex(ijk))) { disagreements++; }

        count++;
    }

    time = Wasp::GetTime() - t0;

    if (expectedCount != count || disagreements > 0) { rc = false; }
    return rc;
}

bool TestConstCoordItr(const Grid *g, size_t &count, size_t &expectedCount, size_t &disagreements, double &time)
{
    bool rc = true;
    count = 0;
    expectedCount = 1;
    disagreements = 0;
    double t0 = Wasp::GetTime();

    Grid::ConstCoordItr itr;
    Grid::ConstCoordItr enditr = g->ConstCoordEnd();

    itr = g->ConstCoordBegin();

    auto tmp = g->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(g->GetNumDimensions());
    for (auto dim : dims) expectedCount *= dim;

    for (; itr != enditr; ++itr) {
        std::vector<size_t> ijkVec = Wasp::VectorizeCoords(count, dims);
        size_t              ijk[] = {ijkVec[X], ijkVec[Y], ijkVec[Z]};
        double              coords[3];

        bool disagree = false;
        g->GetUserCoordinates(ijk, coords);
        for (size_t dim = 0; dim < dims.size(); dim++) {
            if (!Wasp::NearlyEqual((*itr)[dim], coords[dim])) { disagree = true; }
        }
        if (disagree) { disagreements++; }

        count++;
    }

    time = Wasp::GetTime() - t0;

    if (expectedCount != count || disagreements > 0) { rc = false; }
    return rc;
}

void PrintStats(double rms, size_t numMissingValues, size_t disagreements, double time)
{
    cout << "    RMS error:                                           " << rms << endl;
    cout << "    Missing value count:                                 " << numMissingValues << endl;
    cout << "    GetValueAtIndex() vs GetValue() disagreement count:  " << disagreements << endl;
    cout << "    Time:                                                " << time << endl;
    cout << endl;
}

bool RunTest(Grid *grid)
{
    bool   rc = true;
    double rms;
    size_t numMissingValues;
    size_t disagreements;
    double t0 = Wasp::GetTime();

    rc = CompareIndexToCoords(grid, rms, numMissingValues, disagreements);

    if (grid->GetInterpolationOrder() == 0) {
        cout << "  Interpolation order: nearestNeighbor " << endl;
    } else {
        cout << "  Interpolation order: linear          " << endl;
    }

    double time = Wasp::GetTime() - t0;

    PrintStats(rms, numMissingValues, disagreements, time);

    if (rc == false) {
        cout << "*** Error reported in " << grid->GetType() << " grid ***" << endl << endl;
    } else {
        cout << endl;
    }

    return rc;
}

bool RunTests(Grid *grid, const std::vector<std::string> &tests, float minVal, float maxVal)
{
    auto tmp = grid->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(grid->GetNumDimensions());
    size_t x = dims[X];
    size_t y = dims.size() > 1 ? dims[Y] : 1;
    size_t z = dims.size() > 2 ? dims[Z] : 1;

    bool        rc = true;
    std::string type = grid->GetType();

    cout << "=======================================================" << endl << endl;
    if (std::find(tests.begin(), tests.end(), "Constant") != tests.end()) {
        cout << type << " " << x << "x" << y << "x" << z << " Constant field:" << endl;
        MakeConstantField(grid, maxVal);

        grid->SetInterpolationOrder(linear);
        if (RunTest(grid) == false) { rc = false; }

        grid->SetInterpolationOrder(nearestNeighbor);
        if (RunTest(grid) == false) { rc = false; }
    }

    if (std::find(tests.begin(), tests.end(), "Ramp") != tests.end()) {
        cout << type << " " << x << "x" << y << "x" << z << " Ramp up through domain:" << endl;
        MakeRamp(grid, minVal, maxVal);

        grid->SetInterpolationOrder(linear);
        if (RunTest(grid) == false) { rc = false; }

        grid->SetInterpolationOrder(nearestNeighbor);
        if (RunTest(grid) == false) { rc = false; }
    }

    if (std::find(tests.begin(), tests.end(), "RampOnAxis") != tests.end()) {
        cout << type << " " << x << "x" << y << "x" << z << " Ramp up on Z axis:" << endl;
        MakeRampOnAxis(grid, minVal, maxVal, Z);
        grid->SetInterpolationOrder(linear);
        if (RunTest(grid) == false) { rc = false; }

        grid->SetInterpolationOrder(nearestNeighbor);
        if (RunTest(grid) == false) { rc = false; }
    }

    if (std::find(tests.begin(), tests.end(), "Triangle") != tests.end()) {
        cout << type << " " << x << "x" << y << "x" << z << " Triangle signal:" << endl;
        MakeTriangle(grid, minVal, maxVal);

        grid->SetInterpolationOrder(linear);
        if (RunTest(grid) == false) { rc = false; }
        RunTest(grid);

        grid->SetInterpolationOrder(nearestNeighbor);
        if (RunTest(grid) == false) { rc = false; }
    }

    // Iterator tests

    size_t count;
    size_t expectedCount;
    size_t disagreements;
    double time;

    if (TestIterator(grid, count, expectedCount, disagreements, time) == false) { rc = false; }

    PrintGridIteratorResults(type, "Iterator", count, expectedCount, disagreements, time);

    if (TestConstCoordItr(grid, count, expectedCount, disagreements, time) == false) { rc = false; }

    PrintGridIteratorResults(type, "ConstCoordIterator", count, expectedCount, disagreements, time);

    if (TestConstNodeIterator(grid, count, expectedCount, disagreements, time) == false) { rc = false; }

    PrintGridIteratorResults(type, "ConstNodeIterator", count, expectedCount, disagreements, time);

    return rc;
}

void PrintGridIteratorResults(std::string &gridType, std::string itrType, size_t count, size_t expectedCount, size_t disagreements, double time)
{
    std::string passFail = " --- PASS";
    if (count != expectedCount || disagreements > 0) { passFail = " --- FAIL"; }

    cout << gridType << " Grid::" << itrType << passFail << endl;
    cout << "  Count:                " << count << endl;
    ;
    cout << "  Expected Count:       " << expectedCount << endl;
    ;
    cout << "  Value Disagreements:  " << disagreements << endl;
    ;
    cout << "  Time:                 " << time << endl;
    cout << endl;
}

VAPoR::CurvilinearGrid *MakeCurvilinearTerrainGrid(const std::vector<size_t> &bs, const std::vector<double> &minu, const std::vector<double> &maxu, const std::vector<size_t> &dims)
{
    std::vector<size_t> bs2d = {bs[X], bs[Y]};
    std::vector<size_t> dims2d = {dims[X], dims[Y]};
    std::vector<double> minu2d = {minu[X], minu[Y]};
    std::vector<double> maxu2d = {maxu[X], maxu[Y]};

    std::vector<float *> xblks = AllocateBlocks(bs2d, dims2d);
    RegularGrid *        xrg = new RegularGrid(dims2d, bs2d, xblks, minu2d, maxu2d);
    MakeRampOnAxis(xrg, minu[X], maxu[X], X);

    std::vector<float *> yblks = AllocateBlocks(bs2d, dims2d);
    RegularGrid *        yrg = new RegularGrid(dims2d, bs2d, yblks, minu2d, maxu2d);
    MakeRampOnAxis(yrg, minu[Y], maxu[Y], Y);

    std::vector<float *> zblks = AllocateBlocks(bs, dims);
    RegularGrid *        zrg = new RegularGrid(dims, bs, zblks, minu, maxu);
    MakeRampOnAxis(zrg, minu[Z], maxu[Z], Z);

    std::vector<float *> blks = AllocateBlocks(bs, dims);
    CurvilinearGrid *    cg = new CurvilinearGrid(dims, bs, blks, *xrg, *yrg, *zrg, NULL);

    return (cg);
}

LayeredGrid *MakeLayeredGrid(const vector<size_t> &dims, const vector<size_t> &bs, const std::vector<double> &minu, const std::vector<double> &maxu)
{
    std::vector<float *> zCoordBlocks = AllocateBlocks(bs, dims);

    RegularGrid rg(dims, bs, zCoordBlocks, minu, maxu);
    MakeRampOnAxis(&rg, minu[Z], maxu[Z], Z);

    double         deltax = dims[X] > 1 ? maxu[X] - minu[X] / (dims[X] - 1) : 1;
    vector<double> xcoords;
    for (int i = 0; i < dims[X]; i++) { xcoords.push_back(minu[X] + (i * deltax)); }

    // Get horizontal dimensions
    //
    double         deltay = dims[Y] > 2 ? maxu[Y] - minu[Y] / (dims[Y] - 1) : 1;
    vector<double> ycoords;
    for (int i = 0; i < dims[Y]; i++) { ycoords.push_back(minu[1] + (i * deltay)); }

    std::vector<float *> dataBlocks = AllocateBlocks(bs, dims);
    LayeredGrid *        lg = new LayeredGrid(dims, bs, dataBlocks, xcoords, ycoords, rg);

    return (lg);
}

VAPoR::StretchedGrid *MakeStretchedGrid(const vector<size_t> &dims, const vector<size_t> &bs, const std::vector<double> &minu, const std::vector<double> &maxu)
{
    std::vector<double> xCoords(dims[X], 0);
    std::vector<double> yCoords(dims[Y], 0);
    std::vector<double> zCoords(dims[Z], 0);

    double xRange = maxu[X] - minu[X];
    double yRange = maxu[Y] - minu[Y];
    double zRange = maxu[Z] - minu[Z];

    double xDenom = dims[X] > 1 ? dims[X] - 1 : 1;
    double yDenom = dims[Y] > 1 ? dims[Y] - 1 : 1;
    double zDenom = dims[Z] > 1 ? dims[Z] - 1 : 1;

    // Parabolically increasing coordinates
    for (size_t i = 0; i < dims[X]; i++) {
        double xIncrement = xRange * pow(float(i) / xDenom, 2.0);
        xCoords[i] = xIncrement + minu[X];
    }
    for (size_t i = 0; i < dims[Y]; i++) {
        double yIncrement = yRange * pow(float(i) / yDenom, 2.0);
        yCoords[i] = yIncrement + minu[Y];
    }
    for (size_t i = 0; i < dims[Z]; i++) {
        double zIncrement = zRange * pow(float(i) / zDenom, 2.0);
        zCoords[i] = zIncrement + minu[Z];
    }

    vector<float *> blocks = AllocateBlocks(bs, dims);
    StretchedGrid * sg = new StretchedGrid(dims, bs, blocks, xCoords, yCoords, zCoords);
    return sg;
}

VAPoR::UnstructuredGrid2D *MakeUnstructuredGrid2D(const vector<size_t> &dims, const vector<size_t> &bs, const std::vector<double> &minu, const std::vector<double> &maxu)
{
    VAssert(dims.size() >= 2);
    VAssert(bs.size() >= 2);

    vector<size_t>             bs1d = {bs[0] * bs[1]};
    vector<size_t>             dims1d = {dims[0] * dims[1]};
    vector<size_t>             vertexDims = {dims[0] * dims[1]};
    vector<size_t>             faceDims = {(dims[0] - 1) * (dims[1] - 1) * 2};
    vector<size_t>             edgeDims;
    UnstructuredGrid::Location location = UnstructuredGrid2D::Location::NODE;
    size_t                     maxVertexPerFace = 3;    // each cell is a triangle
    size_t                     maxFacePerVertex = 6;    // each interior vertex defines 6 triangles
    long                       vertexOffset = 0;
    long                       faceOffset = 0;

    const int *faceOnFace = NULL;

    std::vector<float *> xCoordBlocks = AllocateBlocksType<float>(bs1d, dims1d);
    std::vector<float *> yCoordBlocks = AllocateBlocksType<float>(bs1d, dims1d);
    std::vector<int *>   vertexOnFace = AllocateBlocksType<int>(vector<size_t>{faceDims[0] * maxVertexPerFace}, vector<size_t>{faceDims[0] * maxVertexPerFace});

    size_t face = 0;
    for (size_t j = 0; j < dims[1] - 1; j++) {
        for (size_t i = 0; i < dims[0] - 1; i++) {
            vertexOnFace[0][face + 0] = j * (dims[0]) + i;
            vertexOnFace[0][face + 1] = j * (dims[0]) + i + 1;
            vertexOnFace[0][face + 2] = (j + 1) * (dims[0]) + i;

            face += maxVertexPerFace;

            vertexOnFace[0][face + 0] = j * (dims[0]) + i + 1;
            vertexOnFace[0][face + 1] = (j + 1) * (dims[0]) + i + 1;
            vertexOnFace[0][face + 2] = (j + 1) * (dims[0]) + i;

            face += maxVertexPerFace;
        }
    }

    std::vector<int *> faceOnVertex = AllocateBlocksType<int>(vector<size_t>{vertexDims[0] * maxFacePerVertex}, vector<size_t>{vertexDims[0] * maxFacePerVertex});


    // In the diagram below the x's are nodes and the triangle faces are numbered 0 to 7
    //
    // The faces connected to the center node (x) are 6,5,4,1,2,3.
    // The faces connected to the bottom, left node (x) are 0
    // The faces connected to the bottom, right node (x) are 3,2
    //
    //  x---x---x
    //  |4\5|6\7|
    //  x---x---x
    //	|0\1|2\3|
    //  x---x---x
    //
    int vertex = 0;
    for (long j = 0; j < dims[1]; j++) {
        int leftMostFaceTop = (j * (dims[0] - 1)) * 2;
        int rightMostFaceTop = leftMostFaceTop + 2 * (dims[0] - 1) - 1;

        int leftMostFaceBot = ((j - 1) * (dims[0] - 1)) * 2;
        int rightMostFaceBot = leftMostFaceBot + 2 * (dims[0] - 1) - 1;

        for (long i = 0; i < dims[0]; i++, vertex++) {
            // Initialize to missing faces
            //
            int face_i = 0;
            for (face_i = 0; face_i < maxFacePerVertex; face_i++) { faceOnVertex[0][(vertex * maxFacePerVertex) + face_i] = -1; }

            // No top of triangles for last row of nodes
            //
            face_i = 0;
            if (j < (dims[1] - 1)) {
                int face = ((j) * ((int)dims[0] - 1) * 2) + (2 * i);

                // top row of triangles - iterate in CC order
                //
                for (int ii = 0; ii < 3; ii++, face--) {
                    if (face < leftMostFaceTop || face > rightMostFaceTop) continue;
                    faceOnVertex[0][(vertex * maxFacePerVertex) + face_i] = face;
                    face_i++;
                }
            }

            // No bottom row of triangles for first row of nodes
            //
            if (j > 0) {
                int face = ((j - 1) * ((int)dims[0] - 1) * 2) + (2 * i) - 1;

                // bottom row of triangles - iterate in CC order
                //
                for (int ii = 0; ii < 3; ii++, face++) {
                    if (face < leftMostFaceBot || face > rightMostFaceBot) continue;
                    faceOnVertex[0][(vertex * maxFacePerVertex) + face_i] = face;
                    face_i++;
                }
            }
        }
    }

    UnstructuredGridCoordless xug(vertexDims, faceDims, edgeDims, bs1d, xCoordBlocks, 2, vertexOnFace[0], faceOnVertex[0], faceOnFace, location, maxVertexPerFace, maxFacePerVertex, vertexOffset,
                                  faceOffset);

    UnstructuredGridCoordless yug(vertexDims, faceDims, edgeDims, bs1d, yCoordBlocks, 2, vertexOnFace[0], faceOnVertex[0], faceOnFace, location, maxVertexPerFace, maxFacePerVertex, vertexOffset,
                                  faceOffset);

    float deltaX = 1.0 / (dims[0] - 1);
    float deltaY = 1.0 / (dims[1] - 1);
    for (long j = 0; j < dims[1]; j++) {
        for (long i = 0; i < dims[0]; i++) {
            DimsType indices = {j * dims[0] + i, 0, 0};

            xug.SetValue(indices, i * deltaX);
            yug.SetValue(indices, j * deltaY);
        }
    }

    UnstructuredGridCoordless zug;

    vector<float *>     blocks = AllocateBlocks(bs1d, dims1d);
    UnstructuredGrid2D *g = new UnstructuredGrid2D(vertexDims, faceDims, edgeDims, bs1d, blocks, vertexOnFace[0], faceOnVertex[0], faceOnFace, location, maxVertexPerFace, maxFacePerVertex,
                                                   vertexOffset, faceOffset, xug, yug, zug, nullptr);

    return g;
}
