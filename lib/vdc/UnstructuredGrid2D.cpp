#include <iostream>
#include <vector>
#include <cassert>
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
#include <vapor/UnstructuredGrid2D.h>

using namespace std;
using namespace VAPoR;

UnstructuredGrid2D::UnstructuredGrid2D(const std::vector<size_t> &vertexDims, const std::vector<size_t> &faceDims, const std::vector<size_t> &edgeDims, const std::vector<size_t> &bs,
                                       const std::vector<float *> &blks, const int *vertexOnFace, const int *faceOnVertex, const int *faceOnFace,
                                       Location location,    // node,face, edge
                                       size_t maxVertexPerFace, size_t maxFacePerVertex, const UnstructuredGridCoordless &xug, const UnstructuredGridCoordless &yug,
                                       const UnstructuredGridCoordless &zug, const KDTreeRG *kdtree)
: UnstructuredGrid(vertexDims, faceDims, edgeDims, bs, blks, 2, vertexOnFace, faceOnVertex, faceOnFace, location, maxVertexPerFace, maxFacePerVertex), _xug(xug), _yug(yug), _zug(zug), _kdtree(kdtree)
{
    assert(xug.GetDimensions() == GetDimensions());
    assert(yug.GetDimensions() == GetDimensions());
    assert(zug.GetDimensions() == GetDimensions() || zug.GetDimensions().size() == 0);

    assert(location == NODE);
}

size_t UnstructuredGrid2D::GetGeometryDim() const { return (_zug.GetDimensions().size() == 0 ? 2 : 3); }

void UnstructuredGrid2D::GetUserExtents(vector<double> &minu, vector<double> &maxu) const
{
    minu.clear();
    maxu.clear();

    float range[2];

    _xug.GetRange(range);
    minu.push_back(range[0]);
    maxu.push_back(range[1]);

    _yug.GetRange(range);
    minu.push_back(range[0]);
    maxu.push_back(range[1]);

    if (GetGeometryDim() < 3) return;

    _zug.GetRange(range);
    minu.push_back(range[0]);
    maxu.push_back(range[1]);
}

void UnstructuredGrid2D::GetBoundingBox(const vector<size_t> &min, const vector<size_t> &max, vector<double> &minu, vector<double> &maxu) const
{
    vector<size_t> cMin = min;
    ClampIndex(cMin);

    vector<size_t> cMax = max;
    ClampIndex(cMax);

    assert(cMin.size() == cMax.size());

    int ncoords = GetGeometryDim();
    minu = vector<double>(ncoords, 0.0);
    maxu = vector<double>(ncoords, 0.0);

    size_t start = Wasp::LinearizeCoords(cMin, GetDimensions());
    size_t stop = Wasp::LinearizeCoords(cMax, GetDimensions());

    // Currently only support ++ opererator for ConstCoordItr. So random
    // access is tricky.
    //
    ConstCoordItr itr = ConstCoordBegin();
    ConstCoordItr enditr = ConstCoordBegin();

    for (size_t i = 0; i < start; i++) ++itr;
    for (size_t i = 0; i < stop; i++) ++enditr;

    for (; itr != enditr; ++itr) {
        for (int i = 0; i < ncoords; i++) {
            if ((*itr)[i] < minu[i]) minu[i] = (*itr)[i];
            if ((*itr)[i] > maxu[i]) maxu[i] = (*itr)[i];
        }
    }
}

void UnstructuredGrid2D::GetEnclosingRegion(const vector<double> &minu, const vector<double> &maxu, vector<size_t> &min, vector<size_t> &max) const
{
    vector<double> cMinu = minu;
    ClampCoord(cMinu);

    vector<double> cMaxu = maxu;
    ClampCoord(cMaxu);

    assert(0 && "Not implemented");
}

void UnstructuredGrid2D::GetUserCoordinates(const std::vector<size_t> &indices, std::vector<double> &coords) const
{
    vector<size_t> cIndices = indices;
    ClampIndex(cIndices);

    assert(cIndices.size() == 1);
    coords.clear();

    coords.push_back(_xug.AccessIndex(cIndices));
    coords.push_back(_yug.AccessIndex(cIndices));
    if (GetGeometryDim() == 3) { coords.push_back(_zug.AccessIndex(cIndices)); }
}

void UnstructuredGrid2D::GetIndices(const std::vector<double> &coords, std::vector<size_t> &indices) const
{
    indices.clear();

    // Clamp coordinates on periodic boundaries to grid extents
    //
    vector<double> cCoords = coords;
    ClampCoord(cCoords);

    _kdtree->Nearest(cCoords, indices);
}

bool UnstructuredGrid2D::GetIndicesCell(const std::vector<double> &coords, std::vector<size_t> &cindices, std::vector<std::vector<size_t>> &nodes, std::vector<double> &lambdav) const
{
    cindices.clear();
    nodes.clear();
    lambdav.clear();

    vector<double> cCoords = coords;
    ClampCoord(cCoords);

    double *lambda = new double[_maxVertexPerFace];
    int     nlambda;

    // See if point is inside any cells (faces)
    //
    size_t              my_index;
    std::vector<size_t> my_nodes;
    bool                status = _insideGridNodeCentered(cCoords, my_index, my_nodes, lambda, nlambda);

    if (status) {
        cindices.push_back(my_index);
        for (int i = 0; i < nlambda; i++) {
            lambdav.push_back(lambda[i]);
            nodes.push_back(vector<size_t>(nodes[i]));
        }
    }

    delete[] lambda;

    return (status);
}

bool UnstructuredGrid2D::InsideGrid(const std::vector<double> &coords) const
{
    vector<double> cCoords = coords;
    ClampCoord(cCoords);

    double *       lambda = new double[_maxVertexPerFace];
    int            nlambda;
    size_t         face;
    vector<size_t> nodes;

    // See if point is inside any cells (faces)
    //
    bool status = _insideGridNodeCentered(cCoords, face, nodes, lambda, nlambda);

    delete[] lambda;

    return (status);
}

float UnstructuredGrid2D::GetValueNearestNeighbor(const std::vector<double> &coords) const
{
    // Clamp coordinates on periodic boundaries to reside within the
    // grid extents
    //
    vector<double> cCoords = coords;
    ClampCoord(cCoords);

    vector<size_t> vertex_indices;
    _kdtree->Nearest(cCoords, vertex_indices);
    assert(vertex_indices.size() == 1);

    return (AccessIndex(vertex_indices));
}

float UnstructuredGrid2D::GetValueLinear(const std::vector<double> &coords) const
{
    // Clamp coordinates on periodic boundaries to reside within the
    // grid extents
    //
    vector<double> cCoords = coords;
    ClampCoord(cCoords);

    double *       lambda = new double[_maxVertexPerFace];
    int            nlambda;
    size_t         face;
    vector<size_t> nodes;

    // See if point is inside any cells (faces)
    //
    bool inside = _insideGrid(cCoords, face, nodes, lambda, nlambda);

    if (!inside) {
        delete[] lambda;
        return (GetMissingValue());
    }
    assert(face < GetCellDimensions()[0]);

    const int *ptr = _vertexOnFace + (face * _maxVertexPerFace);

    double value = 0;
    long   offset = GetNodeOffset();
    for (int i = 0; i < nlambda; i++) {
        value += AccessIJK(*ptr + offset, 0, 0) * lambda[i];
        ptr++;
    }

    delete[] lambda;

    return ((float)value);
}

/////////////////////////////////////////////////////////////////////////////
//
// Iterators
//
/////////////////////////////////////////////////////////////////////////////

UnstructuredGrid2D::ConstCoordItrU2D::ConstCoordItrU2D(const UnstructuredGrid2D *ug, bool begin) : ConstCoordItrAbstract()
{
    _ncoords = ug->GetGeometryDim();
    _coords = vector<double>(_ncoords, 0.0);
    if (begin) {
        _xCoordItr = ug->_xug.cbegin();
        _yCoordItr = ug->_yug.cbegin();
        _zCoordItr = ug->_zug.cbegin();
    } else {
        _xCoordItr = ug->_xug.cend();
        _yCoordItr = ug->_yug.cend();
        _zCoordItr = ug->_zug.cend();
    }
}

UnstructuredGrid2D::ConstCoordItrU2D::ConstCoordItrU2D(const ConstCoordItrU2D &rhs) : ConstCoordItrAbstract()
{
    _ncoords = rhs._ncoords;
    _coords = rhs._coords;
    _xCoordItr = rhs._xCoordItr;
    _yCoordItr = rhs._yCoordItr;
    _zCoordItr = rhs._zCoordItr;
}

UnstructuredGrid2D::ConstCoordItrU2D::ConstCoordItrU2D() : ConstCoordItrAbstract() {}

void UnstructuredGrid2D::ConstCoordItrU2D::next()
{
    ++_xCoordItr;
    _coords[0] = *_xCoordItr;

    ++_yCoordItr;
    _coords[1] = *_yCoordItr;
    if (_ncoords < 3) return;

    ++_zCoordItr;
    _coords[2] = *_zCoordItr;
}

void UnstructuredGrid2D::ConstCoordItrU2D::next(const long &offset)
{
    _xCoordItr += offset;
    _yCoordItr += offset;

    _coords[0] = *_xCoordItr;
    _coords[1] = *_yCoordItr;

    if (_ncoords < 3) return;

    _zCoordItr += offset;
    _coords[2] = *_zCoordItr;
}

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const UnstructuredGrid2D &ug)
{
    o << "UnstructuredGrid2D " << endl;
    o << (const Grid &)ug;

    return o;
}
};    // namespace VAPoR

// Search for a point inside the grid. If the point is inside return true,
// and provide the Wachspress weights/coordinates for the point within
// the face containing the point in XY, and the linear
// interpolation weights/coordinates along Z.
//
bool UnstructuredGrid2D::_insideGrid(const vector<double> &coords, size_t &face, vector<size_t> &nodes, double *lambda, int &nlambda) const
{
    nodes.clear();

    if (_location == NODE) {
        return (_insideGridNodeCentered(coords, face, nodes, lambda, nlambda));
    } else {
        return (_insideGridFaceCentered(coords, face, nodes, lambda, nlambda));
    }
}

bool UnstructuredGrid2D::_insideGridFaceCentered(const vector<double> &coords, size_t &face, vector<size_t> &nodes, double *lambda, int &nlambda) const
{
    assert(0 && "Not supported");
    return false;
}

bool UnstructuredGrid2D::_insideGridNodeCentered(const vector<double> &coords, size_t &face_index, vector<size_t> &nodes, double *lambda, int &nlambda) const
{
    nodes.clear();

    assert(coords.size() == 2);

    double pt[] = {coords[0], coords[1]};

    // Find the indices for the nearest grid point in the plane
    //
    vector<size_t> vertex_indices;
    _kdtree->Nearest(coords, vertex_indices);
    assert(vertex_indices.size() == 1);

    vector<size_t> dims = GetDimensions();
    assert(vertex_indices[0] < dims[0]);

    const int *ptr = _faceOnVertex + (vertex_indices[0] * _maxFacePerVertex);
    long       offset = GetCellOffset();

    for (int i = 0; i < _maxFacePerVertex; i++) {
        if (*ptr == GetMissingID()) break;
        if (*ptr == GetBoundaryID()) continue;

        long face = *ptr + offset;
        if (face < 0) break;

        if (_insideFace(face, pt, nodes, lambda, nlambda)) {
            face_index = face;
            return (true);
        }
        ptr++;
    }

    return (false);
}

bool UnstructuredGrid2D::_insideFace(size_t face, double pt[2], vector<size_t> &node_indices, double *lambda, int &nlambda) const
{
    node_indices.clear();
    nlambda = 0;

    double *verts = new double[_maxVertexPerFace * 2];

    const int *ptr = _vertexOnFace + (face * _maxVertexPerFace);
    long       offset = GetNodeOffset();

    for (int i = 0; i < _maxVertexPerFace; i++) {
        if (*ptr == GetMissingID()) break;

        long vertex = *ptr + offset;
        if (vertex < 0) break;

        verts[i * 2 + 0] = _xug.AccessIJK(vertex, 0, 0);
        verts[i * 2 + 1] = _yug.AccessIJK(vertex, 0, 0);
        node_indices.push_back(*ptr);
        ptr++;
        nlambda++;
    }

    // Should we test the line case where nlambda == 2?
    //
    if (nlambda < 3) {
        delete[] verts;
        return (false);
    }

    bool ret = WachspressCoords2D(verts, pt, nlambda, lambda);

    delete[] verts;

    return ret;
}
