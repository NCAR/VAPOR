#include <stdio.h>
#include <iostream>
#include <cassert>
#include <cmath>
#include <cfloat>
#include "vapor/LayeredGrid.h"
#include "vapor/glutil.h"

using namespace std;
using namespace VAPoR;

void LayeredGrid::_layeredGrid(
    const vector<double> &minu,
    const vector<double> &maxu,
    const RegularGrid &rg) {
    assert(GetTopologyDim() == 3);
    assert(minu.size() == maxu.size());
    assert(minu.size() == 2);

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
        _delta.push_back((_maxu[i] - _minu[i]) / (double)(dims[i] - 1));
    }

    // Get extents of layered dimension
    //
    float range[2];
    _rg.GetRange(range);
    _minu.push_back(range[0]);
    _maxu.push_back(range[1]);
}

LayeredGrid::LayeredGrid(
    const vector<size_t> &dims,
    const vector<size_t> &bs,
    const vector<float *> &blks,
    const vector<double> &minu,
    const vector<double> &maxu,
    const RegularGrid &rg) : StructuredGrid(dims, bs, blks) {

    _layeredGrid(minu, maxu, rg);
}

LayeredGrid::~LayeredGrid() {
}

void LayeredGrid::GetUserExtents(
    vector<double> &minu, vector<double> &maxu) const {
    minu = _minu;
    maxu = _maxu;
}

void LayeredGrid::GetBoundingBox(
    const vector<size_t> &min,
    const vector<size_t> &max,
    vector<double> &minu,
    vector<double> &maxu) const {
    assert(min.size() == max.size());
    assert(min.size() >= 2);

    minu.clear();
    maxu.clear();

    // Get extents of horizontal dimensions. Note: also get vertical
    // dimension, but it's bogus for layered grid.
    //
    GetUserCoordinates(min, minu);
    GetUserCoordinates(max, maxu);

    // Initialize min and max coordinates of varying dimension with
    // coordinates of "first" and "last" grid point. Coordinates of
    // varying dimension are stored as values of a scalar function
    // sampling the coordinate space.
    //
    float mincoord = _rg.AccessIndex(min);
    float maxcoord = _rg.AccessIndex(max);

    // Now find the extreme values of the varying dimension's coordinates
    //
    for (int j = min[1]; j <= max[1]; j++) {
        for (int i = min[0]; i <= max[0]; i++) {
            float v = _rg.AccessIJK(i, j, min[2]);

            if (v < mincoord)
                mincoord = v;
        }
    }

    for (int j = min[1]; j <= max[1]; j++) {
        for (int i = min[0]; i <= max[0]; i++) {
            float v = _rg.AccessIJK(i, j, max[2]);

            if (v > maxcoord)
                maxcoord = v;
        }
    }

    minu[2] = mincoord;
    maxu[2] = maxcoord;
}

void LayeredGrid::GetEnclosingRegion(
    const vector<double> &minu, const vector<double> &maxu,
    vector<size_t> &min, vector<size_t> &max) const {
    assert(minu.size() == maxu.size());
    assert(minu.size() == 3);

    min.clear();
    max.clear();

    //
    // Get coords for non-varying dimension AND varying dimension.
    //
    for (int i = 0; i < 2; i++) {
        assert(minu[i] <= maxu[i]);
        double u = minu[i];
        if (u < minu[i]) {
            u = minu[i];
        }
        size_t index = (u - _minu[i]) / _delta[i];
        min.push_back(index);

        u = maxu[i];
        if (u > maxu[i]) {
            u = maxu[i];
        }
        index = (u - _maxu[i]) / _delta[i];
        max.push_back(index);
    }

    // we have the correct results
    // for X and Y dimensions, Now need to do the varying axis
    //

    min.push_back(0.0);
    max.push_back(0.0);
    vector<size_t> dims = GetDimensions();

    bool done;
    double z;
    //
    // first do max, then min
    //
    done = false;
    for (int k = 0; k < dims[2] && !done; k++) {
        done = true;
        max[2] = k;
        for (int j = min[1]; j <= max[1] && done; j++) {
            for (int i = min[0]; i <= max[0] && done; i++) {
                z = _rg.AccessIJK(i, j, k); // get Z coordinate
                if (z < maxu[2]) {
                    done = false;
                }
            }
        }
    }
    done = false;
    for (int k = dims[2] - 1; k >= 0 && !done; k--) {
        done = true;
        min[2] = k;
        for (int j = min[1]; j <= max[1] && done; j++) {
            for (int i = min[0]; i <= max[0] && done; i++) {
                z = _rg.AccessIJK(i, j, k); // get Z coordinate
                if (z > minu[2]) {
                    done = false;
                }
            }
        }
    }
}

float LayeredGrid::_GetValueNearestNeighbor(
    const std::vector<double> &coords) const {

    // Get the indecies of the nearest grid point
    //
    vector<size_t> indices;
    GetIndices(coords, indices);

    return (AccessIndex(indices));
}

float LayeredGrid::_GetValueLinear(
    const std::vector<double> &coords) const {
    assert(coords.size() == 3);

    vector<size_t> dims = GetDimensions();

    // Get the indecies of the cell containing the point
    //
    vector<size_t> indices0;
    bool found = GetIndicesCell(coords, indices0);
    if (!found)
        return (GetMissingValue());

    vector<size_t> indices1 = indices0;

    indices1[0] += 1;
    indices1[1] += 1;
    indices1[2] += 1;

    // Get user coordinates of cell containing point
    //
    vector<double> coords0, coords1;
    GetUserCoordinates(indices0, coords0);
    GetUserCoordinates(indices1, coords1);

    size_t i0 = indices0[0];
    size_t j0 = indices0[1];
    size_t k0 = indices0[2];
    size_t i1 = indices1[0];
    size_t j1 = indices1[1];
    size_t k1 = indices1[2];

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
    double iwgt, jwgt, kwgt;
    z0 = _interpolateVaryingCoord(i0, j0, k0, x, y);
    z1 = _interpolateVaryingCoord(i0, j0, k1, x, y);

    if (x1 != x0)
        iwgt = fabs((x - x0) / (x1 - x0));
    else
        iwgt = 0.0;
    if (y1 != y0)
        jwgt = fabs((y - y0) / (y1 - y0));
    else
        jwgt = 0.0;
    if (z1 != z0)
        kwgt = fabs((z - z0) / (z1 - z0));
    else
        kwgt = 0.0;

    //
    // perform tri-linear interpolation
    //
    double p0, p1, p2, p3, p4, p5, p6, p7;

    p0 = AccessIJK(i0, j0, k0);
    if (p0 == GetMissingValue())
        return (GetMissingValue());

    if (iwgt != 0.0) {
        p1 = AccessIJK(i1, j0, k0);
        if (p1 == GetMissingValue())
            return (GetMissingValue());
    } else
        p1 = 0.0;

    if (jwgt != 0.0) {
        p2 = AccessIJK(i0, j1, k0);
        if (p2 == GetMissingValue())
            return (GetMissingValue());
    } else
        p2 = 0.0;

    if (iwgt != 0.0 && jwgt != 0.0) {
        p3 = AccessIJK(i1, j1, k0);
        if (p3 == GetMissingValue())
            return (GetMissingValue());
    } else
        p3 = 0.0;

    if (kwgt != 0.0) {
        p4 = AccessIJK(i0, j0, k1);
        if (p4 == GetMissingValue())
            return (GetMissingValue());
    } else
        p4 = 0.0;

    if (kwgt != 0.0 && iwgt != 0.0) {
        p5 = AccessIJK(i1, j0, k1);
        if (p5 == GetMissingValue())
            return (GetMissingValue());
    } else
        p5 = 0.0;

    if (kwgt != 0.0 && jwgt != 0.0) {
        p6 = AccessIJK(i0, j1, k1);
        if (p6 == GetMissingValue())
            return (GetMissingValue());
    } else
        p6 = 0.0;

    if (kwgt != 0.0 && iwgt != 0.0 && jwgt != 0.0) {
        p7 = AccessIJK(i1, j1, k1);
        if (p7 == GetMissingValue())
            return (GetMissingValue());
    } else
        p7 = 0.0;

    double c0 = p0 + iwgt * (p1 - p0) + jwgt * ((p2 + iwgt * (p3 - p2)) - (p0 + iwgt * (p1 - p0)));
    double c1 = p4 + iwgt * (p5 - p4) + jwgt * ((p6 + iwgt * (p7 - p6)) - (p4 + iwgt * (p5 - p4)));

    return (c0 + kwgt * (c1 - c0));
}

float LayeredGrid::GetValue(const std::vector<double> &coords) const {

    // Clamp coordinates on periodic boundaries to grid extents
    //
    vector<double> clampedCoords = coords;
    ClampCoord(clampedCoords);

    if (!LayeredGrid::InsideGrid(clampedCoords))
        return (GetMissingValue());

    vector<size_t> dims = GetDimensions();

    // Figure out interpolation order
    //
    int interp_order = _interpolationOrder;
    if (interp_order == 2) {
        if (dims[2] < 3)
            interp_order = 1;
    }

    if (interp_order == 0) {
        return (_GetValueNearestNeighbor(clampedCoords));
    } else if (interp_order == 1) {
        return (_GetValueLinear(clampedCoords));
    }

    return _getValueQuadratic(clampedCoords);
}

void LayeredGrid::SetInterpolationOrder(int order) {
    if (order < 0 || order > 3)
        order = 2;
    _interpolationOrder = order;
}

void LayeredGrid::GetUserCoordinates(
    const std::vector<size_t> &indices,
    std::vector<double> &coords) const {
    assert(indices.size() == 3);
    coords.clear();

    // First get coordinates of non-varying (horizontal) dimensions
    //
    vector<size_t> dims = GetDimensions();
    assert(indices.size() == GetTopologyDim());

    for (int i = 0; i < 2; i++) {
        size_t index = indices[i];

        if (index >= dims[i]) {
            index = dims[i] - 1;
        }

        coords.push_back(indices[i] * _delta[i] + _minu[i]);
    }

    // Now get coordinates of varying dimension
    //
    float v = _rg.AccessIndex(indices);
    coords.push_back(v);
}

void LayeredGrid::GetIndices(
    const std::vector<double> &coords,
    std::vector<size_t> &indices) const {

    // First get ij index of non-varying dimensions
    //
    assert(coords.size() >= GetTopologyDim());
    indices.clear();

    std::vector<double> clampedCoords = coords;
    ClampCoord(clampedCoords);

    vector<size_t> dims = GetDimensions();

    vector<double> wgts;

    // Get the two horizontal offsets
    //
    for (int i = 0; i < 2; i++) {
        indices.push_back(0);

        if (clampedCoords[i] < _minu[i]) {
            indices[i] = 0;
            continue;
        }
        if (clampedCoords[i] > _maxu[i]) {
            indices[i] = dims[i] - 1;
            continue;
        }

        if (_delta[i] != 0.0) {
            indices[i] = (size_t)floor(
                (clampedCoords[i] - _minu[i]) / _delta[i]);
        }

        assert(indices[i] < dims[i]);

        double wgt = 0.0;

        if (_delta[0] != 0.0) {
            wgt = ((clampedCoords[i] - _minu[i]) - (indices[i] * _delta[i])) /
                  _delta[i];
        }
        if (wgt > 0.5)
            indices[i] += 1;
    }

    // At this point the ij indecies are correct for the non-varying
    // dimensions. We only need to find the index for the varying dimension
    //
    size_t k0;
    int rc = _bsearchKIndexCell(
        indices[0], indices[1], clampedCoords[2], k0);

    // _bsearchKIndexCell returns non-zero value if point is outside of the
    // vertical column  (negative number if below, positive if above);
    //
    if (rc != 0) {
        if (rc < 0) {
            indices.push_back(0);
            return;
        } else {
            indices.push_back(dims[2] - 1);
            return;
        }
    }

    double z0 = _interpolateVaryingCoord(
        indices[0], indices[1], k0, clampedCoords[0], clampedCoords[1]);
    double z1 = _interpolateVaryingCoord(
        indices[0], indices[1], k0 + 1, clampedCoords[0], clampedCoords[1]);
    if (fabs(clampedCoords[2] - z0) < fabs(clampedCoords[2] - z1)) {
        indices.push_back(k0);
    } else {
        indices.push_back(k0 + 1);
    }
}

bool LayeredGrid::GetIndicesCell(
    const std::vector<double> &coords,
    std::vector<size_t> &indices) const {

    assert(coords.size() >= GetTopologyDim());
    indices.clear();

    std::vector<double> clampedCoords = coords;
    ClampCoord(clampedCoords);

    vector<size_t> dims = GetDimensions();

    // Get horizontal indices from regular grid
    //
    for (int i = 0; i < 2; i++) {
        indices.push_back(0);

        if (clampedCoords[i] < _minu[i] || clampedCoords[i] > _maxu[i]) {
            return (false);
        }

        if (_delta[i] != 0.0) {
            indices[i] = (size_t)floor(
                (clampedCoords[i] - _minu[i]) / _delta[i]);
        }

        assert(indices[i] < dims[i]);
    }

    // Now find index for layered grid
    //
    size_t k;
    int rc = _bsearchKIndexCell(indices[0], indices[1], clampedCoords[2], k);
    if (rc != 0)
        return (false);

    indices.push_back(k);

    return (true);
}

bool LayeredGrid::InsideGrid(const std::vector<double> &coords) const {
    assert(coords.size() == 3);

    // Clamp coordinates on periodic boundaries to reside within the
    // grid extents (vary-dimensions can not have periodic boundaries)
    //
    vector<double> clampedCoords = coords;
    ClampCoord(clampedCoords);

    vector<size_t> indices;
    bool found = GetIndicesCell(clampedCoords, indices);
    return (found);
}

void LayeredGrid::_getBilinearWeights(const vector<double> &coords,
                                      double &iwgt, double &jwgt) const {

    vector<size_t> dims = GetDimensions();

    vector<size_t> indices0;
    bool found = GetIndicesCell(coords, indices0);
    assert(found);

    vector<size_t> indices1 = indices0;
    if (indices0[0] != dims[0] - 1) {
        indices1[0] += 1;
    }
    if (indices0[1] != dims[1] - 1) {
        indices1[1] += 1;
    }

    vector<double> coords0, coords1;
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

double LayeredGrid::_bilinearInterpolation(
    size_t i0, size_t i1, size_t j0, size_t j1,
    size_t k0, double iwgt, double jwgt) const {
    double val00, val01, val10, val11, xVal0, result;
    double xVal1 = 0.0;
    double mv = GetMissingValue();

    val00 = AccessIJK(i0, j0, k0);
    val10 = AccessIJK(i1, j0, k0);
    if ((val00 == mv) || (val10 == mv))
        return mv;
    if (val00 == mv)
        xVal0 = val10;
    else if (val10 == mv)
        xVal0 = val00;
    else
        xVal0 = val00 * (1 - iwgt) + val10 * iwgt;

    val01 = AccessIJK(i0, j1, k0);
    val11 = AccessIJK(i1, j1, k0);
    if ((val01 == mv) || (val11 == mv))
        return mv;
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

double LayeredGrid::_bilinearElevation(
    size_t i0, size_t i1, size_t j0, size_t j1,
    size_t k0, double iwgt, double jwgt) const {
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

float LayeredGrid::_getValueQuadratic(
    const std::vector<double> &coords) const {

    double mv = GetMissingValue();
    vector<size_t> dims = GetDimensions();

    // Get the indecies of the hyperslab containing the point
    // k0 = level above the point
    // k1 = level below the point
    // k2 = two levels below the point
    //
    vector<size_t> indices;
    bool found = GetIndicesCell(coords, indices);
    if (!found)
        return (GetMissingValue());

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
    if ((val0 == mv) || (val1 == mv) || (val2 == mv))
        return mv;

    // bilinearly interpolated elevations at each k0, k1, and k2
    double z0, z1, z2;
    z0 = _bilinearElevation(i0, i1, j0, j1, k0, iwgt, jwgt);
    z1 = _bilinearElevation(i0, i1, j0, j1, k1, iwgt, jwgt);
    z2 = _bilinearElevation(i0, i1, j0, j1, k2, iwgt, jwgt);
    if ((z0 == mv) || (z1 == mv) || (z2 == mv))
        return mv;

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

double LayeredGrid::_interpolateVaryingCoord(
    size_t i0, size_t j0, size_t k0,
    double x, double y) const {

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
double pointDotPlane(
    const vector<double> &v1,
    const vector<double> &v2,
    const vector<double> &v3,
    const vector<double> &p) {
    double vec1_2[3] = {v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2]};
    double vec1_3[3] = {v3[0] - v1[0], v3[1] - v1[1], v3[2] - v1[2]};
    double vp[3] = {p[0] - v1[0], p[1] - v1[1], p[2] - v1[2]};
    double normal[3];

    vcross(vec1_2, vec1_3, normal);

    return (vdot(normal, vp));
}
}; // namespace

int LayeredGrid::_bsearchKIndexCell(
    size_t i, size_t j, double z,
    size_t &k) const {
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

    GetUserCoordinates(v1idx, v1);
    GetUserCoordinates(v2idx, v2);
    GetUserCoordinates(v3idx, v3);

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
    if (d < 0.0)
        return (-1);

    v1[2] = _rg.AccessIJK(i, j, k1);
    v2[2] = _rg.AccessIJK(i + 1, j, k1);
    v3[2] = _rg.AccessIJK(i, j + 1, k1);
    d = pointDotPlane(v1, v2, v3, pt);
    if (d > 0.0)
        return (1);

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
