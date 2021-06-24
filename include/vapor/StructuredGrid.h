#ifndef _StructuredGrid_
#define _StructuredGrid_

#include <ostream>
#include <vector>
#include <memory>
#include <vapor/common.h>
#include <vapor/Grid.h>

#ifdef WIN32
    #pragma warning(disable : 4661 4251)    // needed for template class
#endif

namespace VAPoR {

//! \class StructuredGrid
//! \brief Abstract base class for a 2D or 3D structured grid.
//! \author John Clyne
//!
//! This abstract base class defines a 2D or 3D structured
//! grid: a tessellation
//! of Euculidean space by quadrilaterals (2D) or hexahdrons (3D). Each
//! grid point can be addressed by an index(i,j,k), where \a i, \p a
//! and \a k range from 0 to \a dim - 1, where \a dim is the dimension of the
//! \a I, \a J, or \a K axis, respectively. Moreover, each grid point
//! has a coordinate in a user-defined coordinate system.
//!
//! The structured grid samples a scalar function at each grid point. The
//! scalar function samples are stored as an array decomposed into
//! equal-sized blocks.
//!
//! Because grid samples are repesented internally as arrays, when accessing
//! multiple grid points better performance is achieved by using
//! unit stride. The \a I axis varies fastest (has unit stride),
//! followed by \a J, then \a K. Best performance is achieved
//! when using the class iterator: Grid::Iterator.
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
class VDF_API StructuredGrid : public Grid {
public:
    //! Construct a structured grid sampling a 3D or 2D scalar function
    //!
    //! \copydoc Grid()
    //!
    //! The sampled function is represented as a 2D or 3D array, decomposed
    //! into smaller blocks (tiles in 2D). The dimensions of the array are not
    //! constrained to coincide with block (tile) boundaries.
    //!
    //! The length of parameter vectors \p bs, and \p dim
    //! must all be either 3 (3D structured grid) or 2 (2D structured grid)
    //!
    //! If \p blks is empty a dataless StructuredGrid object is returned.
    //! Data can not be retrieved from a dataless StructuredGrid. However,
    //! coordinate access methods may still be invoked.
    //!
    StructuredGrid(const std::vector<size_t> &dims, const std::vector<size_t> &bs, const std::vector<float *> &blks);

    StructuredGrid() = default;
    virtual ~StructuredGrid() = default;

    static std::string GetClassType() { return ("Structured"); }
    std::string        GetType() const override { return (GetClassType()); }

    const std::vector<size_t> &GetNodeDimensions() const override
    {
        auto tmp = GetDimensions();
        _duplicate = {tmp[0], tmp[1], tmp[2]};
        _duplicate.resize(this->GetNumDimensions());
        return _duplicate;
    }

    const std::vector<size_t> &GetCellDimensions() const override { return (_cellDims); };

    //! \copydoc Grid::GetCellNodes()
    //!
    virtual bool GetCellNodes(const Size_tArr3 &cindices, std::vector<Size_tArr3> &nodes) const override;
    // For grandparent inheritance of
    // Grid::GetUserCoordinates(const size_t indices[], double coords[])
    //
    using Grid::GetCellNodes;

    //! \copydoc Grid::GetCellNeighbors()
    //!
    virtual bool GetCellNeighbors(const Size_tArr3 &cindices, std::vector<Size_tArr3> &cells) const override;

    //! \copydoc Grid::GetNodeCells()
    //!
    virtual bool GetNodeCells(const Size_tArr3 &cindices, std::vector<Size_tArr3> &cells) const override;

    virtual bool GetEnclosingRegion(const DblArr3 &minu, const DblArr3 &maxu, Size_tArr3 &min, Size_tArr3 &max) const override;

    size_t GetMaxVertexPerFace() const override { return (4); };

    size_t GetMaxVertexPerCell() const override { return ((GetTopologyDim() == 3) ? 8 : 4); };

    virtual void ClampCoord(const DblArr3 &coords, DblArr3 &cCoords) const override;

    //! \deprecated
    //
    virtual void ClampCoord(const double coords[3], double cCoords[3]) const override { Grid::ClampCoord(coords, cCoords); }

    //! \copydoc Grid::HasInvertedCoordinateSystemHandiness()
    //!
    virtual bool HasInvertedCoordinateSystemHandiness() const override;

    VDF_API friend std::ostream &operator<<(std::ostream &o, const StructuredGrid &sg);

protected:
private:
    std::vector<size_t> _cellDims;

    mutable std::vector<size_t> _duplicate;
};
};    // namespace VAPoR
#endif
