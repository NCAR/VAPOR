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
#include <vapor/UnstructuredGrid.h>

using namespace std;
using namespace VAPoR;

UnstructuredGrid::UnstructuredGrid(
    const std::vector<size_t> &vertexDims,
    const std::vector<size_t> &faceDims,
    const std::vector<size_t> &edgeDims,
    const std::vector<size_t> &bs,
    const std::vector<float *> &blks,
    size_t topology_dimension,
    const int *vertexOnFace,
    const int *faceOnVertex,
    const int *faceOnFace,
    Location location, // node,face, edge
    size_t maxVertexPerFace,
    size_t maxFacePerVertex) : Grid(location == NODE ? vertexDims : (location == CELL ? faceDims : edgeDims),
                                    bs, blks, topology_dimension) {

    assert(vertexDims.size() == 1 || vertexDims.size() == 2);
    assert(vertexDims.size() == faceDims.size());
    assert(vertexDims.size() == edgeDims.size() || edgeDims.size() == 0);

    // Edge data not supported yet
    //
    assert(location == NODE || location == CELL);

    _vertexDims = vertexDims;
    _faceDims = faceDims;
    _edgeDims = edgeDims;

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
}

bool UnstructuredGrid::GetCellNodes(
    const std::vector<size_t> &cindices,
    std::vector<vector<size_t>> &nodes) const {
    nodes.clear();

    vector<size_t> cdims = GetCellDimensions();
    assert(cindices.size() == cdims.size());

    // Check if invalid indices
    //
    for (int i = 0; i < cindices.size(); i++) {
        if (cindices[i] > (cdims[i] - 1))
            return (false);
    }

    // _vertexOnFace is dimensioned cdims[0] x _maxVertexPerFace
    //
    const int *ptr = _vertexOnFace + (_maxVertexPerFace * cindices[0]);
    long offset = GetNodeOffset();

    if (cdims.size() == 1) {
        for (int i = 0; i < _maxVertexPerFace; i++, ptr++) {
            vector<size_t> indices;
            if (*ptr == GetMissingID() || *ptr + offset < 0)
                break;
            if (*ptr == GetBoundaryID())
                continue;

            indices.push_back(*ptr + offset);
            nodes.push_back(indices);
        }
    } else { // layered case

        for (int i = 0; i < _maxVertexPerFace; i++, ptr++) {
            vector<size_t> indices;
            if (*ptr == GetMissingID() || *ptr + offset < 0)
                break;
            if (*ptr == GetBoundaryID())
                continue;

            indices.push_back(*ptr + offset);
            indices.push_back(cindices[1]);
            nodes.push_back(indices);
        }

        ptr = _vertexOnFace + (_maxVertexPerFace * cindices[0]);
        for (int i = 0; i < _maxVertexPerFace; i++) {
            vector<size_t> indices;
            if (*ptr == GetMissingID() || *ptr + offset < 0)
                break;
            if (*ptr == GetBoundaryID())
                continue;

            indices.push_back(*ptr + offset);
            indices.push_back(cindices[1] + 1);
            nodes.push_back(indices);
        }
    }
    return (true);
}

bool UnstructuredGrid::GetCellNeighbors(
    const std::vector<size_t> &cindices,
    std::vector<vector<size_t>> &cells) const {
    cells.clear();

    vector<size_t> cdims = GetCellDimensions();
    assert(cindices.size() == cdims.size());

    // Check if invalid indices
    //
    for (int i = 0; i < cindices.size(); i++) {
        if (cindices[i] > (cdims[i] - 1))
            return (false);
    }

    // _faceOnFace is dimensioned cdims[0] x _maxVertexPerFace
    //
    const int *ptr = _faceOnFace + (_maxVertexPerFace * cindices[0]);
    long offset = GetCellOffset();

    if (cdims.size() == 1) {
        for (int i = 0; i < _maxVertexPerFace; i++) {
            vector<size_t> indices;
            if (*ptr == GetMissingID() || *ptr + offset < 0)
                break;

            if (*ptr != GetBoundaryID()) {
                indices.push_back(*ptr + offset);
                indices.push_back(cindices[1]);
            }
            cells.push_back(indices);
        }
    } else { // layered case

        for (int i = 0; i < _maxVertexPerFace; i++) {
            vector<size_t> indices;
            if (*ptr == GetMissingID() || *ptr + offset < 0)
                break;

            if (*ptr != GetBoundaryID()) {
                indices.push_back(*ptr + offset);
                indices.push_back(cindices[1]);
            }

            cells.push_back(indices);
        }

        // layer below
        //
        if (cindices[1] != 0) {
            vector<size_t> indices = cindices;
            indices[1] = cindices[1] - 1;
            cells.push_back(indices);
        } else {
            cells.push_back(vector<size_t>());
        }

        // layer above
        //
        if (cindices[1] != cdims[1] - 1) {
            vector<size_t> indices = cindices;
            indices[1] = indices[1] + 1;
            cells.push_back(indices);
        } else {
            cells.push_back(vector<size_t>());
        }
    }

    return (true);
}

bool UnstructuredGrid::GetNodeCells(
    const std::vector<size_t> &indices,
    std::vector<vector<size_t>> &cells) const {
    cells.clear();

    vector<size_t> dims = GetDimensions();
    assert(indices.size() == dims.size());

    assert(0 && "GetNodeCells() Not supported");
}

/////////////////////////////////////////////////////////////////////////////
//
// Iterators
//
/////////////////////////////////////////////////////////////////////////////

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const UnstructuredGrid &ug) {
    o << "UnstructuredGrid " << endl;
    o << " Node dimensions ";
    for (int i = 0; i < ug._vertexDims.size(); i++) {
        o << ug._vertexDims[i] << " ";
    }
    o << endl;

    o << " Cell dimensions ";
    for (int i = 0; i < ug._faceDims.size(); i++) {
        o << ug._faceDims[i] << " ";
    }
    o << endl;

    o << " Edge dimensions ";
    for (int i = 0; i < ug._edgeDims.size(); i++) {
        o << ug._edgeDims[i] << " ";
    }
    o << endl;

    o << " Max nodes per face " << ug._maxVertexPerFace << endl;
    o << " Max face per node " << ug._maxFacePerVertex << endl;
    o << " Sample location" << ug._location << endl;
    o << " Missing ID" << ug._missingID << endl;
    o << " Boundary ID" << ug._boundaryID << endl;

    o << (const Grid &)ug;

    return o;
}
}; // namespace VAPoR
