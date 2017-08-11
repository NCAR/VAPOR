#ifndef _RegularGrid_
#define _RegularGrid_

#include <ostream>
#include <vector>
#include <vapor/common.h>
#include <vapor/StructuredGrid.h>

namespace VAPoR {

//! \class RegularGrid
//! \brief This class implements a 2D or 3D regular grid.
//!
//! This class implements a 2D or 3D regular grid: a tessellation
//! of Euculidean space by rectangles (2D) or parallelpipeds (3D). Each
//! grid point can be addressed by an index(i,j,k), where \a i, \p a
//! and \a k range from 0 to \a dim - 1, where \a dim is the dimension of the
//! \a I, \a J, or \a K axis, respectively. Moreover, each grid point
//! has a coordinate in a user-defined coordinate system given by
//! (\a i * \a dx, \a j * \a dy, \a k * \a dz) for some real
//! numbers \a dx, \a dy, and \a dz representing the grid spacing.
//!
//

class VDF_API RegularGrid : public StructuredGrid {
public:
    //! \copydoc StructuredGrid::StructuredGrid(
    //!	const std::vector<size_t>&, const std::vector<bool>&,
    //!	const std::vector<float*>&
    //!	)
    //!
    //! Construct a regular grid sampling a 3D or 2D scalar function.
    //!
    //! Adds new parameters:
    //!
    //! \param[in] minu A vector provding the user coordinates of the first
    //! point in the grid.
    //! \param[in] maxu A vector provding the user coordinates of the last
    //! point in the grid. All elements of \p maxu must be greater than or
    //! equal to corresponding elements in \p minu.
    //!
    //
    RegularGrid(const std::vector<size_t> &dims, const std::vector<size_t> &bs, const std::vector<float *> &blks, const std::vector<double> &minu, const std::vector<double> &maxu);

    RegularGrid();

    virtual ~RegularGrid();

    //! \copydoc Grid::GetUserExtents()
    //
    virtual void GetUserExtents(std::vector<double> &minu, std::vector<double> &maxu) const;

    //! \copydoc Grid::GetBoundingBox()
    //
    virtual void GetBoundingBox(const std::vector<size_t> &min, const std::vector<size_t> &max, std::vector<double> &minu, std::vector<double> &maxu) const;

    //! \copydoc Grid::GetEnclosingRegion()
    //
    virtual void GetEnclosingRegion(const std::vector<double> &minu, const std::vector<double> &maxu, std::vector<size_t> &min, std::vector<size_t> &max) const;

    //! \copydoc Grid::GetUserCoordinates()
    //
    virtual void GetUserCoordinates(const std::vector<size_t> &indices, std::vector<double> &coords) const;

    //! \copydoc Grid::GetIndices()
    //
    virtual void GetIndices(const std::vector<double> &coords, std::vector<size_t> &indices) const;

    //! \copydoc Grid::GetIndicesCell
    //!
    virtual bool GetIndicesCell(const std::vector<double> &coords, std::vector<size_t> &indices) const;

    //! \copydoc Grid::InsideGrid()
    //
    virtual bool InsideGrid(const std::vector<double> &coords) const;

    VDF_API friend std::ostream &operator<<(std::ostream &o, const RegularGrid &rg);

protected:
    virtual float _GetValueNearestNeighbor(const std::vector<double> &coords) const;
    virtual float _GetValueLinear(const std::vector<double> &coords) const;

private:
    void _SetExtents(const std::vector<double> &minu, const std::vector<double> &maxu);

    std::vector<double> _minu;
    std::vector<double> _maxu;     // User coords of first and last voxel
    std::vector<double> _delta;    // increment between grid points in user coords
};
};    // namespace VAPoR
#endif
