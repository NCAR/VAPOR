#ifndef _UnstructuredGrid_
#define _UnstructuredGrid_

#include <ostream>
#include <vector>
#include <memory>
#include <vapor/common.h>
#include <vapor/Grid.h>

#ifdef WIN32
#pragma warning(disable : 4661 4251) // needed for template class
#endif

namespace VAPoR {

//! \class UnstructuredGrid
//! \brief Abstract base class for a 2D or 3D unstructured grid.
//! \author John Clyne
//!
//! This abstract base class defines a 2D or 3D unstructured
//! grid.
//!
//! The unstructured grid samples a scalar function at each grid point. The
//! scalar function samples are stored as an array decomposed into
//! equal-sized blocks.
//!
//! Because grid samples are repesented internally as arrays, when accessing
//! multiple grid points better performance is achieved by using
//! unit stride. The \a I axis varies fastest (has unit stride),
//! followed by \a J, then \a K. Best performance is achieved
//! when using the class iterator: UnstructuredGrid::Iterator.
//!
//! For methods that allow the specification of grid indecies or coordinates
//! as a single parameter tuple (e.g. vector <double> coordinate) the
//! first element
//! of the tuple corresponds to the \a I axis, etc.
//!
//! \note Throughout this class grid vertex offsets are specified as
//! \a i, \a j, \a k, where \a i, \a j, \a k are integers. User coordinates
//! are real values denoted \a x, \a y, \a z, and are given by functions
//! \a X(i,j,k), \a Y(i,j,k), \a Z(i,j,k).
//
class VDF_API UnstructuredGrid : public Grid {
  public:
    enum Location { NODE,
                    CELL,
                    EDGE };

    //!
    //! Construct a unstructured grid sampling a 3D or 2D scalar function
    //!
    //! \param[in] vertexDims Dimensions of grid nodes (vertices).
    //! The product of the
    //! elements of \p vertexDims gives the total number of nodes in the mesh.
    //! If the rank of \p vertexDims is greater than one the mesh is assumed to
    //! be structured along the slower varying dimensions (e.g. a layered mesh)
    //!
    //! \param[in] faceDims Dimensions of grid cells. The product of the
    //! elements of \p cells gives the total number of cells in the mesh.
    //! If the rank of \p cells is greater than one the mesh is assumed to
    //! be structured along the slower varying dimensions (e.g. a layered mesh).
    //! Moreover, the slowest varying dimensions must be one less than the
    //! corresponding dimension of \p vertexDims.
    //!
    //! \param[in] edgeDims Dimensions of grid edges. The product of the
    //! elements of \p edges gives the total number of edges in the mesh.
    //! If the rank of \p edges is greater than one the mesh is assumed to
    //! be structured along the slower varying dimensions (e.g. a layered mesh).
    //! Moreover, the slowest varying dimensions must be one less than the
    //! corresponding dimension of \p vertexDims.
    //!
    //! \param[in] blks Grid data values. The location of the data values
    //! (node, cell, edge) is determined by \p location. If the dimensions
    //! (\p vertexDims, etc) are multi-dimensional the size of \p blks must
    //! match the size of the slowest varying dimension. Each element of
    //! \p blks must point to an area of memory of size \b n elements, where
    //! \b n is the first element of \p vertexDims, \p faceDims, or \p edgeDims
    //! as indicated by \p location.
    //!
    //! \param[in] vertexOnFace An array with dimensions:
    //!
    //! \p faceDims[0] * maxVertexPerFace
    //! that provides for each cell the 1D node IDs of each corner node.
    //! If the number of corner nodes is less than \p maxVertexPerFace
    //! the missing node indices will be equal to GetMissingID();
    //! The ordering of the nodes is counter-clockwise.
    //!
    //! \param[in] faceOnVertex An array with dimensions:
    //!
    //! \p vertexDims[0] * maxFacePerVertex
    //! that provides for each vertex the 1D node IDs of each face
    //! sharing that vertex.
    //! If the number of faces is less than \p maxFacePerVertex
    //! the missing face indices will be equal to GetMissingID();
    //! The ordering of the faces is counter-clockwise.
    //!
    //! \param[in] faceOnFace An array with dimensions:
    //!
    //! \p faceDims[0] * maxVertexPerFace
    //! that provides for each cell the 1D cell IDs of border cell
    //! If the number of corner nodes is less than \p maxVertexPerFace
    //! the missing node indices will be equal to GetMissingID(). If an
    //! edge is on the mesh boundary the index will be set to the value
    //! GetBoundaryIndex().
    //! The ordering of the neighboring faces is counter-clockwise.
    //!
    //! \param[in] location The location of grid data: at the nodes, edge-centered,
    //! or cell-centered
    //!
    //! \param[in] maxVertexPerFace The maxium number of nodes that a face
    //! may have.
    //
    UnstructuredGrid(
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
        size_t maxFacePerVertex

    );

    UnstructuredGrid() = default;
    virtual ~UnstructuredGrid() = default;

    //! \copydoc Grid::GetCellNodes()
    //!
    virtual bool GetCellNodes(
        const std::vector<size_t> &cindices,
        std::vector<std::vector<size_t>> &nodes) const override;

    //! \copydoc Grid::GetCellNeighbors()
    //!
    virtual bool GetCellNeighbors(
        const std::vector<size_t> &cindices,
        std::vector<std::vector<size_t>> &cells) const override;

    //! \copydoc Grid::GetNodeCells()
    //!
    virtual bool GetNodeCells(
        const std::vector<size_t> &indices,
        std::vector<std::vector<size_t>> &cells) const override;

    //! Return the grid node dimmensions
    //!
    const std::vector<size_t> &GetNodeDimensions() const override {
        return (_vertexDims);
    }

    //! Return the grid cell dimmensions
    //!
    const std::vector<size_t> &GetCellDimensions() const override {
        return (_faceDims);
    }

    //! Return the grid edge dimmensions
    //!
    const std::vector<size_t> &GetEdgeDimensions() const {
        return (_edgeDims);
    }

    //! Get missing element ID
    //!
    //! Return the value used to indicate termination of a list of element IDs
    //!
    size_t GetMissingID() const {
        return (_missingID);
    }
    void SetMissingID(size_t v) {
        _missingID = v;
    }

    //! Get boundary element ID
    //!
    //! Return the value used to indicate termination of a list of element IDs
    //!
    size_t GetBoundaryID() const {
        return (_boundaryID);
    }
    void SetBoundaryID(size_t v) {
        _boundaryID = v;
    }

    virtual void ClampCoord(std::vector<double> &coords) const override {
        assert(coords.size() >= GetGeometryDim());
        while (coords.size() > GetGeometryDim()) {
            coords.pop_back();
        }
    }

    /////////////////////////////////////////////////////////////////////////////
    //
    // Iterators
    //
    /////////////////////////////////////////////////////////////////////////////

    VDF_API friend std::ostream &operator<<(std::ostream &o, const UnstructuredGrid &sg);

  protected:
    const int *_vertexOnFace;
    const int *_faceOnVertex;
    const int *_faceOnFace;
    size_t _maxVertexPerFace;
    size_t _maxFacePerVertex;
    Location _location;

  private:
    std::vector<size_t> _vertexDims;
    std::vector<size_t> _faceDims;
    std::vector<size_t> _edgeDims;
    int _missingID;
    int _boundaryID;
};
}; // namespace VAPoR

#endif
