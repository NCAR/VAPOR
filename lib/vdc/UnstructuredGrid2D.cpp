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
#include <vapor/UnstructuredGrid2D.h>

using namespace std;
using namespace VAPoR;

UnstructuredGrid2D::UnstructuredGrid2D(const std::vector<size_t> &vertexDims, const std::vector<size_t> &faceDims, const std::vector<size_t> &edgeDims, const std::vector<size_t> &bs,
                                       const std::vector<float *> &blks, const int *vertexOnFace, const int *faceOnVertex, const int *faceOnFace,
                                       Location location,    // node,face, edge
                                       size_t maxVertexPerFace, size_t maxFacePerVertex, long nodeOffset, long cellOffset, const UnstructuredGridCoordless &xug, const UnstructuredGridCoordless &yug,
                                       const UnstructuredGridCoordless &zug, std::shared_ptr<const QuadTreeRectangle<float, size_t>> qtr)
: UnstructuredGrid(vertexDims, faceDims, edgeDims, bs, blks, 2, vertexOnFace, faceOnVertex, faceOnFace, location, maxVertexPerFace, maxFacePerVertex, nodeOffset, cellOffset), _xug(xug), _yug(yug),
  _zug(zug), _qtr(qtr)
{
    VAssert(xug.GetDimensions() == GetDimensions());
    VAssert(yug.GetDimensions() == GetDimensions());
    VAssert(zug.GetDimensions() == GetDimensions() || zug.GetDimensions().size() == 0);

    VAssert(location == NODE);

    if (!_qtr) { _qtr = _makeQuadTreeRectangle(); }
}

vector<size_t> UnstructuredGrid2D::GetCoordDimensions(size_t dim) const
{
    if (dim == 0) {
        return (_xug.GetDimensions());
    } else if (dim == 1) {
        return (_yug.GetDimensions());
    } else if (dim == 2) {
        if (GetGeometryDim() == 3) {
            return (_zug.GetDimensions());
        } else {
            return (vector<size_t>(1, 1));
        }
    } else {
        return (vector<size_t>(1, 1));
    }
}

size_t UnstructuredGrid2D::GetGeometryDim() const { return (_zug.GetDimensions().size() == 0 ? 2 : 3); }

void UnstructuredGrid2D::GetUserExtentsHelper(DblArr3 &minu, DblArr3 &maxu) const
{
    float range[2];

    _xug.GetRange(range);
    minu[0] = range[0];
    maxu[0] = range[1];

    _yug.GetRange(range);
    minu[1] = range[0];
    maxu[1] = range[1];

    if (GetGeometryDim() < 3) return;

    _zug.GetRange(range);
    minu[2] = range[0];
    maxu[2] = range[1];
}

void UnstructuredGrid2D::GetBoundingBox(const Size_tArr3 &min, const Size_tArr3 &max, DblArr3 &minu, DblArr3 &maxu) const
{
    Size_tArr3 cMin;
    ClampIndex(min, cMin);

    Size_tArr3 cMax;
    ClampIndex(max, cMax);

    int ncoords = GetGeometryDim();
    minu = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
    minu = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest()};

    size_t start = Wasp::LinearizeCoords(cMin.data(), GetDimensions().data(), GetDimensions().size());
    size_t stop = Wasp::LinearizeCoords(cMax.data(), GetDimensions().data(), GetDimensions().size());

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

bool UnstructuredGrid2D::GetEnclosingRegion(const DblArr3 &minu, const DblArr3 &maxu, Size_tArr3 &min, Size_tArr3 &max) const
{
    VAssert(0 && "Not implemented");
    return (true);
}

void UnstructuredGrid2D::GetUserCoordinates(const Size_tArr3 &indices, DblArr3 &coords) const
{
    Size_tArr3 cIndices;
    ClampIndex(indices, cIndices);

    coords[0] = _xug.GetValueAtIndex(cIndices);
    coords[1] = _yug.GetValueAtIndex(cIndices);
    if (GetGeometryDim() == 3) { coords[2] = _zug.GetValueAtIndex(cIndices); }
}

bool UnstructuredGrid2D::GetIndicesCell(const DblArr3 &coords, Size_tArr3 &cindices, std::vector<std::vector<size_t>> &nodes, std::vector<double> &lambdav) const
{
    nodes.clear();
    lambdav.clear();

    DblArr3 cCoords;
    ClampCoord(coords, cCoords);

    double *lambda = new double[_maxVertexPerFace];
    int     nlambda;

    // See if point is inside any cells (faces)
    //
    size_t              my_index;
    std::vector<size_t> my_nodes;
    bool                status = _insideGridNodeCentered(cCoords, my_index, my_nodes, lambda, nlambda);

    if (status) {
        cindices[0] = my_index;
        for (int i = 0; i < nlambda; i++) {
            lambdav.push_back(lambda[i]);
            nodes.push_back(vector<size_t>(1, my_nodes[i]));
        }
    }

    delete[] lambda;

    return (status);
}

bool UnstructuredGrid2D::InsideGrid(const DblArr3 &coords) const
{
    DblArr3 cCoords;
    ClampCoord(coords, cCoords);

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

float UnstructuredGrid2D::GetValueNearestNeighbor(const DblArr3 &coords) const
{
    // Clamp coordinates on periodic boundaries to reside within the
    // grid extents
    //
    DblArr3 cCoords;
    ClampCoord(coords, cCoords);

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
    VAssert(face < GetCellDimensions()[0]);

    const int *ptr = _vertexOnFace + (face * _maxVertexPerFace);

    int maxindx = lambda[0];
    for (int i = 1; i < nlambda; i++) {
        if (lambda[i] > lambda[maxindx]) maxindx = i;
    }

    long  offset = GetNodeOffset();
    float value = AccessIJK(*ptr + maxindx + offset);

    delete[] lambda;

    return ((float)value);
}

float UnstructuredGrid2D::GetValueLinear(const DblArr3 &coords) const
{
    // Clamp coordinates on periodic boundaries to reside within the
    // grid extents
    //
    DblArr3 cCoords;
    ClampCoord(coords, cCoords);

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

    VAssert(nodes.size() == nlambda);
    VAssert(face < GetCellDimensions()[0]);

    double value = 0;
    for (int i = 0; i < nodes.size(); i++) { value += AccessIJK(nodes[i], 0, 0) * lambda[i]; }

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
        _coords[0] = *_xCoordItr;
        _coords[1] = *_yCoordItr;
        if (_ncoords >= 3) { _coords[2] = *_zCoordItr; }
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
bool UnstructuredGrid2D::_insideGrid(const DblArr3 &coords, size_t &face, vector<size_t> &nodes, double *lambda, int &nlambda) const
{
    nodes.clear();

    if (_location == NODE) {
        return (_insideGridNodeCentered(coords, face, nodes, lambda, nlambda));
    } else {
        return (_insideGridFaceCentered(coords, face, nodes, lambda, nlambda));
    }
}

bool UnstructuredGrid2D::_insideGridFaceCentered(const DblArr3 &coords, size_t &face, vector<size_t> &nodes, double *lambda, int &nlambda) const
{
    VAssert(0 && "Not supported");
    return false;
}

bool UnstructuredGrid2D::_insideGridNodeCentered(const DblArr3 &coords, size_t &face_index, vector<size_t> &nodes, double *lambda, int &nlambda) const
{
    nodes.clear();

    double pt[] = {coords[0], coords[1]};

    // Find the indices for the faces that might contain the point
    //
    vector<size_t> face_indices;
    _qtr->GetPayloadContained(pt[0], pt[1], face_indices);

    for (int i = 0; i < face_indices.size(); i++) {
        if (_insideFace(face_indices[i], pt, nodes, lambda, nlambda)) {
            face_index = face_indices[i];
            return (true);
        }
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
        node_indices.push_back(*ptr + offset);
        ptr++;
        nlambda++;
    }

    // Should we test the line case where nlambda == 2?
    //
    if (nlambda < 3) {
        delete[] verts;
        return (false);
    }

    if (!Grid::PointInsideBoundingRectangle(pt, verts, nlambda)) {
        delete[] verts;
        return (false);
    }

    bool ret = WachspressCoords2D(verts, pt, nlambda, lambda);

    delete[] verts;

    return ret;
}

std::shared_ptr<QuadTreeRectangle<float, size_t>> UnstructuredGrid2D::_makeQuadTreeRectangle() const
{
    size_t             maxNodes = GetMaxVertexPerCell();
    vector<Size_tArr3> nodes(maxNodes);

    size_t coordDim = GetGeometryDim();
    VAssert(coordDim == 2);

    double minu[3], maxu[3];
    GetUserExtents(minu, maxu);

    const vector<size_t> &dims = GetDimensions();
    size_t                reserve_size = dims[0];

    std::shared_ptr<QuadTreeRectangle<float, size_t>> qtr = std::make_shared<QuadTreeRectangle<float, size_t>>((float)minu[0], (float)minu[1], (float)maxu[0], (float)maxu[1], 12, reserve_size);

    DblArr3                 coords;
    Grid::ConstCellIterator it = ConstCellBegin();
    Grid::ConstCellIterator end = ConstCellEnd();
    for (; it != end; ++it) {
        const vector<size_t> &cell = *it;
        VAssert(cell.size() == 1);
        GetCellNodes(Size_tArr3{cell[0], 0, 0}, nodes);
        if (nodes.size() < 2) continue;

        GetUserCoordinates(nodes[0], coords);
        float left = (float)coords[0];
        float right = (float)coords[0];
        float top = (float)coords[1];
        float bottom = (float)coords[1];
        for (int i = 1; i < nodes.size(); i++) {
            GetUserCoordinates(nodes[i], coords);
            if (coords[0] < left) left = coords[0];
            if (coords[0] > right) right = coords[0];
            if (coords[1] < top) top = coords[1];
            if (coords[1] > bottom) bottom = coords[1];
        }
        qtr->Insert(left, top, right, bottom, cell[0]);
    }

    return (qtr);
}
