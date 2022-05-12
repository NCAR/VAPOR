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

StretchedGrid::StretchedGrid(const DimsType &dims, const DimsType &bs, const vector<float *> &blks, const vector<double> &xcoords, const vector<double> &ycoords, const vector<double> &zcoords)
: StructuredGrid(dims, bs, blks)
{
    _stretchedGrid(xcoords, ycoords, zcoords);
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

DimsType StretchedGrid::GetCoordDimensions(size_t dim) const
{
    DimsType dims = {1, 1, 1};

    if (dim < 3) { dims[0] = GetDimensions()[dim]; }

    return (dims);
}

void StretchedGrid::GetBoundingBox(const DimsType &min, const DimsType &max, CoordType &minu, CoordType &maxu) const
{
    DimsType cMin;
    ClampIndex(min, cMin);

    DimsType cMax;
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

void StretchedGrid::GetUserCoordinates(const DimsType &indices, CoordType &coords) const
{
    DimsType cIndices;
    ClampIndex(indices, cIndices);

    coords[0] = _xcoords[cIndices[0]];
    coords[1] = _ycoords[cIndices[1]];

    if (GetGeometryDim() > 2) { coords[2] = _zcoords[cIndices[2]]; }
}

bool StretchedGrid::GetIndicesCell(const CoordType &coords, DimsType &indices, double wgts[3]) const
{
    // Clamp coordinates on periodic boundaries to grid extents
    //
    CoordType cCoords;
    ClampCoord(coords, cCoords);

    double x = cCoords[0];
    double y = cCoords[1];
    double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;

    size_t i, j, k;
    wgts[0] = 0.0; wgts[1] = 0.0; wgts[2] = 0.0;
    bool   inside = _insideGrid(x, y, z, i, j, k, wgts[0], wgts[1], wgts[2]);

    if (!inside) return (false);

    indices[0] = i;
    indices[1] = j;

    if (GetGeometryDim() == 2) return (true);

    indices[2] = k;

    return (true);
}

bool StretchedGrid::InsideGrid(const CoordType &coords) const
{
    // Clamp coordinates on periodic boundaries to reside within the
    // grid extents
    //
    CoordType cCoords;
    ClampCoord(coords, cCoords);

    // Do a quick check to see if the point is completely outside of
    // the grid bounds.
    //
    VAssert(GetGeometryDim() <= 3);
    for (int i = 0; i < GetGeometryDim(); i++) {
        if (cCoords[i] < _minu[i] || cCoords[i] > _maxu[i]) return (false);
    }

    double xwgt, ywgt, zwgt;
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
    _index = {0, 0, 0};
    _coords = {0.0, 0.0, 0.0};
    const DimsType &dims = sg->GetDimensions();

    if (!begin) { _index = {0, 0, dims[dims.size() - 1]}; }

    if (_sg->_xcoords.size()) _coords[0] = _sg->_xcoords[0];
    if (_sg->_ycoords.size()) _coords[1] = _sg->_ycoords[0];
    if (_sg->_zcoords.size()) _coords[2] = _sg->_zcoords[0];
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
    _index = {0, 0, 0};
    _coords = {0.0, 0.0, 0.0};
}

void StretchedGrid::ConstCoordItrSG::next()
{
    auto dims = _sg->GetDimensions();

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

    _index[1] = 0;
    _index[2]++;
    if (_index[2] < dims[2]) {
        _coords[0] = _sg->_xcoords[_index[0]];
        _coords[1] = _sg->_ycoords[_index[1]];
        _coords[2] = _sg->_zcoords[_index[2]];
        return;
    }

    _index[2] = dims[2];    // last index;
}

void StretchedGrid::ConstCoordItrSG::next(const long &offset)
{
    auto dims = _sg->GetDimensions();

    long maxIndexL = Wasp::VProduct(dims.data(), dims.size()) - 1;
    long newIndexL = Wasp::LinearizeCoords(_index.data(), dims.data(), dims.size()) + offset;
    if (newIndexL < 0) { newIndexL = 0; }
    if (newIndexL > maxIndexL) {
        _index = {0, 0, dims[dims.size() - 1]};
        return;
    }

    _index = {0, 0, 0};
    Wasp::VectorizeCoords(newIndexL, dims.data(), _index.data(), dims.size());

    _coords[0] = _sg->_xcoords[_index[0]];
    _coords[1] = _sg->_ycoords[_index[1]];
    _coords[2] = _sg->_zcoords[_index[2]];
}

float StretchedGrid::GetValueNearestNeighbor(const CoordType &coords) const
{
    // Clamp coordinates on periodic boundaries to grid extents
    //
    CoordType cCoords;
    ClampCoord(coords, cCoords);

    double wgts[] = {0.0, 0.0, 0.0};
    size_t i, j, k;
    double x = cCoords[0];
    double y = cCoords[1];
    double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;
    bool   inside = _insideGrid(x, y, z, i, j, k, wgts[0], wgts[1], wgts[2]);

    if (wgts[0] < 0.5) i++;
    if (wgts[1] < 0.5) j++;
    if (wgts[2] < 0.5) k++;

    if (!inside) return (GetMissingValue());

    return (AccessIJK(i, j, k));
}

float StretchedGrid::GetValueLinear(const CoordType &coords) const
{
    // Clamp coordinates on periodic boundaries to grid extents
    //
    CoordType cCoords;
    ClampCoord(coords, cCoords);

    // handlese case where grid is 2D. I.e. if 2d then zwgt[0] == 1 &&
    // zwgt[1] = 0.0
    //
    double wgts[] = {0.0, 0.0, 0.0};
    size_t i, j, k;
    double x = cCoords[0];
    double y = cCoords[1];
    double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;
    bool   inside = _insideGrid(x, y, z, i, j, k, wgts[0], wgts[1], wgts[2]);


    float mv = GetMissingValue();
    if (!inside) return (mv);

    return(TrilinearInterpolate(i,j,k, wgts[0], wgts[1], wgts[2]));

}

void StretchedGrid::GetUserExtentsHelper(CoordType &minext, CoordType &maxext) const
{
    auto dims = StructuredGrid::GetDimensions();

    DimsType min = {0, 0, 0};
    DimsType max = {0, 0, 0};
    for (int i = 0; i < dims.size(); i++) {
        assert(dims[i] > 0);    // will help debug
        max[i] = (dims[i] - 1);
    }

    CoordType minv, maxv;
    StretchedGrid::GetBoundingBox(min, max, minv, maxv);
    for (int i = 0; i < min.size(); i++) {
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
bool StretchedGrid::_insideGrid(double x, double y, double z, size_t &i, size_t &j, size_t &k, double &xwgt, double &ywgt, double &zwgt) const
{
	xwgt = 0.0;
	ywgt = 0.0;
	zwgt = 0.0;
    i = j = k = 0;

    if (!Wasp::BinarySearchRange(_xcoords, x, i)) return (false);

    if (_xcoords.size() > 1) {
        xwgt = 1.0 - (x - _xcoords[i]) / (_xcoords[i + 1] - _xcoords[i]);
    } else {
        xwgt = 1.0;
    }


    if (!Wasp::BinarySearchRange(_ycoords, y, j)) return (false);

    if (_ycoords.size() > 1) {
        ywgt = 1.0 - (y - _ycoords[j]) / (_ycoords[j + 1] - _ycoords[j]);
    } else {
        ywgt = 1.0;
    }

    if (GetGeometryDim() == 2) {
        zwgt = 1.0;
        return (true);
    }

    // Now verify that Z coordinate of point is in grid, and find
    // its interpolation weights if so.
    //
    if (!Wasp::BinarySearchRange(_zcoords, z, k)) return (false);

    if (_zcoords.size() > 1) {
        zwgt = 1.0 - (z - _zcoords[k]) / (_zcoords[k + 1] - _zcoords[k]);
    } else {
        zwgt = 1.0;
    }

    return (true);
}
