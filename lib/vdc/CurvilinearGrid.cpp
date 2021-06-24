#include <stdio.h>
#include <iostream>
#include "vapor/VAssert.h"
#include <cmath>
#include <cfloat>
#include <limits>
#include <vapor/utils.h>
#include <vapor/CurvilinearGrid.h>
#include <vapor/QuadTreeRectangleP.h>
#include <vapor/vizutil.h>

using namespace std;
using namespace VAPoR;

void CurvilinearGrid::_curvilinearGrid(const RegularGrid &xrg, const RegularGrid &yrg, const RegularGrid &zrg, const vector<double> &zcoords, std::shared_ptr<const QuadTreeRectangleP> qtr)
{
    _zcoords.clear();
    _xrg = xrg;
    _yrg = yrg;
    _zrg = zrg;

    _zcoords = zcoords;

    GetUserExtentsHelper(_minu, _maxu);

    _qtr = qtr;
    if (!_qtr) { _qtr = _makeQuadTreeRectangle(); }
}

CurvilinearGrid::CurvilinearGrid(const vector<size_t> &dims, const vector<size_t> &bs, const vector<float *> &blks, const RegularGrid &xrg, const RegularGrid &yrg, const vector<double> &zcoords,
                                 std::shared_ptr<const QuadTreeRectangleP> qtr)
: StructuredGrid(dims, bs, blks)
{
    VAssert(dims.size() == 2 || dims.size() == 3);
    VAssert(bs.size() == dims.size());

    // Only support 2D X & Y coordinates currently. I.e. only support
    // "layered" curvilinear grids
    //
    VAssert(xrg.GetNumDimensions() == 2);
    VAssert(yrg.GetNumDimensions() == 2);
    VAssert(zcoords.size() == 0 || zcoords.size() == dims[2]);

    _terrainFollowing = false;
    _curvilinearGrid(xrg, yrg, RegularGrid(), zcoords, qtr);
}

CurvilinearGrid::CurvilinearGrid(const vector<size_t> &dims, const vector<size_t> &bs, const vector<float *> &blks, const RegularGrid &xrg, const RegularGrid &yrg, const RegularGrid &zrg,
                                 std::shared_ptr<const QuadTreeRectangleP> qtr)
: StructuredGrid(dims, bs, blks)
{
    VAssert(dims.size() == 3);
    VAssert(bs.size() == dims.size());

    // Only support 2D X & Y coordinates currently. I.e. only support
    // "layered" curvilinear grids
    //
    VAssert(xrg.GetNumDimensions() == 2);
    VAssert(yrg.GetNumDimensions() == 2);
    VAssert(zrg.GetNumDimensions() == 3);

    _terrainFollowing = true;
    _curvilinearGrid(xrg, yrg, zrg, vector<double>(), qtr);
}

CurvilinearGrid::CurvilinearGrid(const vector<size_t> &dims, const vector<size_t> &bs, const vector<float *> &blks, const RegularGrid &xrg, const RegularGrid &yrg,
                                 std::shared_ptr<const QuadTreeRectangleP> qtr)
: StructuredGrid(dims, bs, blks)
{
    VAssert(dims.size() == 2);
    VAssert(bs.size() == dims.size());

    // Only support 2D X & Y coordinates currently. I.e. only support
    // "layered" curvilinear grids
    //
    VAssert(xrg.GetNumDimensions() == 2);
    VAssert(yrg.GetNumDimensions() == 2);

    _terrainFollowing = false;
    _curvilinearGrid(xrg, yrg, RegularGrid(), vector<double>(), qtr);
}

vector<size_t> CurvilinearGrid::GetCoordDimensions(size_t dim) const
{
    if (dim == 0) {
        auto tmp = _xrg.GetDimensions();
        auto tmp2 = vector<size_t>{tmp[0], tmp[1], tmp[2]};
        tmp2.resize(_xrg.GetNumDimensions());
        return tmp2;
    } else if (dim == 1) {
        auto tmp = _yrg.GetDimensions();
        auto tmp2 = vector<size_t>{tmp[0], tmp[1], tmp[2]};
        tmp2.resize(_yrg.GetNumDimensions());
        return tmp2;
    } else if (dim == 2) {
        if (_terrainFollowing) {
            auto tmp = _zrg.GetDimensions();
            auto tmp2 = vector<size_t>{tmp[0], tmp[1], tmp[2]};
            tmp2.resize(_zrg.GetNumDimensions());
            return tmp2;
        } else {
            return (vector<size_t>(1, _zcoords.size()));
        }
    } else {
        return (vector<size_t>(1, 1));
    }
}

void CurvilinearGrid::GetBoundingBox(const Size_tArr3 &min, const Size_tArr3 &max, DblArr3 &minu, DblArr3 &maxu) const
{
    Size_tArr3 cMin;
    ClampIndex(min, cMin);

    Size_tArr3 cMax;
    ClampIndex(max, cMax);

    for (int i = 0; i < GetGeometryDim(); i++) { VAssert(cMin[i] <= cMax[i]); }

    for (int i = 0; i < minu.size(); i++) {
        minu[i] = 0.0;
        maxu[i] = 0.0;
    }

    // Get the horiztonal (X & Y) extents by visiting every point
    // on a single plane (horizontal coordinates are constant over Z).
    //
    vector<size_t> min2d = {cMin[0], cMin[1]};
    vector<size_t> max2d = {cMax[0], cMax[1]};
    float          xrange[2], yrange[2];
    _xrg.GetRange(min2d, max2d, xrange);
    _yrg.GetRange(min2d, max2d, yrange);

    minu[0] = xrange[0];
    minu[1] = yrange[0];
    maxu[0] = xrange[1];
    maxu[1] = yrange[1];

    // We're done if 2D grid
    //
    if (GetGeometryDim() == 2) return;

    if (_terrainFollowing) {
        float zrange[2];
        _zrg.GetRange(cMin, cMax, zrange);

        minu[2] = zrange[0];
        maxu[2] = zrange[1];
    } else {
        minu[2] = _zcoords[cMin[2]];
        maxu[2] = _zcoords[cMax[2]];
    }
}

void CurvilinearGrid::GetUserCoordinates(const Size_tArr3 &indices, DblArr3 &coords) const
{
    Size_tArr3 cIndices;
    ClampIndex(indices, cIndices);

    coords[0] = _xrg.AccessIJK(cIndices[0], cIndices[1]);
    coords[1] = _yrg.AccessIJK(cIndices[0], cIndices[1]);

    if (GetGeometryDim() < 3) return;

    if (_terrainFollowing) {
        coords[2] = _zrg.AccessIJK(cIndices[0], cIndices[1], cIndices[2]);
    } else {
        coords[2] = _zcoords[cIndices[2]];
    }
}

bool CurvilinearGrid::GetIndicesCell(const DblArr3 &coords, Size_tArr3 &indices) const
{
    // Clamp coordinates on periodic boundaries to grid extents
    //
    DblArr3 cCoords;
    ClampCoord(coords, cCoords);

    double x = cCoords[0];
    double y = cCoords[1];
    double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;

    double lambda[4], zwgt[2];
    size_t i, j, k;
    bool   inside = _insideGrid(x, y, z, i, j, k, lambda, zwgt);

    if (!inside) return (false);

    indices[0] = i;
    indices[1] = j;

    if (GetGeometryDim() == 2) return (true);

    indices[2] = k;

    return (true);
}

bool CurvilinearGrid::InsideGrid(const DblArr3 &coords) const
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

    double lambda[4], zwgt[2];
    size_t i, j, k;    // not used
    double x = cCoords[0];
    double y = cCoords[1];
    double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;

    bool inside = _insideGrid(x, y, z, i, j, k, lambda, zwgt);

    return (inside);
}

CurvilinearGrid::ConstCoordItrCG::ConstCoordItrCG(const CurvilinearGrid *cg, bool begin) : ConstCoordItrAbstract()
{
    _cg = cg;
    auto tmp = _cg->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(_cg->GetNumDimensions());
    _index = vector<size_t>(dims.size(), 0);
    _terrainFollowing = _cg->_terrainFollowing;
    if (begin) {
        _xCoordItr = _cg->_xrg.cbegin();
        _yCoordItr = _cg->_yrg.cbegin();
        if (_terrainFollowing) { _zCoordItr = _cg->_zrg.cbegin(); }
    } else {
        _xCoordItr = _cg->_xrg.cend();
        _yCoordItr = _cg->_yrg.cend();
        if (_terrainFollowing) { _zCoordItr = _cg->_zrg.cend(); }
        _index[dims.size() - 1] = dims[dims.size() - 1];
        return;
    }
    _coords.push_back(*_xCoordItr);
    _coords.push_back(*_yCoordItr);

    if (dims.size() == 3) {
        if (_terrainFollowing) {
            _coords.push_back(*_zCoordItr);
        } else {
            _coords.push_back(_cg->_zcoords[0]);
        }
    }
}

CurvilinearGrid::ConstCoordItrCG::ConstCoordItrCG(const ConstCoordItrCG &rhs) : ConstCoordItrAbstract()
{
    _cg = rhs._cg;
    _index = rhs._index;
    _coords = rhs._coords;
    _xCoordItr = rhs._xCoordItr;
    _yCoordItr = rhs._yCoordItr;
    if (rhs._terrainFollowing) { _zCoordItr = rhs._zCoordItr; }
    _terrainFollowing = rhs._terrainFollowing;
}

CurvilinearGrid::ConstCoordItrCG::ConstCoordItrCG() : ConstCoordItrAbstract()
{
    _cg = NULL;
    _index.clear();
    _coords.clear();
}

void CurvilinearGrid::ConstCoordItrCG::next()
{
    auto dims = _cg->GetDimensions();

    _index[0]++;
    ++_xCoordItr;
    ++_yCoordItr;
    if (_terrainFollowing) { ++_zCoordItr; }

    if (_index[0] < dims[0]) {
        _coords[0] = *_xCoordItr;
        _coords[1] = *_yCoordItr;
        if (_terrainFollowing) { _coords[2] = *_zCoordItr; }
        return;
    }

    _index[0] = 0;
    _index[1]++;

    if (_index[1] < dims[1]) {
        _coords[0] = *_xCoordItr;
        _coords[1] = *_yCoordItr;
        if (_terrainFollowing) { _coords[2] = *_zCoordItr; }
        return;
    }

    if (_cg->GetNumDimensions() == 2) return;

    _index[1] = 0;
    _index[2]++;
    if (_index[2] < dims[2]) {
        _xCoordItr = _cg->_xrg.cbegin();
        _yCoordItr = _cg->_yrg.cbegin();

        _coords[0] = *_xCoordItr;
        _coords[1] = *_yCoordItr;
        if (_terrainFollowing) {
            _coords[2] = *_zCoordItr;
        } else {
            _coords[2] = _cg->_zcoords[_index[2]];
        }
        return;
    }
}

void CurvilinearGrid::ConstCoordItrCG::next(const long &offset)
{
    if (!_index.size()) return;

    auto tmp = _cg->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(_cg->GetNumDimensions());

    vector<size_t> maxIndex;

    for (int i = 0; i < dims.size(); i++) maxIndex.push_back(dims[i] - 1);

    long maxIndexL = Wasp::LinearizeCoords(maxIndex, dims);
    long newIndexL = Wasp::LinearizeCoords(_index, dims) + offset;
    if (newIndexL < 0) { newIndexL = 0; }
    if (newIndexL > maxIndexL) {
        _index = vector<size_t>(dims.size(), 0);
        _index[dims.size() - 1] = dims[dims.size() - 1];
        return;
    }

    size_t index2DL = _index[1] * dims[0] + _index[0];

    _index = Wasp::VectorizeCoords(newIndexL, dims);

    VAssert(_index[1] * dims[0] + _index[0] >= index2DL);
    size_t offset2D = (_index[1] * dims[0] + _index[0]) - index2DL;

    _xCoordItr += offset2D;
    _yCoordItr += offset2D;

    _coords[0] = *_xCoordItr;
    _coords[1] = *_yCoordItr;

    if (dims.size() == 2) return;

    if (_terrainFollowing) {
        _zCoordItr += offset;
        _coords[2] = *_zCoordItr;
    } else {
        _coords[2] = _cg->_zcoords[_index[2]];
    }
}

float CurvilinearGrid::GetValueNearestNeighbor(const DblArr3 &coords) const
{
    // Clamp coordinates on periodic boundaries to grid extents
    //
    DblArr3 cCoords;
    ClampCoord(coords, cCoords);

    double lambda[4], zwgt[2];
    size_t i, j, k;
    double x = cCoords[0];
    double y = cCoords[1];
    double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;
    bool   inside = _insideGrid(x, y, z, i, j, k, lambda, zwgt);

    if (!inside) return (GetMissingValue());

    // Find closest point within face
    //
    double maxl = lambda[0];
    int    maxidx = 0;
    for (int idx = 1; idx < 4; idx++) {
        if (lambda[idx] > maxl) {
            maxl = lambda[idx];
            maxidx = idx;
        }
    }
    if (maxidx == 1) {
        i++;
    } else if (maxidx == 2) {
        i++;
        j++;
    } else if (maxidx == 3) {
        j++;
    }

    if (zwgt[1] > zwgt[0]) k++;

    return (AccessIJK(i, j, k));
}

namespace {

float interpolateQuad(const float values[4], const double lambda[4], float mv)
{
    double lambda0[] = {lambda[0], lambda[1], lambda[2], lambda[3]};

    // Look for missing values. If found, zero out weight
    //
    float wTotal = 0.0;
    int   nMissing = 0;
    for (int i = 0; i < 4; i++) {
        if (values[i] == mv) {
            lambda0[i] = 0.0;
            nMissing++;
        } else {
            wTotal += lambda0[i];
        }
    }

    // Re-normalize weights if we have missing values
    //
    if (nMissing) {
        wTotal = 1.0 / wTotal;
        for (int i = 0; i < 4; i++) { lambda0[i] *= wTotal; }
    }

    float v = 0.0;
    if (nMissing == 4) {
        v = mv;
    } else {
        for (int i = 0; i < 4; i++) { v += values[i] * lambda0[i]; }
    }

    return (v);
}
};    // namespace

float CurvilinearGrid::GetValueLinear(const DblArr3 &coords) const
{
    // Clamp coordinates on periodic boundaries to grid extents
    //
    DblArr3 cCoords;
    ClampCoord(coords, cCoords);

    // Get Wachspress coordinates for horizontal weights, and
    // simple linear interpolation weights for vertical axis. _insideGrid
    // handlese case where grid is 2D. I.e. if 2d then zwgt[0] == 1 &&
    // zwgt[1] = 0.0
    //
    double lambda[4], zwgt[2];
    size_t i, j, k;
    double x = cCoords[0];
    double y = cCoords[1];
    double z = GetGeometryDim() == 3 ? cCoords[2] : 0.0;
    bool   inside = _insideGrid(x, y, z, i, j, k, lambda, zwgt);

    float mv = GetMissingValue();

    if (!inside) return (mv);

    // Use Wachspress coordinates as weights to do linear interpolation
    // along XY plane
    //
    auto dims = GetDimensions();
    VAssert(i < dims[0] - 1);
    VAssert(j < dims[1] - 1);
    if (GetNumDimensions() > 2) VAssert(k < dims[2]);

    float v0s[] = {AccessIJK(i, j, k), AccessIJK(i + 1, j, k), AccessIJK(i + 1, j + 1, k), AccessIJK(i, j + 1, k)};

    float v0 = interpolateQuad(v0s, lambda, mv);

    if (GetGeometryDim() == 2 || dims[2] < 2) return (v0);

    if (v0 == mv) zwgt[0] = 0.0;

    float v1s[] = {AccessIJK(i, j, k + 1), AccessIJK(i + 1, j, k + 1), AccessIJK(i + 1, j + 1, k + 1), AccessIJK(i, j + 1, k + 1)};

    float v1 = interpolateQuad(v1s, lambda, mv);

    if (v1 == mv) zwgt[1] = 0.0;

    // Linearly interpolate along Z axis
    //
    if (zwgt[0] == 0.0)
        return (v1);
    else if (zwgt[1] == 0.0)
        return (v0);
    else
        return (v0 * zwgt[0] + v1 * zwgt[1]);
}

void CurvilinearGrid::GetUserExtentsHelper(DblArr3 &minu, DblArr3 &maxu) const
{
    // Get the horiztonal (X & Y) extents by visiting every point
    // on a single plane (horizontal coordinates are constant over Z).
    //
    float xrange[2], yrange[2];
    _xrg.GetRange(xrange);
    _yrg.GetRange(yrange);

    minu[0] = xrange[0];
    minu[1] = yrange[0];
    maxu[0] = xrange[1];
    maxu[1] = yrange[1];

    // We're done if 2D grid
    //
    if (GetGeometryDim() == 2) return;

    if (_terrainFollowing) {
        float zrange[2];
        _zrg.GetRange(zrange);

        minu[2] = zrange[0];
        maxu[2] = zrange[1];
    } else {
        minu[2] = _zcoords[0];
        maxu[2] = _zcoords[_zcoords.size() - 1];
    }
}

bool CurvilinearGrid::_insideGridHelperStretched(double z, size_t &k, double zwgt[2]) const
{
    // Now verify that Z coordinate of point is in grid, and find
    // its interpolation weights if so.
    //
    size_t kFound = 0;

    if (!Wasp::BinarySearchRange(_zcoords, z, kFound)) return (false);

    k = kFound;
    zwgt[0] = 1.0 - (z - _zcoords[k]) / (_zcoords[k + 1] - _zcoords[k]);
    zwgt[1] = 1.0 - zwgt[0];

    return (true);
}

bool CurvilinearGrid::_insideGridHelperTerrain(double x, double y, double z, const size_t &i, const size_t &j, size_t &k, double zwgt[2]) const
{
    // XZ and YZ cell sides are planar, but XY sides may not be. We divide
    // the XY faces into two triangles (changing hexahedrals into prims)
    // and figure out which triangle (prism) the point is in (first or
    // second). Then we search the stack of first (or second) prism in Z
    //
    //

    // Check if point is in "first" triangle (0,0), (1,0), (1,1)
    //
    double     lambda[3];
    double     pt[] = {x, y};
    Size_tArr3 iv = {i, i + 1, i + 1};
    Size_tArr3 jv = {j, j, j + 1};
    double     tverts0[] = {_xrg.AccessIJK(iv[0], jv[0], 0), _yrg.AccessIJK(iv[0], jv[0], 0), _xrg.AccessIJK(iv[1], jv[1], 0),
                        _yrg.AccessIJK(iv[1], jv[1], 0), _xrg.AccessIJK(iv[2], jv[2], 0), _yrg.AccessIJK(iv[2], jv[2], 0)};

    bool inside = VAPoR::BarycentricCoordsTri(tverts0, pt, lambda);
    if (!inside) {
        // Not in first triangle.
        // Now check if point is in "second" triangle (0,0), (1,1), (0,1)
        //
        iv = {i, i + 1, i};
        jv = {j, j + 1, j + 1};
        double tverts1[] = {_xrg.AccessIJK(iv[0], jv[0], 0), _yrg.AccessIJK(iv[0], jv[0], 0), _xrg.AccessIJK(iv[1], jv[1], 0),
                            _yrg.AccessIJK(iv[1], jv[1], 0), _xrg.AccessIJK(iv[2], jv[2], 0), _yrg.AccessIJK(iv[2], jv[2], 0)};

        inside = VAPoR::BarycentricCoordsTri(tverts1, pt, lambda);
        if (!inside) return (false);
    }

    float z0, z1;

    // Find k index of cell containing z. Already know i and j indices
    //
    size_t         nz = GetDimensions()[2];
    vector<double> zcoords(nz);
    for (int kk = 0; kk < nz; kk++) {
        // Interpolate Z coordinate across triangle
        //
        float zk = _zrg.AccessIJK(iv[0], jv[0], kk) * lambda[0] + _zrg.AccessIJK(iv[1], jv[1], kk) * lambda[1] + _zrg.AccessIJK(iv[2], jv[2], kk) * lambda[2];

        zcoords[kk] = zk;
    }

    if (!Wasp::BinarySearchRange(zcoords, z, k)) return (false);

    z0 = zcoords[k];
    z1 = k < nz - 1 ? zcoords[k + 1] : z0;

    zwgt[0] = 1.0 - (z - z0) / (z1 - z0);
    zwgt[1] = 1.0 - zwgt[0];

    return (true);
}

bool CurvilinearGrid::_insideFace(const Size_tArr3 &face, double pt[2], double lambda[4], vector<Size_tArr3> &nodes) const
{
    DblArr3 verts[4];    // space for 4 vertices with 3D user coordinates

    size_t gDim = GetGeometryDim();

    bool ok = GetCellNodes(face, nodes);
    VAssert(ok);

    // For 3D data GetCellNodes returns 3D cells. We only need the 2D
    // bottom face (all cells in a vertical column have same horizontal
    // coordinates
    // for layered 3D data)
    //
    size_t n = nodes.size();
    if (gDim > 2 && n == 8) n /= 2;
    VAssert(n == 4);

    // Get X and Y coordinates for each vertex making up the face
    //
    for (int i = 0; i < n; i++) { GetUserCoordinates(nodes[i], verts[i]); }

    // The following functions operate on packed, raw arrays
    //
    double verts2d[] = {verts[0][0], verts[0][1], verts[1][0], verts[1][1], verts[2][0], verts[2][1], verts[3][0], verts[3][1]};
    if (!Grid::PointInsideBoundingRectangle(pt, verts2d, 4)) { return (false); }

    bool ret = WachspressCoords2D(verts2d, pt, 4, lambda);

    return ret;
}

// Search for a point inside the grid. If the point is inside return true,
// and provide the Wachspress weights/coordinates for the point within
// the XY quadrilateral cell containing the point in XY, and the linear
// interpolation weights/coordinates along Z. If the grid is 2D then
// zwgt[0] == 1.0, and zwgt[1] == 0.0. If the point is outside of the
// grid the values of 'lambda', and 'zwgt' are not defined
//
bool CurvilinearGrid::_insideGrid(double x, double y, double z, size_t &i, size_t &j, size_t &k, double lambda[4], double zwgt[2]) const
{
    for (int l = 0; l < 4; l++) lambda[l] = 0.0;
    for (int l = 0; l < 2; l++) zwgt[l] = 0.0;
    i = j = k = 0;

    // Find the indices for the faces that might contain the point
    //
    vector<Size_tArr3> face_indices;
    _qtr->GetPayloadContained(x, y, face_indices);

    bool               inside = false;
    double             pt[] = {x, y};
    vector<Size_tArr3> nodes(8);
    for (int ii = 0; ii < face_indices.size(); ii++) {
        if (_insideFace(face_indices[ii], pt, lambda, nodes)) {
            i = face_indices[ii][0];
            j = face_indices[ii][1];

            inside = true;
            break;
        }
    }

    if (!inside) return (false);

    if (GetGeometryDim() == 2) {
        zwgt[0] = 1.0;
        zwgt[1] = 0.0;
        return (true);
    }

    if (_terrainFollowing) {
        return (_insideGridHelperTerrain(x, y, z, i, j, k, zwgt));
    } else {
        return (_insideGridHelperStretched(z, k, zwgt));
    }
}

std::shared_ptr<QuadTreeRectangleP> CurvilinearGrid::_makeQuadTreeRectangle() const
{
    const vector<size_t> &dims = GetCellDimensions();
    size_t                reserve_size = dims[0] * dims[1];

    DblArr3 minu, maxu;
    GetUserExtents(minu, maxu);

    std::shared_ptr<QuadTreeRectangleP> qtr = std::make_shared<QuadTreeRectangleP>((float)minu[0], (float)minu[1], (float)maxu[0], (float)maxu[1], 12, reserve_size);

    qtr->Insert(this, dims[0] * dims[1]);
    return (qtr);
}
