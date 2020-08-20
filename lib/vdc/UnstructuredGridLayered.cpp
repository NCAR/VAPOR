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

UnstructuredGridLayered::UnstructuredGridLayered(const std::vector<size_t> &vertexDims, const std::vector<size_t> &faceDims, const std::vector<size_t> &edgeDims, const std::vector<size_t> &bs,
                                                 const std::vector<float *> &blks, const int *vertexOnFace, const int *faceOnVertex, const int *faceOnFace,
                                                 Location location,    // node,face, edge
                                                 size_t maxVertexPerFace, size_t maxFacePerVertex, long nodeOffset, long cellOffset, const UnstructuredGridCoordless &xug,
                                                 const UnstructuredGridCoordless &yug, const UnstructuredGridCoordless &zug, std::shared_ptr<const QuadTreeRectangle<float, size_t>> qtr)
: UnstructuredGrid(vertexDims, faceDims, edgeDims, bs, blks, 3, vertexOnFace, faceOnVertex, faceOnFace, location, maxVertexPerFace, maxFacePerVertex, nodeOffset, cellOffset),
  _ug2d(vector<size_t>{vertexDims[0]}, vector<size_t>{faceDims[0]}, edgeDims.size() ? vector<size_t>{edgeDims[0]} : vector<size_t>(), vector<size_t>{bs[0]}, vector<float *>(), vertexOnFace,
        faceOnVertex, faceOnFace, location, maxVertexPerFace, maxFacePerVertex, nodeOffset, cellOffset, xug, yug, UnstructuredGridCoordless(), qtr),
  _zug(zug)
{
    VAssert(xug.GetDimensions().size() == 1);
    VAssert(yug.GetDimensions().size() == 1);
    VAssert(zug.GetDimensions().size() == 2);

    VAssert(location == NODE);
}

vector<size_t> UnstructuredGridLayered::GetCoordDimensions(size_t dim) const
{
    if (dim == 0) {
        return (_ug2d.GetCoordDimensions(dim));
    } else if (dim == 1) {
        return (_ug2d.GetCoordDimensions(dim));
    } else if (dim == 2) {
        return (_zug.GetDimensions());
    } else {
        return (vector<size_t>(1, 1));
    }
}

size_t UnstructuredGridLayered::GetGeometryDim() const { return (3); }

void UnstructuredGridLayered::GetUserExtentsHelper(DblArr3 &minu, DblArr3 &maxu) const
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

void UnstructuredGridLayered::GetBoundingBox(const Size_tArr3 &min, const Size_tArr3 &max, DblArr3 &minu, DblArr3 &maxu) const
{
    Size_tArr3 cMin;
    ClampIndex(min, cMin);

    Size_tArr3 cMax;
    ClampIndex(max, cMax);

    _ug2d.GetBoundingBox(cMin, cMax, minu, maxu);

    float range[2];
    _zug.GetRange(min, max, range);
    minu[2] = range[0];
    maxu[2] = range[1];
}

bool UnstructuredGridLayered::GetEnclosingRegion(const DblArr3 &minu, const DblArr3 &maxu, Size_tArr3 &min, Size_tArr3 &max) const
{
    VAssert(0 && "Not implemented");
    return (true);
}

void UnstructuredGridLayered::GetUserCoordinates(const Size_tArr3 &indices, DblArr3 &coords) const
{
    Size_tArr3 cIndices;
    ClampIndex(indices, cIndices);

    _ug2d.GetUserCoordinates(cIndices, coords);

    coords[2] = _zug.GetValueAtIndex(cIndices);
}

bool UnstructuredGridLayered::_insideGrid(const DblArr3 &coords, Size_tArr3 &cindices, std::vector<size_t> &nodes2D, std::vector<double> &lambda, float zwgt[2]) const
{
    VAssert(_location == NODE);

    nodes2D.clear();
    lambda.clear();

    DblArr3 cCoords;
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

bool UnstructuredGridLayered::GetIndicesCell(const DblArr3 &coords, Size_tArr3 &indices) const
{
    vector<size_t> nodes2D;
    vector<double> lambda;
    float          zwgt[2];

    return (_insideGrid(coords, indices, nodes2D, lambda, zwgt));
}

bool UnstructuredGridLayered::InsideGrid(const DblArr3 &coords) const
{
    Size_tArr3          indices;
    std::vector<size_t> nodes2D;
    vector<double>      lambda;
    float               zwgt[2];

    return (_insideGrid(coords, indices, nodes2D, lambda, zwgt));
}

float UnstructuredGridLayered::GetValueNearestNeighbor(const DblArr3 &coords) const
{
    Size_tArr3          indices;
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

float UnstructuredGridLayered::GetValueLinear(const DblArr3 &coords) const
{
    Size_tArr3          indices;
    std::vector<size_t> nodes2D;
    vector<double>      lambda;
    float               zwgt[2];

    bool inside = _insideGrid(coords, indices, nodes2D, lambda, zwgt);
    if (!inside) return (GetMissingValue());

    // Interpolate value inside bottom face
    //
    float  z0 = 0.0;
    size_t k0 = indices[1];
    for (int i = 0; i < lambda.size(); i++) { z0 += AccessIJK(nodes2D[i], k0) * lambda[i]; }

    // Interpolate value inside top face
    //
    float z1 = 0.0;
    float k1 = indices[1] + 1;
    for (int i = 0; i < lambda.size(); i++) { z1 += AccessIJK(nodes2D[i], k1) * lambda[i]; }

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
    _coords = vector<double>(3, 0.0);
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

    long offset2D = offset % _nElements2D;

    if (offset2D + _index2D < _nElements2D) {
        _itr2D += offset;
        _index2D += offset;
    } else {
        size_t o = (offset2D + _index2D) % _nElements2D;
        _itr2D = _ug->_ug2d.ConstCoordBegin() + o;
        _index2D = o;
    }

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
