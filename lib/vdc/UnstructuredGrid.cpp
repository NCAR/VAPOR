#include <iostream>
#include <vector>
#include "vapor/VAssert.h"
#include <cmath>
#include <cassert>
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
#include <vapor/UnstructuredGrid.h>

using namespace std;
using namespace VAPoR;

void UnstructuredGrid::_unstructuredGrid(const DimsType &vertexDims, const DimsType &faceDims, const DimsType &edgeDims, const DimsType &bs,
                                   const std::vector<float *> &blks, size_t topology_dimension, const int *vertexOnFace, const int *faceOnVertex, const int *faceOnFace,
                                   Location location,    // node,face, edge
                                   size_t maxVertexPerFace, size_t maxFacePerVertex, long nodeOffset, long cellOffset)
{
    VAssert(GetNumDimensions(vertexDims) == 1 || GetNumDimensions(vertexDims) == 2);
    VAssert(GetNumDimensions(vertexDims) == GetNumDimensions(faceDims));
    VAssert((GetNumDimensions(vertexDims) == GetNumDimensions(edgeDims)) || (GetNumDimensions(edgeDims) == 0));

    // Edge data not supported yet
    //
    VAssert(location == NODE || location == CELL);

    _vertexDims = vertexDims;
    _faceDims = faceDims;
    _edgeDims = edgeDims;
    _nDims = GetNumDimensions(vertexDims);

    //
    // Shallow copy raw pointers
    //
    _vertexOnFace = vertexOnFace;
    _faceOnVertex = faceOnVertex;
    _faceOnFace = faceOnFace;

    _location = location;
    _maxVertexPerFace = maxVertexPerFace;
    _maxFacePerVertex = maxFacePerVertex;
    _missingID = -1;
    _boundaryID = -2;

    Grid::SetNodeOffset(nodeOffset);
    Grid::SetCellOffset(cellOffset);
}

UnstructuredGrid::UnstructuredGrid(const DimsType &vertexDims, const DimsType &faceDims, const DimsType &edgeDims, const DimsType &bs,
                                   const std::vector<float *> &blks, size_t topology_dimension, const int *vertexOnFace, const int *faceOnVertex, const int *faceOnFace,
                                   Location location,    // node,face, edge
                                   size_t maxVertexPerFace, size_t maxFacePerVertex, long nodeOffset, long cellOffset)
: Grid(location == NODE ? vertexDims : (location == CELL ? faceDims : edgeDims), bs, blks, topology_dimension)
{
    _unstructuredGrid(vertexDims, faceDims, edgeDims, bs, blks, topology_dimension, vertexOnFace, faceOnVertex, faceOnFace, location, maxVertexPerFace, maxFacePerVertex, nodeOffset, cellOffset);
}

UnstructuredGrid::UnstructuredGrid(const std::vector<size_t> &vertexDimsv, const std::vector<size_t> &faceDimsv, const std::vector<size_t> &edgeDimsv, const std::vector<size_t> &bsv,
                                   const std::vector<float *> &blks, size_t topology_dimension, const int *vertexOnFace, const int *faceOnVertex, const int *faceOnFace,
                                   Location location,    // node,face, edge
                                   size_t maxVertexPerFace, size_t maxFacePerVertex, long nodeOffset, long cellOffset)
: Grid(location == NODE ? vertexDimsv : (location == CELL ? faceDimsv : edgeDimsv), bsv, blks, topology_dimension)
{
    DimsType vertexDims = {1,1,1};
    DimsType faceDims = {1,1,1};
    DimsType edgeDims = {1,1,1};
    DimsType bs = {1,1,1};
    CopyToArr3(vertexDimsv, vertexDims);
    CopyToArr3(faceDimsv, faceDims);
    CopyToArr3(edgeDimsv, edgeDims);
    CopyToArr3(bsv, bs);

    _unstructuredGrid(vertexDims, faceDims, edgeDims, bs, blks, topology_dimension, vertexOnFace, faceOnVertex, faceOnFace, location, maxVertexPerFace, maxFacePerVertex, nodeOffset, cellOffset);
}


const VAPoR::DimsType &UnstructuredGrid::GetNodeDimensions() const { return (_vertexDims); }


const size_t UnstructuredGrid::GetNumNodeDimensions() const { return _nDims; }


bool UnstructuredGrid::GetCellNodes(const DimsType &cindices, vector<DimsType> &nodes) const
{
    DimsType cCindices;
    ClampCellIndex(cindices, cCindices);

    // _vertexOnFace is dimensioned cdims[0] x _maxVertexPerFace
    //
    const int *ptr = _vertexOnFace + (_maxVertexPerFace * cCindices[0]);
    long       offset = GetNodeOffset();

    size_t n = 0;
    if (GetNumCellDimensions() == 1) {
        nodes.resize(_maxVertexPerFace);    // ensure sufficient memory
        for (int i = 0; i < _maxVertexPerFace; i++, ptr++) {
            if (*ptr == GetMissingID() || *ptr + offset < 0) break;
            if (*ptr == GetBoundaryID()) continue;

            nodes[n][0] = *ptr + offset;
            nodes[n][1] = 0;
            nodes[n][2] = 0;
            n++;
        }
    } else {                                    // layered case
        nodes.resize(2 * _maxVertexPerFace);    // ensure sufficient memory

        // Bottom layer
        //
        for (int i = 0; i < _maxVertexPerFace; i++, ptr++) {
            if (*ptr == GetMissingID() || *ptr + offset < 0) break;
            if (*ptr == GetBoundaryID()) continue;

            nodes[n][0] = *ptr + offset;
            nodes[n][1] = cCindices[1];
            n++;
        }

        // Top layer is identical to bottom layer accept for the
        // slowest varying index (the layer index)
        //
        int nNodesPerLayer = n;
        for (int i = 0; i < nNodesPerLayer; i++) {
            nodes[n][0] = nodes[i][0];
            nodes[n][1] = nodes[i][1] + 1;
            n++;
        }
    }
    nodes.resize(n);    // resize to actual count
    return (true);
}

bool UnstructuredGrid::GetCellNeighbors(const DimsType &cindices, std::vector<DimsType> &cells) const
{
    cells.clear();

    DimsType cCindices = {0, 0, 0};
    ClampCellIndex(cindices, cCindices);

    const DimsType &cdims = GetCellDimensions();

    // _faceOnFace is dimensioned cdims[0] x _maxVertexPerFace
    //
    const int *ptr = _faceOnFace + (_maxVertexPerFace * cCindices[0]);
    long       offset = GetCellOffset();

    DimsType indices = {0, 0, 0};
    if (GetNumCellDimensions() == 1) {
        for (int i = 0; i < _maxVertexPerFace; i++) {
            if (*ptr == GetMissingID() || *ptr + offset < 0) break;

            if (*ptr != GetBoundaryID()) { indices[0] = *ptr + offset; }
            cells.push_back(indices);
        }
    } else {    // layered case

        for (int i = 0; i < _maxVertexPerFace; i++) {
            if (*ptr == GetMissingID() || *ptr + offset < 0) break;

            if (*ptr != GetBoundaryID()) {
                indices[0] = *ptr + offset;
                indices[1] = cCindices[1];
            }

            cells.push_back(indices);
        }

        // layer below
        //
        if (cCindices[1] != 0) {
            DimsType indices = {cCindices[0], cCindices[1], 0};
            indices[1] = cCindices[1] - 1;
            cells.push_back(indices);
        }

        // layer above
        //
        if (cCindices[1] != cdims[1] - 1) {
            DimsType indices = {cCindices[0], cCindices[1], 0};
            indices[1] = indices[1] + 1;
            cells.push_back(indices);
        }
    }

    return (true);
}

bool UnstructuredGrid::GetNodeCells(const DimsType &indices, std::vector<DimsType> &cells) const
{
    cells.clear();

    VAssert(indices.size() == GetNumDimensions());

    VAssert(0 && "GetNodeCells() Not supported");
    return false;
}

/////////////////////////////////////////////////////////////////////////////
//
// Iterators
//
/////////////////////////////////////////////////////////////////////////////

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const UnstructuredGrid &ug)
{
    o << "UnstructuredGrid " << endl;
    o << " Node dimensions ";
    for (int i = 0; i < ug._vertexDims.size(); i++) { o << ug._vertexDims[i] << " "; }
    o << endl;

    o << " Cell dimensions ";
    for (int i = 0; i < ug._faceDims.size(); i++) { o << ug._faceDims[i] << " "; }
    o << endl;

    o << " Edge dimensions ";
    for (int i = 0; i < ug._edgeDims.size(); i++) { o << ug._edgeDims[i] << " "; }
    o << endl;

    o << " Max nodes per face " << ug._maxVertexPerFace << endl;
    o << " Max face per node " << ug._maxFacePerVertex << endl;
    o << " Sample location" << ug._location << endl;
    o << " Missing ID" << ug._missingID << endl;
    o << " Boundary ID" << ug._boundaryID << endl;

    o << (const Grid &)ug;

    return o;
}
};    // namespace VAPoR
