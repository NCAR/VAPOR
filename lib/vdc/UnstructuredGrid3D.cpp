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
#include <vapor/UnstructuredGrid3D.h>

using namespace std;
using namespace VAPoR;

UnstructuredGrid3D::UnstructuredGrid3D(const DimsType &vertexDims, const DimsType &faceDims, const DimsType &edgeDims, const DimsType &bs,
                                       const std::vector<float *> &blks, const int *vertexOnFace, const int *faceOnVertex, const int *faceOnFace,
                                       Location location,    // node,face, edge
                                       size_t maxVertexPerFace, size_t maxFacePerVertex, long nodeOffset, long cellOffset, const UnstructuredGridCoordless &xug, const UnstructuredGridCoordless &yug,
                                       const UnstructuredGridCoordless &zug)
: UnstructuredGrid(vertexDims, faceDims, edgeDims, bs, blks, 3, vertexOnFace, faceOnVertex, faceOnFace, location, maxVertexPerFace, maxFacePerVertex, nodeOffset, cellOffset), _xug(xug), _yug(yug),
  _zug(zug)
{
    VAssert(xug.GetNumDimensions() == 1);
    VAssert(yug.GetNumDimensions() == 1);
    VAssert(zug.GetNumDimensions() == 1);

    VAssert(location == NODE);
}

UnstructuredGrid3D::UnstructuredGrid3D(const std::vector<size_t> &vertexDims, const std::vector<size_t> &faceDims, const std::vector<size_t> &edgeDims, const std::vector<size_t> &bs,
                                       const std::vector<float *> &blks, const int *vertexOnFace, const int *faceOnVertex, const int *faceOnFace,
                                       Location location,    // node,face, edge
                                       size_t maxVertexPerFace, size_t maxFacePerVertex, long nodeOffset, long cellOffset, const UnstructuredGridCoordless &xug, const UnstructuredGridCoordless &yug,
                                       const UnstructuredGridCoordless &zug)
: UnstructuredGrid(vertexDims, faceDims, edgeDims, bs, blks, 3, vertexOnFace, faceOnVertex, faceOnFace, location, maxVertexPerFace, maxFacePerVertex, nodeOffset, cellOffset), _xug(xug), _yug(yug),
  _zug(zug)
{
    VAssert(xug.GetNumDimensions() == 1);
    VAssert(yug.GetNumDimensions() == 1);
    VAssert(zug.GetNumDimensions() == 1);

    VAssert(location == NODE);
}


vector<size_t> UnstructuredGrid3D::GetCoordDimensions(size_t dim) const
{
    const Grid *ptr = nullptr;

    if (dim == 0) {
        ptr = &_xug;
    } else if (dim == 1) {
        ptr = &_yug;
    } else if (dim == 2) {
        ptr = &_zug;
    } else {
        return (vector<size_t>(1, 1));
    }

    auto tmp = ptr->GetDimensions();
    auto dims = std::vector<size_t>{tmp[0], tmp[1], tmp[2]};
    dims.resize(ptr->GetNumDimensions());
    return dims;
}


size_t UnstructuredGrid3D::GetGeometryDim() const { return (3); }


void UnstructuredGrid3D::GetUserExtentsHelper(CoordType &minu, CoordType &maxu) const
{
    float range[2];

    _xug.GetRange(range);
    minu[0] = range[0];
    maxu[0] = range[1];

    _yug.GetRange(range);
    minu[1] = range[0];
    maxu[1] = range[1];

    _zug.GetRange(range);
    minu[2] = range[0];
    maxu[2] = range[1];
}


void UnstructuredGrid3D::GetBoundingBox(const DimsType &min, const DimsType &max, CoordType &minu, CoordType &maxu) const
{
    DimsType cMin;
    ClampIndex(min, cMin);

    DimsType cMax;
    ClampIndex(max, cMax);

    float range[2];

    _xug.GetRange(min, max, range);
    minu[0] = range[0];
    maxu[0] = range[1];

    _yug.GetRange(min, max, range);
    minu[1] = range[0];
    maxu[1] = range[1];

    _zug.GetRange(min, max, range);
    minu[2] = range[0];
    maxu[2] = range[1];
}


bool UnstructuredGrid3D::GetEnclosingRegion(const CoordType &minu, const CoordType &maxu, DimsType &min, DimsType &max) const
{
    VAssert(0 && "Not implemented");
    return (true);
}


void UnstructuredGrid3D::GetUserCoordinates(const DimsType &indices, CoordType &coords) const
{
    DimsType cIndices;
    ClampIndex(indices, cIndices);

    coords[0] = _xug.GetValueAtIndex(cIndices);
    coords[1] = _yug.GetValueAtIndex(cIndices);
    coords[2] = _zug.GetValueAtIndex(cIndices);
}


bool UnstructuredGrid3D::GetIndicesCell(const CoordType &coords, DimsType &indices) const
{
    vector<size_t> nodes2D;
    vector<double> lambda;
    float          zwgt[2];

    return (_insideGrid(coords, indices, nodes2D, lambda, zwgt));
}


bool UnstructuredGrid3D::InsideGrid(const CoordType &coords) const
{
    DimsType            indices;
    std::vector<size_t> nodes2D;
    vector<double>      lambda;
    float               zwgt[2];

    return (_insideGrid(coords, indices, nodes2D, lambda, zwgt));
}


bool UnstructuredGrid3D::_insideGrid(const CoordType &coords, DimsType &cindices, std::vector<size_t> &nodes2D, std::vector<double> &lambda, float zwgt[2]) const
{
    VAssert(!"Not implemented");
    return false;
}


float UnstructuredGrid3D::GetValueNearestNeighbor(const CoordType &coords) const
{
    VAssert(!"Not implemented");
    return NAN;
}


float UnstructuredGrid3D::GetValueLinear(const CoordType &coords) const
{
    VAssert(!"Not implemented");
    return NAN;
}



/////////////////////////////////////////////////////////////////////////////
//
// Iterators
//
/////////////////////////////////////////////////////////////////////////////



UnstructuredGrid3D::ConstCoordItrU3D::ConstCoordItrU3D(const UnstructuredGrid3D *ug, bool begin) : ConstCoordItrAbstract()
{
    _ug = ug;
    _coords = {0.0, 0.0, 0.0};
    if (begin) {
        _xCoordItr = ug->_xug.cbegin();
        _yCoordItr = ug->_yug.cbegin();
        _zCoordItr = ug->_zug.cbegin();
        _coords[0] = *_xCoordItr;
        _coords[1] = *_yCoordItr;
        _coords[2] = *_zCoordItr;
    } else {
        _xCoordItr = ug->_xug.cend();
        _yCoordItr = ug->_yug.cend();
        _zCoordItr = ug->_zug.cend();
    }
}


UnstructuredGrid3D::ConstCoordItrU3D::ConstCoordItrU3D(const ConstCoordItrU3D &rhs) : ConstCoordItrAbstract()
{
    _ug = rhs._ug;
    _xCoordItr = rhs._xCoordItr;
    _yCoordItr = rhs._yCoordItr;
    _zCoordItr = rhs._zCoordItr;
    _coords = rhs._coords;
}


UnstructuredGrid3D::ConstCoordItrU3D::ConstCoordItrU3D() : ConstCoordItrAbstract() {}


void UnstructuredGrid3D::ConstCoordItrU3D::next()
{
    ++_xCoordItr;
    ++_yCoordItr;
    ++_zCoordItr;

    _coords[0] = *_xCoordItr;
    _coords[1] = *_yCoordItr;
    _coords[2] = *_zCoordItr;
}


void UnstructuredGrid3D::ConstCoordItrU3D::next(const long &offset)
{
    VAssert(offset >= 0);

    _xCoordItr += offset;
    _yCoordItr += offset;
    _zCoordItr += offset;

    _coords[0] = *_xCoordItr;
    _coords[1] = *_yCoordItr;
    _coords[2] = *_zCoordItr;
}


namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const UnstructuredGrid3D &ug)
{
    o << "UnstructuredGrid3D " << endl;
    o << (const Grid &)ug;

    return o;
}
};    // namespace VAPoR
