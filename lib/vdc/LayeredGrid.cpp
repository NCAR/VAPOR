#include <stdio.h>
#include <iostream>
#include <cmath>
#include <cfloat>
#include <vapor/vizutil.h>
#include "vapor/utils.h"
#include "vapor/LayeredGrid.h"
#define INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
#include "vapor/LegacyVectorMath.h"
#include "vapor/VAssert.h"

using namespace std;
using namespace VAPoR;

LayeredGrid::LayeredGrid(const vector<size_t> &dims, const vector<size_t> &bs, const vector<float *> &blks, const std::vector<double> &xcoords, const std::vector<double> &ycoords,
                         const RegularGrid &zrg)
: StructuredGrid(dims, bs, blks), _sg2d(vector<size_t>(dims.begin(), dims.begin() + 2), vector<size_t>(bs.begin(), bs.begin() + 2), vector<float *>(), xcoords, ycoords, vector<double>()), _zrg(zrg),
  _xcoords(xcoords), _ycoords(ycoords)
{
    VAssert(GetDimensions().size() == 3);
    VAssert(xcoords.size() == GetDimensions()[0]);
    VAssert(ycoords.size() == GetDimensions()[1]);
    VAssert(zrg.GetDimensions()[0] == xcoords.size());
    VAssert(zrg.GetDimensions()[1] == ycoords.size());

    _interpolationOrder = 1;

    // Set horizontal extents from sg2d
    //
    _sg2d.GetUserExtents(_minu, _maxu);

    // Get extents of layered dimension
    //
    float range[2];
    _zrg.GetRange(range);
    _minu[2] = (double)range[0];
    _maxu[2] = (double)range[1];
}

vector<size_t> LayeredGrid::GetCoordDimensions(size_t dim) const
{
    if (dim == 0) {
        return (vector<size_t>(1, GetDimensions()[0]));
    } else if (dim == 1) {
        return (vector<size_t>(1, GetDimensions()[1]));
    } else if (dim == 2) {
        auto tmp = _zrg.GetDimensions();
        auto tmp2 = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
        tmp2.resize(_zrg.GetNumDimensions());
        return tmp2;
    } else {
        return (vector<size_t>(1, 1));
    }
}

void LayeredGrid::GetUserExtentsHelper(DblArr3 &minu, DblArr3 &maxu) const
{
    minu = _minu;
    maxu = _maxu;
}

void LayeredGrid::GetBoundingBox(const Size_tArr3 &min, const Size_tArr3 &max, DblArr3 &minu, DblArr3 &maxu) const
{
    Size_tArr3 cMin;
    ClampIndex(min, cMin);

    Size_tArr3 cMax;
    ClampIndex(max, cMax);

    // Get extents of horizontal dimensions. Note: also get vertical
    // dimension, but it's bogus for layered grid.
    //
    GetUserCoordinates(cMin, minu);
    GetUserCoordinates(cMax, maxu);

    // Initialize min and max coordinates of varying dimension with
    // coordinates of "first" and "last" grid point. Coordinates of
    // varying dimension are stored as values of a scalar function
    // sampling the coordinate space.
    //
    float mincoord = _zrg.GetValueAtIndex(cMin);
    float maxcoord = _zrg.GetValueAtIndex(cMax);

    // Now find the extreme values of the varying dimension's coordinates
    //
    for (int j = cMin[1]; j <= cMax[1]; j++) {
        for (int i = cMin[0]; i <= cMax[0]; i++) {
            float v = _zrg.AccessIJK(i, j, cMin[2]);

            if (v < mincoord) mincoord = v;
        }
    }

    for (int j = cMin[1]; j <= cMax[1]; j++) {
        for (int i = cMin[0]; i <= cMax[0]; i++) {
            float v = _zrg.AccessIJK(i, j, cMax[2]);

            if (v > maxcoord) maxcoord = v;
        }
    }

    minu[2] = mincoord;
    maxu[2] = maxcoord;
}

bool LayeredGrid::_insideGrid(const DblArr3 &coords, Size_tArr3 &indices, double wgts[3]) const
{
    // Get indices and weights for horizontal slice
    //
    bool found = _sg2d.GetIndicesCell(coords, indices, wgts);
    if (!found) return (found);

    // XZ and YZ cell sides are planar, but XY sides may not be. We divide
    // the XY faces into two triangles (changing hexahedrals into prims)
    // and figure out which triangle (prism) the point is in (first or
    // second). Then we search the stack of first (or second) prism in Z
    //
    //

    // Check if point is in "first" triangle (0,0), (1,0), (1,1)
    //
    double     lambda[3];
    double     pt[] = {coords[0], coords[1]};
    Size_tArr3 iv = {indices[0], indices[0] + 1, indices[0] + 1};
    Size_tArr3 jv = {indices[1], indices[1], indices[1] + 1};
    double     tverts[] = {_xcoords[iv[0]], _ycoords[jv[0]], _xcoords[iv[1]], _ycoords[jv[1]], _xcoords[iv[2]], _ycoords[jv[2]]};

    bool inside = VAPoR::BarycentricCoordsTri(tverts, pt, lambda);
    if (!inside) {
        // Not in first triangle.
        // Now check if point is in "second" triangle (0,0), (1,1), (0,1)
        //
        iv = {indices[0], indices[0] + 1, indices[0]};
        jv = {indices[1], indices[1] + 1, indices[1] + 1};
        double tverts[] = {_xcoords[iv[0]], _ycoords[jv[0]], _xcoords[iv[1]], _ycoords[jv[1]], _xcoords[iv[2]], _ycoords[jv[2]]};

        inside = VAPoR::BarycentricCoordsTri(tverts, pt, lambda);

        // Mathematically this shouldn't happen if _sg2d.GetIndicesCell()
        // returns true, but have to contend with floating point roundoff
        //
        if (!inside) return (false);
    }

    float z0, z1;

    // Find k index of cell containing z. Already know i and j indices
    //
    vector<double> zcoords;

    size_t nz = GetDimensions()[2];
    zcoords.reserve(nz);
    for (int kk = 0; kk < nz; kk++) {
        // Interpolate Z coordinate across triangle
        //
        float zk = _zrg.AccessIJK(iv[0], jv[0], kk) * lambda[0] + _zrg.AccessIJK(iv[1], jv[1], kk) * lambda[1] + _zrg.AccessIJK(iv[2], jv[2], kk) * lambda[2];

        zcoords.push_back(zk);
    }

    if (!Wasp::BinarySearchRange(zcoords, coords[2], indices[2])) return (false);

    z0 = zcoords[indices[2]];
    z1 = indices[2] < nz - 1 ? zcoords[indices[2] + 1] : z0;

    wgts[2] = 1.0 - (coords[2] - z0) / (z1 - z0);

    return (true);
}

float LayeredGrid::GetValueNearestNeighbor(const DblArr3 &coords) const
{
    Size_tArr3 indices;
    double     wgts[3];
    bool       found = _insideGrid(coords, indices, wgts);
    if (!found) return (GetMissingValue());

    if (wgts[0] < 0.5) indices[0] += 1;
    if (wgts[1] < 0.5) indices[1] += 1;
    if (wgts[2] < 0.5) indices[2] += 1;

    return (AccessIJK(indices[0], indices[1], indices[2]));
}

float LayeredGrid::GetValueLinear(const DblArr3 &coords) const
{
    Size_tArr3 indices;
    double     wgts[3];
    bool       found = _insideGrid(coords, indices, wgts);
    if (!found) return (GetMissingValue());

    size_t i0 = indices[0];
    size_t j0 = indices[1];
    size_t k0 = indices[2];
    size_t i1 = indices[0] + 1;
    size_t j1 = indices[1] + 1;
    size_t k1 = indices[2] + 1;

    //
    // perform tri-linear interpolation
    //
    double p0, p1, p2, p3, p4, p5, p6, p7;
    double iwgt = 1.0 - wgts[0];    // Oops. Weights reversed.
    double jwgt = 1.0 - wgts[1];
    double kwgt = 1.0 - wgts[2];

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

float LayeredGrid::GetValue(const DblArr3 &coords) const
{
    // Clamp coordinates on periodic boundaries to grid extents
    //
    DblArr3 cCoords;
    ClampCoord(coords, cCoords);

    auto dims = GetDimensions();

    // Figure out interpolation order
    //
    int interp_order = _interpolationOrder;
    if (interp_order == 2) {
        if (dims[2] < 3) interp_order = 1;
    }

    if (interp_order == 0) {
        return (GetValueNearestNeighbor(cCoords));
    } else if (interp_order == 1) {
        return (GetValueLinear(cCoords));
    }

    return _getValueQuadratic(cCoords.data());
}

void LayeredGrid::SetInterpolationOrder(int order)
{
    if (order < 0 || order > 3) order = 2;
    _interpolationOrder = order;
}

void LayeredGrid::GetUserCoordinates(const Size_tArr3 &indices, DblArr3 &coords) const
{
    Size_tArr3 cIndices;
    ClampIndex(indices, cIndices);

    // First get coordinates of (horizontal) dimensions
    //
    _sg2d.GetUserCoordinates(indices, coords);

    // Now get coordinates of z dimension
    //
    coords[2] = _zrg.GetValueAtIndex(cIndices);
}

bool LayeredGrid::GetIndicesCell(const DblArr3 &coords, Size_tArr3 &indices) const
{
    DblArr3 cCoords;
    ClampCoord(coords, cCoords);

    double dummy[3];
    return (_insideGrid(coords, indices, dummy));

    return (true);
}

bool LayeredGrid::InsideGrid(const DblArr3 &coords) const
{
    // Clamp coordinates on periodic boundaries to reside within the
    // grid extents (vary-dimensions can not have periodic boundaries)
    //
    DblArr3 cCoords;
    ClampCoord(coords, cCoords);

    Size_tArr3 indices;
    bool       found = GetIndicesCell(cCoords, indices);
    return (found);
}

LayeredGrid::ConstCoordItrLayered::ConstCoordItrLayered(const LayeredGrid *lg, bool begin) : ConstCoordItrAbstract()
{
    _lg = lg;
    _nElements2D = lg->GetDimensions()[0] * lg->GetDimensions()[1];
    _coords = vector<double>(3, 0.0);

    if (begin) {
        _index2D = 0;
        _zCoordItr = lg->_zrg.cbegin();
        _itr2D = lg->_sg2d.ConstCoordBegin();
    } else {
        _index2D = _nElements2D - 1;
        _zCoordItr = lg->_zrg.cend();
        _itr2D = lg->_sg2d.ConstCoordEnd();
    }
}

LayeredGrid::ConstCoordItrLayered::ConstCoordItrLayered(const ConstCoordItrLayered &rhs) : ConstCoordItrAbstract()
{
    _lg = rhs._lg;
    _nElements2D = rhs._nElements2D;
    _coords = rhs._coords;
    _index2D = rhs._index2D;
    _zCoordItr = rhs._zCoordItr;
    _itr2D = rhs._itr2D;
}

LayeredGrid::ConstCoordItrLayered::ConstCoordItrLayered() : ConstCoordItrAbstract() { _coords.clear(); }

void LayeredGrid::ConstCoordItrLayered::next()
{
    ++_index2D;
    ++_itr2D;
    ++_zCoordItr;

    // Check for overflow
    //
    if (_index2D == _nElements2D) {
        _itr2D = _lg->_sg2d.ConstCoordBegin();
        _index2D = 0;
    }

    _coords[0] = (*_itr2D)[0];
    _coords[1] = (*_itr2D)[1];

    _coords[2] = *_zCoordItr;
}

void LayeredGrid::ConstCoordItrLayered::next(const long &offset)
{
    long offset2D = offset % _nElements2D;

    if (offset2D + _index2D < _nElements2D) {
        _itr2D += offset;
        _index2D += offset;
    } else {
        size_t o = (offset2D + _index2D) % _nElements2D;
        _itr2D = _lg->_sg2d.ConstCoordBegin() + o;
        _index2D = o;
    }

    _coords[0] = (*_itr2D)[0];
    _coords[1] = (*_itr2D)[1];

    _zCoordItr += offset;
    _coords[2] = *_zCoordItr;
}

void LayeredGrid::_getBilinearWeights(const double coords[3], double &iwgt, double &jwgt) const
{
    auto dims = GetDimensions();

    size_t indices0[3];
    bool   found = GetIndicesCell(coords, indices0);
    VAssert(found);

    size_t indices1[3] = {indices0[0], indices0[1], indices0[2]};
    if (indices0[0] != dims[0] - 1) { indices1[0] += 1; }
    if (indices0[1] != dims[1] - 1) { indices1[1] += 1; }

    double coords0[3], coords1[3];
    GetUserCoordinates(indices0, coords0);
    GetUserCoordinates(indices1, coords1);
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

float LayeredGrid::_getValueQuadratic(const double coords[3]) const
{
    double mv = GetMissingValue();
    auto   dims = GetDimensions();

    // Get the indecies of the hyperslab containing the point
    // k0 = level above the point
    // k1 = level below the point
    // k2 = two levels below the point
    //
    size_t indices[3];
    bool   found = GetIndicesCell(coords, indices);
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

    auto dims = GetDimensions();

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

    c00 = _zrg.AccessIJK(i0, j0, k0);
    c01 = _zrg.AccessIJK(i1, j0, k0);
    c10 = _zrg.AccessIJK(i0, j1, k0);
    c11 = _zrg.AccessIJK(i1, j1, k0);

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
