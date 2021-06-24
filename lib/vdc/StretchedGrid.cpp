#include <stdio.h>
#include <iostream>
#include "vapor/VAssert.h"
#include <cmath>
#include <cfloat>
#include <vapor/utils.h>
#include <vapor/StretchedGrid.h>
#include <vapor/KDTreeRG.h>
#include <vapor/vizutil.h>

using namespace std;
using namespace VAPoR;

void StretchedGrid::_stretchedGrid(const vector<double> &xcoords, const vector<double> &ycoords, const vector<double> &zcoords)
{
    VAssert(xcoords.size() != 0);
    VAssert(ycoords.size() != 0);

    _xcoords.clear();
    _ycoords.clear();
    _zcoords.clear();
    _xcoords = xcoords;
    _ycoords = ycoords;
    _zcoords = zcoords;

    // Get the user extents now. Do this only once.
    //
    GetUserExtentsHelper(_minu, _maxu);
}

StretchedGrid::StretchedGrid(const vector<size_t> &dims, const vector<size_t> &bs, const vector<float *> &blks, const vector<double> &xcoords, const vector<double> &ycoords,
                             const vector<double> &zcoords)
: StructuredGrid(dims, bs, blks)
{
    VAssert(bs.size() == dims.size());
    VAssert(bs.size() >= 1 && bs.size() <= 3);

    _stretchedGrid(xcoords, ycoords, zcoords);
}

size_t StretchedGrid::GetGeometryDim() const { return (_zcoords.size() == 0 ? 2 : 3); }

vector<size_t> StretchedGrid::GetCoordDimensions(size_t dim) const
{
    if (dim < 3) {
        return (vector<size_t>(1, GetDimensions()[dim]));
    } else {
        return (vector<size_t>(1, 1));
    }
}

void StretchedGrid::GetBoundingBox(const Size_tArr3 &min, const Size_tArr3 &max, DblArr3 &minu, DblArr3 &maxu) const
{
    Size_tArr3 cMin;
    ClampIndex(min, cMin);

    Size_tArr3 cMax;
    ClampIndex(max, cMax);

    for (int i = 0; i < GetNodeDimensions().size(); i++) { VAssert(cMin[i] <= cMax[i]); }

    for (int i = 0; i < GetGeometryDim(); i++) {
        minu[i] = 0.0;
        maxu[i] = 0.0;
    }

    minu[0] = _xcoords[cMin[0]];
    maxu[0] = _xcoords[cMax[0]];
    if (minu[0] > maxu[0]) std::swap(minu[0], maxu[0]);

    minu[1] = _ycoords[cMin[1]];
    maxu[1] = _ycoords[cMax[1]];
    if (minu[1] > maxu[1]) std::swap(minu[1], maxu[1]);

    // We're done if 2D grid
    //
    if (GetGeometryDim() == 2) return;

    minu[2] = _zcoords[cMin[2]];
    maxu[2] = _zcoords[cMax[2]];
    if (minu[2] > maxu[2]) std::swap(minu[2], maxu[2]);
}

void StretchedGrid::GetUserCoordinates(const Size_tArr3 &indices, DblArr3 &coords) const
{
    Size_tArr3 cIndices;
    ClampIndex(indices, cIndices);

    coords[0] = _xcoords[cIndices[0]];
    coords[1] = _ycoords[cIndices[1]];

    if (GetGeometryDim() > 2) { coords[2] = _zcoords[cIndices[2]]; }
}

bool StretchedGrid::GetIndicesCell(const DblArr3 &coords, Size_tArr3 &indices, double wgts[3]) const
{
    // Clamp coordinates on periodic boundaries to grid extents
    //
    DblArr3 cCoords;
    ClampCoord(coords, cCoords);

    double x = cCoords[0];
    double y = cCoords[1];
    double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;

    double xwgt[2], ywgt[2], zwgt[2];
    size_t i, j, k;
    bool   inside = _insideGrid(x, y, z, i, j, k, xwgt, ywgt, zwgt);

    if (!inside) return (false);
    wgts[0] = xwgt[0];
    wgts[1] = ywgt[0];

    indices[0] = i;
    indices[1] = j;

    if (GetGeometryDim() == 2) return (true);

    indices[2] = k;
    wgts[2] = zwgt[0];

    return (true);
}

bool StretchedGrid::InsideGrid(const DblArr3 &coords) const
{
    // Clamp coordinates on periodic boundaries to reside within the
    // grid extents
    //
    DblArr3 cCoords;
    ClampCoord(coords, cCoords);

    // Do a quick check to see if the point is completely outside of
    // the grid bounds.
    //
    VAssert(GetGeometryDim() <= 3);
    for (int i = 0; i < GetGeometryDim(); i++) {
        if (cCoords[i] < _minu[i] || cCoords[i] > _maxu[i]) return (false);
    }

    double xwgt[2], ywgt[2], zwgt[2];
    size_t i, j, k;    // not used
    double x = cCoords[0];
    double y = cCoords[1];
    double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;

    bool inside = _insideGrid(x, y, z, i, j, k, xwgt, ywgt, zwgt);

    return (inside);
}

StretchedGrid::ConstCoordItrSG::ConstCoordItrSG(const StretchedGrid *sg, bool begin) : ConstCoordItrAbstract()
{
    _sg = sg;
    auto tmp = _sg->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(_sg->GetNumDimensions());

    _index = vector<size_t>(dims.size(), 0);
    if (!begin) { _index[dims.size() - 1] = dims[dims.size() - 1]; }

    _coords.push_back(_sg->_xcoords[0]);
    _coords.push_back(_sg->_ycoords[0]);
    if (dims.size() == 3) { _coords.push_back(_sg->_zcoords[0]); }
}

StretchedGrid::ConstCoordItrSG::ConstCoordItrSG(const ConstCoordItrSG &rhs) : ConstCoordItrAbstract()
{
    _sg = rhs._sg;
    _index = rhs._index;
    _coords = rhs._coords;
}

StretchedGrid::ConstCoordItrSG::ConstCoordItrSG() : ConstCoordItrAbstract()
{
    _sg = NULL;
    _index.clear();
    _coords.clear();
}

void StretchedGrid::ConstCoordItrSG::next()
{
    auto tmp = _sg->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(_sg->GetNumDimensions());

    _index[0]++;

    if (_index[0] < dims[0]) {
        _coords[0] = _sg->_xcoords[_index[0]];
        _coords[1] = _sg->_ycoords[_index[1]];
        return;
    }

    _index[0] = 0;
    _index[1]++;

    if (_index[1] < dims[1]) {
        _coords[0] = _sg->_xcoords[_index[0]];
        _coords[1] = _sg->_ycoords[_index[1]];
        return;
    }

    if (dims.size() == 2) return;

    _index[1] = 0;
    _index[2]++;
    if (_index[2] < dims[2]) {
        _coords[0] = _sg->_xcoords[_index[0]];
        _coords[1] = _sg->_ycoords[_index[1]];
        _coords[2] = _sg->_zcoords[_index[2]];
        return;
    }
}

void StretchedGrid::ConstCoordItrSG::next(const long &offset)
{
    auto tmp = _sg->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(_sg->GetNumDimensions());

    if (!_index.size()) return;

    vector<size_t> maxIndex;
    ;
    for (int i = 0; i < dims.size(); i++) maxIndex.push_back(dims[i] - 1);

    long maxIndexL = Wasp::LinearizeCoords(maxIndex, dims);
    long newIndexL = Wasp::LinearizeCoords(_index, dims) + offset;
    if (newIndexL < 0) { newIndexL = 0; }
    if (newIndexL > maxIndexL) {
        _index = vector<size_t>(dims.size(), 0);
        _index[dims.size() - 1] = dims[dims.size() - 1];
        return;
    }

    _index = Wasp::VectorizeCoords(newIndexL, dims);

    _coords[0] = _sg->_xcoords[_index[0]];
    _coords[1] = _sg->_ycoords[_index[1]];

    if (dims.size() == 2) return;

    _coords[2] = _sg->_zcoords[_index[2]];
}

float StretchedGrid::GetValueNearestNeighbor(const DblArr3 &coords) const
{
    // Clamp coordinates on periodic boundaries to grid extents
    //
    DblArr3 cCoords;
    ClampCoord(coords, cCoords);

    double xwgt[2], ywgt[2], zwgt[2];
    size_t i, j, k;
    double x = cCoords[0];
    double y = cCoords[1];
    double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;
    bool   inside = _insideGrid(x, y, z, i, j, k, xwgt, ywgt, zwgt);

    if (xwgt[1] > xwgt[0]) i++;
    if (ywgt[1] > ywgt[0]) j++;
    if (zwgt[1] > zwgt[0]) k++;

    if (!inside) return (GetMissingValue());

    return (AccessIJK(i, j, k));
}

float StretchedGrid::GetValueLinear(const DblArr3 &coords) const
{
    // Clamp coordinates on periodic boundaries to grid extents
    //
    DblArr3 cCoords;
    ClampCoord(coords, cCoords);

    // handlese case where grid is 2D. I.e. if 2d then zwgt[0] == 1 &&
    // zwgt[1] = 0.0
    //
    double xwgt[2], ywgt[2], zwgt[2];
    size_t i, j, k;
    double x = cCoords[0];
    double y = cCoords[1];
    double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;
    bool   inside = _insideGrid(x, y, z, i, j, k, xwgt, ywgt, zwgt);

    if (!inside) return (GetMissingValue());

    auto tmp = GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(GetNumDimensions());
    VAssert(i < dims[0]);
    VAssert(j < dims[1]);
    if (dims.size() > 2) VAssert(k < dims[2]);

    float verts0[4];
    verts0[0] = AccessIJK(i, j, k);
    verts0[1] = dims[0] > 1 ? AccessIJK(i + 1, j, k) : 0.0;
    verts0[2] = dims[1] > 1 ? AccessIJK(i, j + 1, k) : 0.0;
    verts0[3] = dims[0] > 1 && dims[1] > 1 ? AccessIJK(i + 1, j + 1, k) : 0.0;

    float v0 = ((verts0[0] * xwgt[0] + verts0[1] * xwgt[1]) * ywgt[0]) + ((verts0[2] * xwgt[0] + verts0[3] * xwgt[1]) * ywgt[1]);

    if (GetGeometryDim() == 2) return (v0);

    if (dims[2] > 1) k++;

    float verts1[4];
    verts1[0] = AccessIJK(i, j, k);
    verts1[1] = dims[0] > 1 ? AccessIJK(i + 1, j, k) : 0.0;
    verts1[2] = dims[1] > 1 ? AccessIJK(i, j + 1, k) : 0.0;
    verts1[3] = dims[0] > 1 && dims[1] > 1 ? AccessIJK(i + 1, j + 1, k) : 0.0;

    float v1 = ((verts1[0] * xwgt[0] + verts1[1] * xwgt[1]) * ywgt[0]) + ((verts1[2] * xwgt[0] + verts1[3] * xwgt[1]) * ywgt[1]);


    // Linearly interpolate along Z axis
    //
    return (v0 * zwgt[0] + v1 * zwgt[1]);
}

void StretchedGrid::GetUserExtentsHelper(DblArr3 &minext, DblArr3 &maxext) const
{
    auto tmp = StructuredGrid::GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(StructuredGrid::GetNumDimensions());

    Size_tArr3 min, max;
    for (int i = 0; i < dims.size(); i++) {
        min[i] = 0;
        max[i] = (dims[i] - 1);
    }

    DblArr3 minv, maxv;
    StretchedGrid::GetBoundingBox(min, max, minv, maxv);
    for (int i = 0; i < GetNumDimensions(); i++) {
        minext[i] = minv[i];
        maxext[i] = maxv[i];
    }
}

// Search for a point inside the grid. If the point is inside return true,
// and provide the weights/coordinates for the point within
// the XYZ cell containing the point
// If the point is outside of the
// grid the values of 'xwgt', 'ywgt', and 'zwgt' are not defined
//
bool StretchedGrid::_insideGrid(double x, double y, double z, size_t &i, size_t &j, size_t &k, double xwgt[2], double ywgt[2], double zwgt[2]) const
{
    for (int l = 0; l < 2; l++) {
        xwgt[l] = 0.0;
        ywgt[l] = 0.0;
        zwgt[l] = 0.0;
    }
    i = j = k = 0;

    if (!Wasp::BinarySearchRange(_xcoords, x, i)) return (false);

    if (_xcoords.size() > 1) {
        xwgt[0] = 1.0 - (x - _xcoords[i]) / (_xcoords[i + 1] - _xcoords[i]);
        xwgt[1] = 1.0 - xwgt[0];
    } else {
        xwgt[0] = 1.0;
    }


    if (!Wasp::BinarySearchRange(_ycoords, y, j)) return (false);

    if (_ycoords.size() > 1) {
        ywgt[0] = 1.0 - (y - _ycoords[j]) / (_ycoords[j + 1] - _ycoords[j]);
        ywgt[1] = 1.0 - ywgt[0];
    } else {
        ywgt[0] = 1.0;
    }

    if (GetGeometryDim() == 2) {
        zwgt[0] = 1.0;
        zwgt[1] = 0.0;
        return (true);
    }

    // Now verify that Z coordinate of point is in grid, and find
    // its interpolation weights if so.
    //
    if (!Wasp::BinarySearchRange(_zcoords, z, k)) return (false);

    if (_zcoords.size() > 1) {
        zwgt[0] = 1.0 - (z - _zcoords[k]) / (_zcoords[k + 1] - _zcoords[k]);
        zwgt[1] = 1.0 - zwgt[0];
    } else {
        zwgt[0] = 1.0;
    }

    return (true);
}
