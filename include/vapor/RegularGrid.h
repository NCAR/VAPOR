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
    //! Alternate constructor.
    //!
    //! \deprecated This constructor is deprecated.
    //!
    RegularGrid(const size_t bs[3], const size_t min[3], const size_t max[3], const double extents[6], const bool periodic[3], const std::vector<float *> &blks);

    //! \copydoc StructuredGrid::StructuredGrid(
    //!	const std::vector<size_t>&, const std::vector<size_t>&,
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
    RegularGrid(const std::vector<size_t> &bs, const std::vector<size_t> &min, const std::vector<size_t> &max, const std::vector<double> &minu, const std::vector<double> &maxu,
                const std::vector<bool> &periodic, const std::vector<float *> &blks);

    //! Alternate constructor.
    //!
    //! \deprecated This constructor is deprecated.
    //!
    RegularGrid(const size_t bs[3], const size_t min[3], const size_t max[3], const double extents[6], const bool periodic[3], const std::vector<float *> &blks, float missing_value);

    //! \copydoc StructuredGrid::StructuredGrid(
    //!	const std::vector<size_t>&, const std::vector<size_t>&,
    //!	const std::vector<size_t>&, const std::vector<bool>&,
    //!	const std::vector<float*>&, float
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
    RegularGrid(const std::vector<size_t> &bs, const std::vector<size_t> &min, const std::vector<size_t> &max, const std::vector<double> &minu, const std::vector<double> &maxu,
                const std::vector<bool> &periodic, const std::vector<float *> &blks, float missing_value);
    RegularGrid(const std::vector<size_t> &bs, const std::vector<size_t> &min, const std::vector<size_t> &max, const std::vector<bool> &periodic, const std::vector<float *> &blks,
                float missing_value);
    RegularGrid();

    virtual ~RegularGrid();

    // Implements StructuredGrid::GetUserExtents()
    //
    virtual void GetUserExtents(std::vector<double> &minu, std::vector<double> &maxu) const;

    //! \deprecated  This method is deprecated
    //
    virtual void GetUserExtents(double extents[6]) const;

    // Implements StructuredGrid::GetBoundingBox()
    //
    virtual void GetBoundingBox(const std::vector<size_t> &min, const std::vector<size_t> &max, std::vector<double> &minu, std::vector<double> &maxu) const;

    //! \deprecated  This method is deprecated
    //
    virtual void GetBoundingBox(const size_t min[3], const size_t max[3], double extents[6]) const;

    // Implements StructuredGrid::GetEnclosingRegion()
    //
    virtual void GetEnclosingRegion(const std::vector<double> &minu, const std::vector<double> &maxu, std::vector<size_t> &min, std::vector<size_t> &max) const;

    // Implements StructuredGrid::GetUserCoordinates()
    //
    virtual int GetUserCoordinates(size_t i, size_t j, size_t k, double *x, double *y, double *z) const;

    // Implements StructuredGrid::GetIJKIndex()
    //
    virtual void GetIJKIndex(double x, double y, double z, size_t *i, size_t *j, size_t *k) const;

    //! Return the corner grid point of the cell containing the
    //! specified user coordinates
    //!
    //! This method returns the smallest ijk index of the grid point of
    //! associated with the cell containing
    //! the specified user coordinates. If any
    //! of the input coordinates correspond to periodic dimensions the
    //! the coordinate(s) are first re-mapped to lie inside the grid
    //! extents as returned by GetUserExtents()
    //!
    //! If the specified coordinates lie outside of the grid (are not
    //! contained by any cell) the lowest valued ijk index of the grid points
    //! defining the boundary cell closest
    //! to the point are returned.
    //!
    //! \param[in] x coordinate along fastest varying dimension
    //! \param[in] y coordinate along second fastest varying dimension
    //! \param[in] z coordinate along third fastest varying dimension
    //! \param[out] i index of grid point along fastest varying dimension
    //! \param[out] j index of grid point along second fastest varying dimension
    //! \param[out] k index of grid point along third fastest varying dimension
    //!
    virtual void GetIJKIndexFloor(double x, double y, double z, size_t *i, size_t *j, size_t *k) const;

    // Implements StructuredGrid::InsideGrid()
    //
    virtual bool InsideGrid(double x, double y, double z) const;

    // Implements StructuredGrid::GetMinCelExtents()
    //
    virtual void GetMinCellExtents(double *x, double *y, double *z) const
    {
        *x = _delta[0];
        *y = _delta[1];
        *z = _delta[2];
    };

    VDF_API friend std::ostream &operator<<(std::ostream &o, const RegularGrid &rg);

protected:
    virtual float _GetValueNearestNeighbor(double x, double y, double z) const;
    virtual float _GetValueLinear(double x, double y, double z) const;
    virtual void  _ClampCoord(double &x, double &y, double &z) const;

    //! \copydoc StructuredGrid::StructuredGrid(
    //!	const std::vector<size_t>&, const std::vector<size_t>&,
    //!	const std::vector<size_t>&, const std::vector<bool>&,
    //!	const std::vector<float*>&
    //!	)
    //!
    //! Construct a regular grid sampling a 3D or 2D scalar function.
    //!
    //! This constructor does not specify user coordinates.
    //
    RegularGrid(const std::vector<size_t> &bs, const std::vector<size_t> &min, const std::vector<size_t> &max, const std::vector<bool> &periodic, const std::vector<float *> &blks);
    RegularGrid(const size_t bs[3], const size_t min[3], const size_t max[3], const bool periodic[3], const std::vector<float *> &blks);

    RegularGrid(const size_t bs[3], const size_t min[3], const size_t max[3], const bool periodic[3], const std::vector<float *> &blks, float missing_value);
    void _SetExtents(const std::vector<double> &minu, const std::vector<double> &maxu);

private:
    std::vector<double> _minu;
    std::vector<double> _maxu;     // User coords of first and last voxel
    std::vector<double> _delta;    // increment between grid points in user coords
};
};    // namespace VAPoR
#endif
