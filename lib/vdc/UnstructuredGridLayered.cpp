#include <iostream>
#include <vector>
#include "vapor/VAssert.h"
#include <cmath>
#include <time.h>
#ifdef Darwin
    #include <mach/mach_time.h>
#endif
#ifdef _WINDOWS
    #include "windows.h"
    #include "Winbase.h"
    #include <limits>
#endif

#include <vapor/utils.h>
#include <vapor/vizutil.h>
#include <vapor/UnstructuredGridLayered.h>

using namespace std;
using namespace VAPoR;

UnstructuredGridLayered::UnstructuredGridLayered(const DimsType &vertexDims, const DimsType &faceDims, const DimsType &edgeDims, const DimsType &bs, const std::vector<float *> &blks,
                                                 const int *vertexOnFace, const int *faceOnVertex, const int *faceOnFace,
                                                 Location location,    // node,face, edge
                                                 size_t maxVertexPerFace, size_t maxFacePerVertex, long nodeOffset, long cellOffset, const UnstructuredGridCoordless &xug,
                                                 const UnstructuredGridCoordless &yug, const UnstructuredGridCoordless &zug, std::shared_ptr<const QuadTreeRectangleP> qtr)
: UnstructuredGrid(vertexDims, faceDims, edgeDims, bs, blks, 3, vertexOnFace, faceOnVertex, faceOnFace, location, maxVertexPerFace, maxFacePerVertex, nodeOffset, cellOffset),
  _ug2d(vector<size_t>{vertexDims[0]}, vector<size_t>{faceDims[0]}, edgeDims.size() ? vector<size_t>{edgeDims[0]} : vector<size_t>(), vector<size_t>{bs[0]}, vector<float *>(), vertexOnFace,
        faceOnVertex, faceOnFace, location, maxVertexPerFace, maxFacePerVertex, nodeOffset, cellOffset, xug, yug, UnstructuredGridCoordless(), qtr),
  _zug(zug)
{
    VAssert(xug.GetNumDimensions() == 1);
    VAssert(yug.GetNumDimensions() == 1);
    VAssert(zug.GetNumDimensions() == 2);

    VAssert(location == NODE);
}

UnstructuredGridLayered::UnstructuredGridLayered(const std::vector<size_t> &vertexDims, const std::vector<size_t> &faceDims, const std::vector<size_t> &edgeDims, const std::vector<size_t> &bs,
                                                 const std::vector<float *> &blks, const int *vertexOnFace, const int *faceOnVertex, const int *faceOnFace,
                                                 Location location,    // node,face, edge
                                                 size_t maxVertexPerFace, size_t maxFacePerVertex, long nodeOffset, long cellOffset, const UnstructuredGridCoordless &xug,
                                                 const UnstructuredGridCoordless &yug, const UnstructuredGridCoordless &zug, std::shared_ptr<const QuadTreeRectangleP> qtr)
: UnstructuredGrid(vertexDims, faceDims, edgeDims, bs, blks, 3, vertexOnFace, faceOnVertex, faceOnFace, location, maxVertexPerFace, maxFacePerVertex, nodeOffset, cellOffset),
  _ug2d(vector<size_t>{vertexDims[0]}, vector<size_t>{faceDims[0]}, edgeDims.size() ? vector<size_t>{edgeDims[0]} : vector<size_t>(), vector<size_t>{bs[0]}, vector<float *>(), vertexOnFace,
        faceOnVertex, faceOnFace, location, maxVertexPerFace, maxFacePerVertex, nodeOffset, cellOffset, xug, yug, UnstructuredGridCoordless(), qtr),
  _zug(zug)
{
    VAssert(xug.GetNumDimensions() == 1);
    VAssert(yug.GetNumDimensions() == 1);
    VAssert(zug.GetNumDimensions() == 2);

    VAssert(location == NODE);
}

DimsType UnstructuredGridLayered::GetCoordDimensions(size_t dim) const
{
    DimsType dims = {1, 1, 1};

    if (dim == 0) {
        dims = _ug2d.GetCoordDimensions(dim);
    } else if (dim == 1) {
        dims = _ug2d.GetCoordDimensions(dim);
    } else if (dim == 2) {
        dims = _zug.GetDimensions();
    }

    return (dims);
}

size_t UnstructuredGridLayered::GetGeometryDim() const { return (3); }

void UnstructuredGridLayered::GetUserExtentsHelper(CoordType &minu, CoordType &maxu) const
{
    // Get horizontal extents from base class
    //
    _ug2d.GetUserExtents(minu, maxu);

    // Now add vertical extents
    //
    float range[2];
    _zug.GetRange(range);
    minu[2] = range[0];
    maxu[2] = range[1];
}

void UnstructuredGridLayered::GetBoundingBox(const DimsType &min, const DimsType &max, CoordType &minu, CoordType &maxu) const
{
    DimsType cMin;
    ClampIndex(min, cMin);

    DimsType cMax;
    ClampIndex(max, cMax);

    _ug2d.GetBoundingBox(cMin, cMax, minu, maxu);

    float range[2];
    _zug.GetRange(min, max, range);
    minu[2] = range[0];
    maxu[2] = range[1];
}

bool UnstructuredGridLayered::GetEnclosingRegion(const CoordType &minu, const CoordType &maxu, DimsType &min, DimsType &max) const
{
    VAssert(0 && "Not implemented");
    return (true);
}

void UnstructuredGridLayered::GetUserCoordinates(const DimsType &indices, CoordType &coords) const
{
    DimsType cIndices;
    ClampIndex(indices, cIndices);

    _ug2d.GetUserCoordinates(cIndices, coords);

    coords[2] = _zug.GetValueAtIndex(cIndices);
}

bool UnstructuredGridLayered::_insideGrid(const CoordType &coords, DimsType &cindices, std::vector<size_t> &nodes2D, std::vector<double> &lambda, float zwgt[2]) const
{
    VAssert(_location == NODE);

    nodes2D.clear();
    lambda.clear();

    CoordType cCoords;
    ClampCoord(coords, cCoords);

    // Find the 2D horizontal cell containing the X,Y coordinates
    //
    std::vector<std::vector<size_t>> nodes;

    bool status = _ug2d.GetIndicesCell(cCoords, cindices, nodes, lambda);
    if (!status) return (status);

    VAssert(lambda.size() == nodes.size());
    for (int i = 0; i < nodes.size(); i++) { nodes2D.push_back(nodes[i][0]); }

    // Find k index of cell containing z
    //
    vector<double> zcoords;

    size_t nz = GetDimensions()[1];
    for (int kk = 0; kk < nz; kk++) {
        float z = 0.0;
        for (int i = 0; i < lambda.size(); i++) { z += _zug.AccessIJK(nodes2D[i], kk) * lambda[i]; }

        zcoords.push_back(z);
    }

    size_t k;
    if (!Wasp::BinarySearchRange(zcoords, cCoords[2], k)) return (false);

    VAssert(k >= 0 && k < nz);
    cindices[1] = k;

    float z = cCoords[2];
    zwgt[0] = 1.0 - (z - zcoords[k]) / (zcoords[k + 1] - zcoords[k]);
    zwgt[1] = 1.0 - zwgt[0];

    return (true);
}

bool UnstructuredGridLayered::GetIndicesCell(const CoordType &coords, DimsType &indices) const
{
    vector<size_t> nodes2D;
    vector<double> lambda;
    float          zwgt[2];

    return (_insideGrid(coords, indices, nodes2D, lambda, zwgt));
}

bool UnstructuredGridLayered::InsideGrid(const CoordType &coords) const
{
    DimsType            indices;
    std::vector<size_t> nodes2D;
    vector<double>      lambda;
    float               zwgt[2];

    return (_insideGrid(coords, indices, nodes2D, lambda, zwgt));
}

float UnstructuredGridLayered::GetValueNearestNeighbor(const CoordType &coords) const
{
    DimsType            indices;
    std::vector<size_t> nodes2D;
    vector<double>      lambda;
    float               zwgt[2];

    bool inside = _insideGrid(coords, indices, nodes2D, lambda, zwgt);
    if (!inside) return (GetMissingValue());

    // Find nearest node in XY plane (the curvilinear part of grid)
    // The nearest node will have largest lambda resampling value.
    //
    float max_lambda = 0.0;
    int   max_nodes2d_index = 0;
    for (int i = 0; i < lambda.size(); i++) {
        if (lambda[i] > max_lambda) {
            max_lambda = lambda[i];
            max_nodes2d_index = i;
        }
    }

    // Now find out which node is closest along vertical axis. We rely on
    // the Cell index returned in 'indices' being identical to the node ID
    // along the vertical axis ('cause its a layered grid)
    //
    int max_vert_id = indices[1];
    if (zwgt[1] > zwgt[0]) { max_vert_id += 1; }

    return (AccessIJK(nodes2D[max_nodes2d_index], max_vert_id));
}

float UnstructuredGridLayered::GetValueLinear(const CoordType &coords) const
{
    DimsType            indices;
    std::vector<size_t> nodes2D;
    vector<double>      lambda;
    float               zwgt[2];

    bool inside = _insideGrid(coords, indices, nodes2D, lambda, zwgt);
    if (!inside) return (GetMissingValue());

    // Interpolate value inside bottom face
    //
    float mv = GetMissingValue();

    float  z0 = 0.0;
    size_t k0 = indices[1];
    for (size_t i = 0; i < lambda.size(); i++) {
        float v = AccessIJK(nodes2D[i], k0, 0);
        if (v == mv) {
            if (lambda[i] != 0.0) {
                z0 = mv;
                break;
            } else
                v = 0.0;
        }

        z0 += v * lambda[i];
    }

    if (z0 == mv) return (mv);

    size_t k1 = k0 + 1;
    if (k1 >= GetDimensions()[1] || zwgt[1] == 0.0) return (z0);

    // Interpolate value inside top face
    //
    float z1 = 0.0;
    for (int i = 0; i < lambda.size(); i++) {
        float v = AccessIJK(nodes2D[i], k1, 0);
        if (v == mv) {
            if (lambda[i] != 0.0) {
                z1 = mv;
                break;
            } else
                v = 0.0;
        }

        z1 += v * lambda[i];
    }

    if (z1 == mv) return (mv);

    return (z0 * zwgt[0] + z1 * zwgt[1]);
}

/////////////////////////////////////////////////////////////////////////////
//
// Iterators
//
/////////////////////////////////////////////////////////////////////////////

UnstructuredGridLayered::ConstCoordItrULayered::ConstCoordItrULayered(const UnstructuredGridLayered *ug, bool begin) : ConstCoordItrAbstract()
{
    _ug = ug;
    _coords = {0.0, 0.0, 0.0};
    _nElements2D = ug->GetDimensions()[0];
    if (begin) {
        _itr2D = ug->_ug2d.ConstCoordBegin();
        _zCoordItr = ug->_zug.cbegin();
        _index2D = 0;
        _coords[0] = (*_itr2D)[0];
        _coords[1] = (*_itr2D)[1];
        _coords[2] = *_zCoordItr;
    } else {
        _itr2D = ug->_ug2d.ConstCoordEnd();
        _zCoordItr = ug->_zug.cend();
        _index2D = _nElements2D - 1;
    }
}

UnstructuredGridLayered::ConstCoordItrULayered::ConstCoordItrULayered(const ConstCoordItrULayered &rhs) : ConstCoordItrAbstract()
{
    _ug = rhs._ug;
    _itr2D = rhs._itr2D;
    _zCoordItr = rhs._zCoordItr;
    _coords = rhs._coords;
    _nElements2D = rhs._nElements2D;
    _index2D = rhs._index2D;
}

UnstructuredGridLayered::ConstCoordItrULayered::ConstCoordItrULayered() : ConstCoordItrAbstract() {}

void UnstructuredGridLayered::ConstCoordItrULayered::next()
{
    ++_index2D;

    ++_itr2D;

    // Check for overflow
    //
    if (_index2D == _nElements2D) {
        _itr2D = _ug->_ug2d.ConstCoordBegin();
        _index2D = 0;
    }

    _coords[0] = (*_itr2D)[0];
    _coords[1] = (*_itr2D)[1];

    ++_zCoordItr;
    _coords[2] = *_zCoordItr;
}

void UnstructuredGridLayered::ConstCoordItrULayered::next(const long &offset)
{
    VAssert(offset >= 0);

    long offset2D = (_index2D + offset) % _nElements2D;

    _itr2D += (offset2D - _index2D);
    _index2D += (offset2D - _index2D);

    _coords[0] = (*_itr2D)[0];
    _coords[1] = (*_itr2D)[1];

    _zCoordItr += offset;
    _coords[2] = *_zCoordItr;
}

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const UnstructuredGridLayered &ug)
{
    o << "UnstructuredGridLayered " << endl;
    o << (const Grid &)ug;

    return o;
}
};    // namespace VAPoR
