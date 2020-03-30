#include <stdio.h>
#include <iostream>
#include <cmath>
#include <cfloat>
#include "vapor/utils.h"
#include "vapor/LayeredGrid.h"
#define INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
#include "vapor/LegacyVectorMath.h"
#include "vapor/VAssert.h"

using namespace std;
using namespace VAPoR;

void LayeredGrid::_layeredGrid(const vector<double> &minu, const vector<double> &maxu, const RegularGrid &rg)
{
    VAssert(GetDimensions().size() == 3);
    VAssert(minu.size() == maxu.size());
    VAssert(minu.size() == 2);

    _minu.clear();
    _maxu.clear();
    _delta.clear();
    _interpolationOrder = 1;

    _rg = rg;
    _minu = minu;
    _maxu = maxu;

    // Coordinates for horizontal dimensions
    //
    vector<size_t> dims = GetDimensions();
    for (int i = 0; i < _minu.size(); i++) {
        if (dims[i] < 2)
            _delta.push_back(0.0);
        else
            _delta.push_back((_maxu[i] - _minu[i]) / (double)(dims[i] - 1));
    }

    // Get extents of layered dimension
    //
    float range[2];
    _rg.GetRange(range);
    _minu.push_back(range[0]);
    _maxu.push_back(range[1]);
}

LayeredGrid::LayeredGrid(const vector<size_t> &dims, const vector<size_t> &bs, const vector<float *> &blks, const vector<double> &minu, const vector<double> &maxu, const RegularGrid &rg)
: StructuredGrid(dims, bs, blks)
{
    _layeredGrid(minu, maxu, rg);
}

vector<size_t> LayeredGrid::GetCoordDimensions(size_t dim) const
{
    if (dim == 0) {
        return (vector<size_t>(1, GetDimensions()[0]));
    } else if (dim == 1) {
        return (vector<size_t>(1, GetDimensions()[1]));
    } else if (dim == 2) {
        return (_rg.GetDimensions());
    } else {
        return (vector<size_t>(1, 1));
    }
}

void LayeredGrid::GetUserExtents(vector<double> &minu, vector<double> &maxu) const
{
    minu = _minu;
    maxu = _maxu;
}

void LayeredGrid::GetBoundingBox(const vector<size_t> &min, const vector<size_t> &max, vector<double> &minu, vector<double> &maxu) const
{
    vector<size_t> cMin = min;
    ClampIndex(cMin);

    vector<size_t> cMax = max;
    ClampIndex(cMax);

    VAssert(cMin.size() == cMax.size());

    minu.clear();
    maxu.clear();

    // Get extents of horizontal dimensions. Note: also get vertical
    // dimension, but it's bogus for layered grid.
    //
    Grid::GetUserCoordinates(cMin, minu);
    Grid::GetUserCoordinates(cMax, maxu);

    // Initialize min and max coordinates of varying dimension with
    // coordinates of "first" and "last" grid point. Coordinates of
    // varying dimension are stored as values of a scalar function
    // sampling the coordinate space.
    //
    float mincoord = _rg.GetValueAtIndex(cMin);
    float maxcoord = _rg.GetValueAtIndex(cMax);

    // Now find the extreme values of the varying dimension's coordinates
    //
    for (int j = cMin[1]; j <= cMax[1]; j++) {
        for (int i = cMin[0]; i <= cMax[0]; i++) {
            float v = _rg.AccessIJK(i, j, cMin[2]);

            if (v < mincoord) mincoord = v;
        }
    }

    for (int j = cMin[1]; j <= cMax[1]; j++) {
        for (int i = cMin[0]; i <= cMax[0]; i++) {
            float v = _rg.AccessIJK(i, j, cMax[2]);

            if (v > maxcoord) maxcoord = v;
        }
    }

    minu[2] = mincoord;
    maxu[2] = maxcoord;
}

bool LayeredGrid::_getCellAndWeights(const double coords[3], size_t indices0[3], double wgts[3]) const
{
    // Get the indecies of the cell containing the point. No raw pointer
    // version of GetIndicesCell()
    //
    vector<double> coordsv = {coords[0], coords[1], coords[2]};
    vector<size_t> indices0v;
    if (!GetIndicesCell(coordsv, indices0v)) return (false);
    VAssert(indices0v.size() == 3);

    size_t indices1[3];
    for (int i = 0; i < 3; i++) {
        indices0[i] = indices0v[i];
        indices1[i] = indices0v[i] + 1;
    }

    // Get user coordinates of cell containing point
    //
    double coords0[3], coords1[3];
    GetUserCoordinates(indices0, coords0);
    GetUserCoordinates(indices1, coords1);

    double x = coords[0];
    double y = coords[1];
    double z = coords[2];
    double x0 = coords0[0];
    double y0 = coords0[1];
    double z0 = coords0[2];
    double x1 = coords1[0];
    double y1 = coords1[1];
    double z1 = coords1[2];

    //
    // Calculate interpolation weights. We always interpolate along
    // the varying dimension last (the kwgt)
    //
    z0 = _interpolateVaryingCoord(indices0[0], indices0[1], indices0[2], x, y);
    z1 = _interpolateVaryingCoord(indices0[0], indices0[1], indices1[2], x, y);

    if (x1 != x0)
        wgts[0] = fabs((x - x0) / (x1 - x0));
    else
        wgts[0] = 0.0;
    if (y1 != y0)
        wgts[1] = fabs((y - y0) / (y1 - y0));
    else
        wgts[1] = 0.0;
    if (z1 != z0)
        wgts[2] = fabs((z - z0) / (z1 - z0));
    else
        wgts[2] = 0.0;

    return (true);
}

float LayeredGrid::GetValueNearestNeighbor(const std::vector<double> &coords) const
{
    VAssert(coords.size() == 3);

    size_t indices[3];
    double wgts[3];
    bool   found = _getCellAndWeights(coords.data(), indices, wgts);
    if (!found) return (GetMissingValue());

    if (wgts[0] > 0.5) indices[0] += 1;
    if (wgts[1] > 0.5) indices[1] += 1;
    if (wgts[2] > 0.5) indices[2] += 1;

    return (AccessIJK(indices[0], indices[1], indices[2]));
}

float LayeredGrid::GetValueLinear(const std::vector<double> &coords) const
{
    VAssert(coords.size() == 3);

    size_t indices0[3];
    double wgts[3];
    bool   found = _getCellAndWeights(coords.data(), indices0, wgts);
    if (!found) return (GetMissingValue());

    size_t i0 = indices0[0];
    size_t j0 = indices0[1];
    size_t k0 = indices0[2];
    size_t i1 = indices0[0] + 1;
    size_t j1 = indices0[1] + 1;
    size_t k1 = indices0[2] + 1;

    //
    // perform tri-linear interpolation
    //
    double p0, p1, p2, p3, p4, p5, p6, p7;
    double iwgt = wgts[0];
    double jwgt = wgts[1];
    double kwgt = wgts[2];

    p0 = AccessIJK(i0, j0, k0);
    if (p0 == GetMissingValue()) return (GetMissingValue());

    if (iwgt != 0.0) {
        p1 = AccessIJK(i1, j0, k0);
        if (p1 == GetMissingValue()) return (GetMissingValue());
    } else
        p1 = 0.0;

    if (jwgt != 0.0) {
        p2 = AccessIJK(i0, j1, k0);
        if (p2 == GetMissingValue()) return (GetMissingValue());
    } else
        p2 = 0.0;

    if (iwgt != 0.0 && jwgt != 0.0) {
        p3 = AccessIJK(i1, j1, k0);
        if (p3 == GetMissingValue()) return (GetMissingValue());
    } else
        p3 = 0.0;

    if (kwgt != 0.0) {
        p4 = AccessIJK(i0, j0, k1);
        if (p4 == GetMissingValue()) return (GetMissingValue());
    } else
        p4 = 0.0;

    if (kwgt != 0.0 && iwgt != 0.0) {
        p5 = AccessIJK(i1, j0, k1);
        if (p5 == GetMissingValue()) return (GetMissingValue());
    } else
        p5 = 0.0;

    if (kwgt != 0.0 && jwgt != 0.0) {
        p6 = AccessIJK(i0, j1, k1);
        if (p6 == GetMissingValue()) return (GetMissingValue());
    } else
        p6 = 0.0;

    if (kwgt != 0.0 && iwgt != 0.0 && jwgt != 0.0) {
        p7 = AccessIJK(i1, j1, k1);
        if (p7 == GetMissingValue()) return (GetMissingValue());
    } else
        p7 = 0.0;

    double c0 = p0 + iwgt * (p1 - p0) + jwgt * ((p2 + iwgt * (p3 - p2)) - (p0 + iwgt * (p1 - p0)));
    double c1 = p4 + iwgt * (p5 - p4) + jwgt * ((p6 + iwgt * (p7 - p6)) - (p4 + iwgt * (p5 - p4)));

    return (c0 + kwgt * (c1 - c0));
}

float LayeredGrid::GetValue(const std::vector<double> &coords) const
{
    // Clamp coordinates on periodic boundaries to grid extents
    //
    vector<double> clampedCoords = coords;
    ClampCoord(clampedCoords);

    if (!LayeredGrid::InsideGrid(clampedCoords)) return (GetMissingValue());

    const vector<size_t> &dims = GetDimensions();

    // Figure out interpolation order
    //
    int interp_order = _interpolationOrder;
    if (interp_order == 2) {
        if (dims[2] < 3) interp_order = 1;
    }

    if (interp_order == 0) {
        return (GetValueNearestNeighbor(clampedCoords));
    } else if (interp_order == 1) {
        return (GetValueLinear(clampedCoords));
    }

    return _getValueQuadratic(clampedCoords);
}

void LayeredGrid::SetInterpolationOrder(int order)
{
    if (order < 0 || order > 3) order = 2;
    _interpolationOrder = order;
}

void LayeredGrid::GetUserCoordinates(const size_t indices[], double coords[]) const
{
    size_t cIndices[3];
    ClampIndex(indices, cIndices);

    // First get coordinates of non-varying (horizontal) dimensions
    //
    vector<size_t> dims = GetDimensions();

    for (int i = 0; i < 2; i++) {
        size_t index = cIndices[i];

        if (index >= dims[i]) { index = dims[i] - 1; }

        coords[i] = cIndices[i] * _delta[i] + _minu[i];
    }

    // Now get coordinates of varying dimension
    //
    coords[2] = _rg.GetValueAtIndex(cIndices);
}

bool LayeredGrid::GetIndicesCell(const std::vector<double> &coords, std::vector<size_t> &indices) const
{
    indices.clear();

    std::vector<double> clampedCoords = coords;
    ClampCoord(clampedCoords);

    vector<size_t> dims = GetDimensions();

    // Get horizontal indices from regular grid
    //
    for (int i = 0; i < 2; i++) {
        indices.push_back(0);

        if (clampedCoords[i] < _minu[i] || clampedCoords[i] > _maxu[i]) { return (false); }

        if (_delta[i] != 0.0) {
            indices[i] = (size_t)floor((clampedCoords[i] - _minu[i]) / _delta[i]);

            // Edge case
            //
            if (indices[i] == dims[i] - 1) indices[i]--;
        }

        VAssert((indices[i] < dims[i] - 1) || (indices[i] == 0 && dims[i] == 1));
    }

    // Now find index for layered grid
    //
    size_t k;
    int    rc = _bsearchKIndexCell(indices[0], indices[1], clampedCoords[2], k);
    if (rc != 0) return (false);

    indices.push_back(k);

    return (true);
}

bool LayeredGrid::InsideGrid(const std::vector<double> &coords) const
{
    VAssert(coords.size() == 3);

    // Clamp coordinates on periodic boundaries to reside within the
    // grid extents (vary-dimensions can not have periodic boundaries)
    //
    vector<double> clampedCoords = coords;
    ClampCoord(clampedCoords);

    vector<size_t> indices;
    bool           found = GetIndicesCell(clampedCoords, indices);
    return (found);
}

LayeredGrid::ConstCoordItrLayered::ConstCoordItrLayered(const LayeredGrid *lg, bool begin) : ConstCoordItrAbstract()
{
    _dims = lg->GetDimensions();
    _minu = lg->_minu;
    _delta = lg->_delta;
    _coords = lg->_minu;

    _index = vector<size_t>(_dims.size(), 0);
    _zCoordItr = lg->_rg.cbegin();

    if (!begin) {
        _index[_dims.size() - 1] = _dims[_dims.size() - 1];
        _zCoordItr = lg->_rg.cend();
    }
}

LayeredGrid::ConstCoordItrLayered::ConstCoordItrLayered(const ConstCoordItrLayered &rhs) : ConstCoordItrAbstract()
{
    _index = rhs._index;
    _dims = rhs._dims;
    _minu = rhs._minu;
    _delta = rhs._delta;
    _coords = rhs._coords;
    _zCoordItr = rhs._zCoordItr;
}

LayeredGrid::ConstCoordItrLayered::ConstCoordItrLayered() : ConstCoordItrAbstract()
{
    _index.clear();
    _dims.clear();
    _minu.clear();
    _delta.clear();
    _coords.clear();
}

void LayeredGrid::ConstCoordItrLayered::next()
{
    _index[0]++;
    ++_zCoordItr;
    _coords[0] += _delta[0];
    if (_index[0] < _dims[0]) {
        _coords[2] = *_zCoordItr;
        return;
    }

    _index[0] = 0;
    _coords[0] = _minu[0];
    _index[1]++;
    _coords[1] += _delta[1];

    if (_index[1] < _dims[1]) {
        _coords[2] = *_zCoordItr;
        return;
    }

    _index[1] = 0;
    _coords[1] = _minu[1];
    _index[2]++;
    if (_index[2] < _dims[2]) {
        _coords[2] = *_zCoordItr;
        return;
    }
}

void LayeredGrid::ConstCoordItrLayered::next(const long &offset)
{
    if (!_index.size()) return;

    vector<size_t> maxIndex;
    ;
    for (int i = 0; i < _dims.size(); i++) maxIndex.push_back(_dims[i] - 1);

    long maxIndexL = Wasp::LinearizeCoords(maxIndex, _dims);
    long newIndexL = Wasp::LinearizeCoords(_index, _dims) + offset;
    if (newIndexL < 0) { newIndexL = 0; }
    if (newIndexL > maxIndexL) {
        _index = vector<size_t>(_dims.size(), 0);
        _index[_dims.size() - 1] = _dims[_dims.size() - 1];
        return;
    }

    _index = Wasp::VectorizeCoords(newIndexL, _dims);
    _zCoordItr += offset;

    _coords[0] = _index[0] * _delta[0] + _minu[0];
    _coords[1] = _index[1] * _delta[1] + _minu[1];
    _coords[2] = *_zCoordItr;
}

void LayeredGrid::_getBilinearWeights(const vector<double> &coords, double &iwgt, double &jwgt) const
{
    vector<size_t> dims = GetDimensions();

    vector<size_t> indices0;
    bool           found = GetIndicesCell(coords, indices0);
    VAssert(found);

    vector<size_t> indices1 = indices0;
    if (indices0[0] != dims[0] - 1) { indices1[0] += 1; }
    if (indices0[1] != dims[1] - 1) { indices1[1] += 1; }

    vector<double> coords0, coords1;
    Grid::GetUserCoordinates(indices0, coords0);
    Grid::GetUserCoordinates(indices1, coords1);
    double x = coords[0];
    double y = coords[1];
    double x0 = coords0[0];
    double y0 = coords0[1];
    double x1 = coords1[0];
    double y1 = coords1[1];

    if (x1 != x0)
        iwgt = fabs((x - x0) / (x1 - x0));
    else
        iwgt = 0.0;
    if (y1 != y0)
        jwgt = fabs((y - y0) / (y1 - y0));
    else
        jwgt = 0.0;
}

double LayeredGrid::_bilinearInterpolation(size_t i0, size_t i1, size_t j0, size_t j1, size_t k0, double iwgt, double jwgt) const
{
    double val00, val01, val10, val11, xVal0, result;
    double xVal1 = 0.0;
    double mv = GetMissingValue();

    val00 = AccessIJK(i0, j0, k0);
    val10 = AccessIJK(i1, j0, k0);
    if ((val00 == mv) || (val10 == mv)) return mv;
    if (val00 == mv)
        xVal0 = val10;
    else if (val10 == mv)
        xVal0 = val00;
    else
        xVal0 = val00 * (1 - iwgt) + val10 * iwgt;

    val01 = AccessIJK(i0, j1, k0);
    val11 = AccessIJK(i1, j1, k0);
    if ((val01 == mv) || (val11 == mv)) return mv;
    if (val01 == mv)
        xVal0 = val11;
    else if (val11 == mv)
        xVal0 = val01;
    else
        xVal1 = val01 * (1 - iwgt) + val11 * iwgt;

    result = xVal0 * (1 - jwgt) + xVal1 * jwgt;

    if ((val00 == mv) || (val01 == mv) || (val10 == mv) || (val11 == mv))
        return mv;
    else
        return result;
}

double LayeredGrid::_bilinearElevation(size_t i0, size_t i1, size_t j0, size_t j1, size_t k0, double iwgt, double jwgt) const
{
    double xVal0, result;
    double xVal1 = 0.0;
    double x, y, z00, z10, z01, z11;

    GetUserCoordinates(i0, j0, k0, x, y, z00);
    GetUserCoordinates(i1, j0, k0, x, y, z10);
    xVal0 = z00 * (1 - iwgt) + z10 * iwgt;

    GetUserCoordinates(i0, j1, k0, x, y, z01);
    GetUserCoordinates(i1, j1, k0, x, y, z11);
    xVal1 = z01 * (1 - iwgt) + z11 * iwgt;

    result = xVal0 * (1 - jwgt) + xVal1 * jwgt;
    return result;
}

float LayeredGrid::_getValueQuadratic(const std::vector<double> &coords) const
{
    double         mv = GetMissingValue();
    vector<size_t> dims = GetDimensions();

    // Get the indecies of the hyperslab containing the point
    // k0 = level above the point
    // k1 = level below the point
    // k2 = two levels below the point
    //
    vector<size_t> indices;
    bool           found = GetIndicesCell(coords, indices);
    if (!found) return (GetMissingValue());

    size_t i0 = indices[0];
    size_t j0 = indices[1];
    size_t k1 = indices[2];

    size_t k0, k2;
    size_t i1, j1;

    if (i0 == dims[0] - 1)
        i1 = i0;
    else
        i1 = i0 + 1;
    if (j0 == dims[1] - 1)
        j1 = j0;
    else
        j1 = j0 + 1;
    if (k1 == 0) {
        k2 = 0;
        k1 = 1;
        k0 = 2;
    } else if (k1 == dims[2] - 1) {
        k2 = dims[2] - 3;
        k1 = dims[2] - 2;
        k0 = dims[2] - 1;
    } else {
        k0 = k1 + 1;
        k2 = k1 - 1;
    }

    double iwgt, jwgt;
    _getBilinearWeights(coords, iwgt, jwgt);

    // bilinearly interpolated values at each k0, k1 and k2
    double val0, val1, val2;
    val0 = _bilinearInterpolation(i0, i1, j0, j1, k0, iwgt, jwgt);
    val1 = _bilinearInterpolation(i0, i1, j0, j1, k1, iwgt, jwgt);
    val2 = _bilinearInterpolation(i0, i1, j0, j1, k2, iwgt, jwgt);
    if ((val0 == mv) || (val1 == mv) || (val2 == mv)) return mv;

    // bilinearly interpolated elevations at each k0, k1, and k2
    double z0, z1, z2;
    z0 = _bilinearElevation(i0, i1, j0, j1, k0, iwgt, jwgt);
    z1 = _bilinearElevation(i0, i1, j0, j1, k1, iwgt, jwgt);
    z2 = _bilinearElevation(i0, i1, j0, j1, k2, iwgt, jwgt);
    if ((z0 == mv) || (z1 == mv) || (z2 == mv)) return mv;

    // quadratic interpolation weights
    double z = coords[2];
    double w0, w1, w2;
    w0 = ((z - z1) * (z - z2)) / ((z0 - z1) * (z0 - z2));
    w1 = ((z - z0) * (z - z2)) / ((z1 - z0) * (z1 - z2));
    w2 = ((z - z0) * (z - z1)) / ((z2 - z0) * (z2 - z1));

    double val;
    val = val0 * w0 + val1 * w1 + val2 * w2;

    return val;
}

double LayeredGrid::_interpolateVaryingCoord(size_t i0, size_t j0, size_t k0, double x, double y) const
{
    // varying dimension coord at corner grid points of cell face
    //
    double c00, c01, c10, c11;

    vector<size_t> dims = GetDimensions();

    size_t i1, j1, k1;
    if (i0 == dims[0] - 1)
        i1 = i0;
    else
        i1 = i0 + 1;
    if (j0 == dims[1] - 1)
        j1 = j0;
    else
        j1 = j0 + 1;
    if (k0 == dims[2] - 1)
        k1 = k0;
    else
        k1 = k0 + 1;

    // Coordinates of grid points for non-varying dimensions
    double x0, y0, z0, x1, y1, z1;
    GetUserCoordinates(i0, j0, k0, x0, y0, z0);
    GetUserCoordinates(i1, j1, k1, x1, y1, z1);
    double iwgt, jwgt;

    c00 = _rg.AccessIJK(i0, j0, k0);
    c01 = _rg.AccessIJK(i1, j0, k0);
    c10 = _rg.AccessIJK(i0, j1, k0);
    c11 = _rg.AccessIJK(i1, j1, k0);

    if (x1 != x0)
        iwgt = fabs((x - x0) / (x1 - x0));
    else
        iwgt = 0.0;
    if (y1 != y0)
        jwgt = fabs((y - y0) / (y1 - y0));
    else
        jwgt = 0.0;

    double z = c00 + iwgt * (c01 - c00) + jwgt * ((c10 + iwgt * (c11 - c10)) - (c00 + iwgt * (c01 - c00)));
    return (z);
}

namespace {
double pointDotPlane(const vector<double> &v1, const vector<double> &v2, const vector<double> &v3, const vector<double> &p)
{
    double vec1_2[3] = {v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]};
    double vec1_3[3] = {v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2]};
    double vp[3] = {p[0] - v1[0], p[1] - v1[1], p[2] - v1[2]};
    double normal[3];

    vcross(vec1_2, vec1_3, normal);

    return (vdot(normal, vp));
}
};    // namespace

int LayeredGrid::_bsearchKIndexCell(size_t i, size_t j, double z, size_t &k) const
{
    k = 0;

    vector<size_t> dims = GetDimensions();

    // Binary search for starting index of cell containing z
    //

    // Indices of bottom level triangle inside of column containing point
    //
    vector<size_t> v1idx = {i, j, 0};
    vector<size_t> v2idx = {i + 1, j, 0};
    vector<size_t> v3idx = {i, j + 1, 0};

    // Coordinates of bottom level triangle inside column containing point
    // The horizontal coordinates (X & Y) don't change for the search
    //
    vector<double> v1;
    vector<double> v2;
    vector<double> v3;
    vector<double> pt;

    Grid::GetUserCoordinates(v1idx, v1);
    Grid::GetUserCoordinates(v2idx, v2);
    Grid::GetUserCoordinates(v3idx, v3);

    // Coordinates of point we're looking for. X & Y are meaningless. Just
    // use first triangle vertex.
    //
    pt = v1;
    pt[2] = z;

    size_t k0 = 0;
    size_t k1 = dims[2] - 1;

    // See if point is below or above the column
    //
    v1[2] = _rg.AccessIJK(i, j, k0);
    v2[2] = _rg.AccessIJK(i + 1, j, k0);
    v3[2] = _rg.AccessIJK(i, j + 1, k0);
    double d = pointDotPlane(v1, v2, v3, pt);
    if (d < 0.0) return (-1);

    v1[2] = _rg.AccessIJK(i, j, k1);
    v2[2] = _rg.AccessIJK(i + 1, j, k1);
    v3[2] = _rg.AccessIJK(i, j + 1, k1);
    d = pointDotPlane(v1, v2, v3, pt);
    if (d > 0.0) return (1);

    // point is inside column. Now find it
    //
    while (k1 - k0 > 1) {
        // Update Z coordinate only for search
        //
        v1[2] = _rg.AccessIJK(i, j, (k0 + k1) >> 1);
        v2[2] = _rg.AccessIJK(i + 1, j, (k0 + k1) >> 1);
        v3[2] = _rg.AccessIJK(i, j + 1, (k0 + k1) >> 1);

        // d is signed distance from plane. Sign determines if it is
        // above triangle (positive) or below (negative)
        //
        double d = pointDotPlane(v1, v2, v3, pt);

        // Pathlogical case. Point is on triangle.
        //
        if (d == 0.0) {
            k0 = (k0 + k1) >> 1;
            break;
        }

        if (d < 0.0) {
            k1 = (k0 + k1) >> 1;
        } else {
            k0 = (k0 + k1) >> 1;
        }
    }
    k = k0;

    return (0);
}
