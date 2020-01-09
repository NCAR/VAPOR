#include <iostream>
#include <vector>
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

    _cellDims = Grid::GetDimensions();
    for (int i = 0; i < _cellDims.size(); i++) {
        _cellDims[i]--;
        if (_cellDims[i] < 1) _cellDims[i] = 1;
    }
}

bool StructuredGrid::GetCellNodes(const size_t cindices[], size_t nodes[], int &n) const
{
    n = 0;

    size_t cCindices[3];
    ClampCellIndex(cindices, cCindices);

    const vector<size_t> &dims = GetDimensions();

    // Cells have the same ID's as their first node
    //
    // walk counter-clockwise order
    //
    vector<size_t> indices;

    if (dims.size() == 2) {
        nodes[0] = cCindices[0];
        nodes[1] = cCindices[1];

        nodes[2] = cCindices[0] + 1;
        nodes[3] = cCindices[1];

        nodes[4] = cCindices[0] + 1;
        nodes[5] = cCindices[1] + 1;

        nodes[6] = cCindices[0];
        nodes[7] = cCindices[1] + 1;
        n = 4;

    } else if (dims.size() == 3) {
        nodes[0] = cCindices[0];
        nodes[1] = cCindices[1];
        nodes[2] = cCindices[2];

        nodes[3] = cCindices[0] + 1;
        nodes[4] = cCindices[1];
        nodes[5] = cCindices[2];

        nodes[6] = cCindices[0] + 1;
        nodes[7] = cCindices[1] + 1;
        nodes[8] = cCindices[2];

        nodes[9] = cCindices[0];
        nodes[10] = cCindices[1] + 1;
        nodes[11] = cCindices[2];

        nodes[12] = cCindices[0];
        nodes[13] = cCindices[1];
        nodes[14] = cCindices[2] + 1;

        nodes[15] = cCindices[0] + 1;
        nodes[16] = cCindices[1];
        nodes[17] = cCindices[2] + 1;

        nodes[18] = cCindices[0] + 1;
        nodes[19] = cCindices[1] + 1;
        nodes[20] = cCindices[2] + 1;

        nodes[21] = cCindices[0];
        nodes[22] = cCindices[1] + 1;
        nodes[23] = cCindices[2] + 1;
        n = 8;
    }

    // Handle dims[i] == 1
    //
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < dims.size(); i++) {
            if (nodes[j * dims.size() + i] >= dims[i]) { nodes[j * dims.size() + i] -= 1; }
        }
    }

    return (true);
}

bool StructuredGrid::GetCellNeighbors(const std::vector<size_t> &cindices, std::vector<vector<size_t>> &cells) const
{
    cells.clear();

    size_t cCindices[3];
    ClampCellIndex(cindices, cCindices);

    const vector<size_t> &dims = GetDimensions();

    VAssert((dims.size() == 2) && "3D cells not yet supported");

    // Cells have the same ID's as their first node
    //
    // walk counter-clockwise order
    //
    if (dims.size() == 2) {
        vector<size_t> indices;

        if (cCindices[1] != 0) {    // below
            indices = {cCindices[0], cCindices[1] - 1};
        }
        cells.push_back(indices);

        if (cCindices[0] != dims[0] - 2) {    // right
            indices = {cCindices[0] + 1, cCindices[1]};
        }
        cells.push_back(indices);

        if (cCindices[1] != dims[1] - 2) {    // top
            indices = {cCindices[0], cCindices[1] + 1};
        }
        cells.push_back(indices);

        if (cCindices[0] != 0) {    // left
            indices = {cCindices[0] - 1, cCindices[1]};
        }
        cells.push_back(indices);
    }
    return (true);
}

bool StructuredGrid::GetNodeCells(const std::vector<size_t> &indices, std::vector<vector<size_t>> &cells) const
{
    cells.clear();

    vector<size_t> dims = GetDimensions();
    VAssert(indices.size() == dims.size());

    VAssert((dims.size() == 2) && "3D cells not yet supported");

    // Check if invalid indices
    //
    for (int i = 0; i < indices.size(); i++) {
        if (indices[i] > (dims[i] - 1)) return (false);
    }

    if (dims.size() == 2) {
        vector<size_t> indices;

        if (indices[0] != 0 && indices[1] != 0) {    // below, left
            indices = {indices[0] - 1, indices[1] - 1};
            cells.push_back(indices);
        }

        if (indices[1] != 0) {    // below, right
            indices = {indices[0], indices[1] - 1};
            cells.push_back(indices);
        }

        if (indices[0] != (dims[0] - 1) && indices[1] != (dims[1])) {    // top, right
            indices = {indices[0], indices[1]};
            cells.push_back(indices);
        }

        if (indices[0] != 0) {    // top, top
            indices = {indices[0] - 1, indices[1]};
            cells.push_back(indices);
        }
    }
    return (true);
}

bool StructuredGrid::GetEnclosingRegion(const std::vector<double> &minu, const std::vector<double> &maxu, std::vector<size_t> &min, std::vector<size_t> &max) const
{
    VAssert(minu.size() == maxu.size());

    if (!GetIndicesCell(minu, min)) return (false);
    if (!GetIndicesCell(maxu, max)) return (false);
    for (int i = 0; i < max.size(); i++) { max[i] += 1; }

    // For curvilinear grids it's possible that minu and maxu components
    // are swapped
    //
    vector<double> newMinu, newMaxu;
    GetUserCoordinates(min, newMinu);
    GetUserCoordinates(max, newMaxu);

    for (int i = 0; i < min.size(); i++) {
        if (newMinu > newMaxu) std::swap(min[i], max[i]);
    }

    return (true);
};

void StructuredGrid::ClampCoord(std::vector<double> &coords) const
{
    VAssert(coords.size() >= GetGeometryDim());

    while (coords.size() > GetGeometryDim()) { coords.pop_back(); }

    vector<bool>   periodic = GetPeriodic();
    vector<size_t> dims = GetDimensions();

    vector<double> minu, maxu;
    GetUserExtents(minu, maxu);

    for (int i = 0; i < coords.size(); i++) {
        //
        // Handle coordinates for dimensions of length 1
        //
        if (dims[i] == 1) {
            coords[i] = minu[i];
            continue;
        }

        if (coords[i] < minu[i] && periodic[i]) {
            while (coords[i] < minu[i]) coords[i] += maxu[i] - minu[i];
        }
        if (coords[i] > maxu[i] && periodic[i]) {
            while (coords[i] > maxu[i]) coords[i] -= maxu[i] - minu[i];
        }
    }
}

bool StructuredGrid::HasInvertedCoordinateSystemHandiness() const
{
    const vector<size_t> &dims = GetDimensions();

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
