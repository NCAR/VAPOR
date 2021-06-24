#include <iostream>
#include <vector>
#include <algorithm>
#include "vapor/VAssert.h"
#include <cmath>
#include <time.h>
#include <glm/glm.hpp>

#ifdef Darwin
    #include <mach/mach_time.h>
#endif
#ifdef _WINDOWS
    #include "windows.h"
    #include "Winbase.h"
    #include <limits>
#endif

#include <vapor/utils.h>
#include <vapor/StructuredGrid.h>

using namespace std;
using namespace VAPoR;

StructuredGrid::StructuredGrid(const vector<size_t> &dims, const vector<size_t> &bs, const vector<float *> &blks) : Grid(dims, bs, blks, dims.size())
{
    VAssert(bs.size() == 2 || bs.size() == 3);

    auto tmp = Grid::GetDimensions();
    _cellDims = {tmp[0], tmp[1], tmp[2]};
    _cellDims.resize(Grid::GetNumDimensions());
    for (int i = 0; i < _cellDims.size(); i++) {
        _cellDims[i]--;
        if (_cellDims[i] < 1) _cellDims[i] = 1;
    }
}

bool StructuredGrid::GetCellNodes(const Size_tArr3 &cindices, vector<Size_tArr3> &nodes) const
{
    Size_tArr3 cCindices;
    ClampCellIndex(cindices, cCindices);

    auto tmp = GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(GetNumDimensions());

    // Cells have the same ID's as their first node
    //
    // walk counter-clockwise order
    //

    if (dims.size() == 2) {
        nodes.resize(4);
        nodes[0][0] = cCindices[0];
        nodes[0][1] = cCindices[1];
        nodes[0][2] = 0;

        nodes[1][0] = cCindices[0] + 1;
        nodes[1][1] = cCindices[1];
        nodes[1][2] = 0;

        nodes[2][0] = cCindices[0] + 1;
        nodes[2][1] = cCindices[1] + 1;
        nodes[2][2] = 0;

        nodes[3][0] = cCindices[0];
        nodes[3][1] = cCindices[1] + 1;
        nodes[3][2] = 0;

    } else if (dims.size() == 3) {
        nodes.resize(8);
        nodes[0][0] = cCindices[0];
        nodes[0][1] = cCindices[1];
        nodes[0][2] = cCindices[2];

        nodes[1][0] = cCindices[0] + 1;
        nodes[1][1] = cCindices[1];
        nodes[1][2] = cCindices[2];

        nodes[2][0] = cCindices[0] + 1;
        nodes[2][1] = cCindices[1] + 1;
        nodes[2][2] = cCindices[2];

        nodes[3][0] = cCindices[0];
        nodes[3][1] = cCindices[1] + 1;
        nodes[3][2] = cCindices[2];

        nodes[4][0] = cCindices[0];
        nodes[4][1] = cCindices[1];
        nodes[4][2] = cCindices[2] + 1;

        nodes[5][0] = cCindices[0] + 1;
        nodes[5][1] = cCindices[1];
        nodes[5][2] = cCindices[2] + 1;

        nodes[6][0] = cCindices[0] + 1;
        nodes[6][1] = cCindices[1] + 1;
        nodes[6][2] = cCindices[2] + 1;

        nodes[7][0] = cCindices[0];
        nodes[7][1] = cCindices[1] + 1;
        nodes[7][2] = cCindices[2] + 1;
    }

    // Handle dims[i] == 1
    //
    for (int j = 0; j < nodes.size(); j++) {
        for (int i = 0; i < dims.size(); i++) {
            if (nodes[j][i] >= dims[i]) { nodes[j][i] -= 1; }
        }
    }

    return (true);
}

bool StructuredGrid::GetCellNeighbors(const Size_tArr3 &cindices, std::vector<Size_tArr3> &cells) const
{
    cells.clear();

    Size_tArr3 cCindices;
    ClampCellIndex(cindices, cCindices);

    auto tmp = GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(GetNumDimensions());

    VAssert((dims.size() == 2) && "3D cells not yet supported");

    // Cells have the same ID's as their first node
    //
    // walk counter-clockwise order
    //
    if (dims.size() == 2) {
        Size_tArr3 indices;

        if (cCindices[1] != 0) {    // below
            indices = {cCindices[0], cCindices[1] - 1, 0};
        }
        cells.push_back(indices);

        if (cCindices[0] != dims[0] - 2) {    // right
            indices = {cCindices[0] + 1, cCindices[1], 0};
        }
        cells.push_back(indices);

        if (cCindices[1] != dims[1] - 2) {    // top
            indices = {cCindices[0], cCindices[1] + 1, 0};
        }
        cells.push_back(indices);

        if (cCindices[0] != 0) {    // left
            indices = {cCindices[0] - 1, cCindices[1], 0};
        }
        cells.push_back(indices);
    }
    return (true);
}

bool StructuredGrid::GetNodeCells(const Size_tArr3 &indices, std::vector<Size_tArr3> &cells) const
{
    cells.clear();

    auto tmp = GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(GetNumDimensions());

    VAssert((dims.size() == 2) && "3D cells not yet supported");

    // Check if invalid indices
    //
    for (int i = 0; i < GetGeometryDim(); i++) {
        if (indices[i] > (dims[i] - 1)) return (false);
    }

    if (dims.size() == 2) {
        Size_tArr3 indices;

        if (indices[0] != 0 && indices[1] != 0) {    // below, left
            indices = {indices[0] - 1, indices[1] - 1, 0};
            cells.push_back(indices);
        }

        if (indices[1] != 0) {    // below, right
            indices = {indices[0], indices[1] - 1, 0};
            cells.push_back(indices);
        }

        if (indices[0] != (dims[0] - 1) && indices[1] != (dims[1])) {    // top, right
            indices = {indices[0], indices[1], 0};
            cells.push_back(indices);
        }

        if (indices[0] != 0) {    // top, top
            indices = {indices[0] - 1, indices[1], 0};
            cells.push_back(indices);
        }
    }
    return (true);
}

bool StructuredGrid::GetEnclosingRegion(const DblArr3 &minu, const DblArr3 &maxu, Size_tArr3 &min, Size_tArr3 &max) const
{
    if (!GetIndicesCell(minu, min)) return (false);
    if (!GetIndicesCell(maxu, max)) return (false);
    for (int i = 0; i < GetNumDimensions(); i++) { max[i] += 1; }

    // For curvilinear grids it's possible that minu and maxu components
    // are swapped
    //
    DblArr3 newMinu, newMaxu;
    GetUserCoordinates(min.data(), newMinu.data());
    GetUserCoordinates(max.data(), newMaxu.data());

    for (int i = 0; i < GetNumDimensions(); i++) {
        if (newMinu > newMaxu) std::swap(min[i], max[i]);
    }

    return (true);
};

void StructuredGrid::ClampCoord(const DblArr3 &coords, DblArr3 &cCoords) const
{
    const vector<bool> &periodic = GetPeriodic();

    size_t n = min(GetGeometryDim(), periodic.size());
    auto   p = [](bool v) { return (v == true); };
    if (std::none_of(periodic.begin(), periodic.begin() + n, p)) {
        cCoords = coords;
        return;
    }

    auto tmp = GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(GetNumDimensions());

    DblArr3 minu, maxu;
    GetUserExtents(minu, maxu);

    cCoords = coords;
    VAssert(GetGeometryDim() <= 3);
    for (int i = 0; i < GetGeometryDim(); i++) {
        //
        // Handle coordinates for dimensions of length 1
        //
        if (dims[i] == 1) {
            cCoords[i] = minu[i];
            continue;
        }

        if (cCoords[i] < minu[i] && periodic[i]) {
            while (cCoords[i] < minu[i]) cCoords[i] += maxu[i] - minu[i];
        }
        if (cCoords[i] > maxu[i] && periodic[i]) {
            while (cCoords[i] > maxu[i]) cCoords[i] -= maxu[i] - minu[i];
        }
    }
}

bool StructuredGrid::HasInvertedCoordinateSystemHandiness() const
{
    auto tmp = GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(GetNumDimensions());

    if (dims.size() < 2) return (true);    // Arbitrary

    size_t vi0[] = {0, 0, 0};
    size_t vi1[] = {1, 0, 0};
    size_t vi2[] = {0, 1, 0};

    double v0[3], v1[3], v2[3];

    GetUserCoordinates(vi0, v0);
    GetUserCoordinates(vi1, v1);
    GetUserCoordinates(vi2, v2);

    glm::vec3 glm_v0(v0[0], v0[1], v0[2]);
    glm::vec3 glm_v1(v1[0], v1[1], v1[2]);
    glm::vec3 glm_v2(v2[0], v2[1], v2[2]);

    glm::vec3 c2d = glm::cross(glm_v1 - glm_v0, glm_v2 - glm_v0);

    // CCW if Z component of cross product is positive
    //
    if (dims.size() == 2) return (c2d[2] >= 0.0);

    size_t vi3[] = {0, 0, 1};
    double v3[3];

    GetUserCoordinates(vi3, v3);

    return (c2d[2] * (v3[2] - v0[2]) >= 0.0);
}

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const StructuredGrid &sg)
{
    o << "StructuredGrid " << endl;
    o << endl;

    o << (const Grid &)sg;

    return o;
}
};    // namespace VAPoR
