#ifndef _SphericalGrid_
#define _SphericalGrid_
#include <vector>
#include <vapor/common.h>
#include "RegularGrid.h"
#ifdef _WINDOWS
#pragma warning(disable : 4251 4100)
#endif

namespace VAPoR {

//! \class SphericalGrid
//!
//! \brief This class implements a 2D or 3D spherical grid.
//!
//! This class implements a 2D or 3D spherical grid: a generalization
//! of a regular grid where the spacing of grid points along a single dimension
//! may vary at each grid point. The spacing along the remaining one (2D case)
//! or two (3D case) dimensions is invariant between grid points. For example,
//! if K is the spherical dimension than the z coordinate is given by some
//! function f(i,j,k):
//!
//! z = f(i,j,k)
//!
//! while the remaining x and y coordinates are givey by (i*dx, j*dy)
//! for some real dx and dy .
//!
//
class VDF_API SphericalGrid : public RegularGrid {
  public:
    //!
    //! Construct a spherical grid sampling a 3D or 2D scalar function
    //!
    //! \param[in] bs A three-element vector specifying the dimensions of
    //! each block storing the sampled scalar function.
    //! \param[in] min A three-element vector specifying the ijk index
    //! of the first point in the grid. The first grid point need not coincide
    //! with
    //! block boundaries. I.e. the indecies need not be (0,0,0)
    //! \param[in] max A three-element vector specifying the ijk index
    //! of the last point in the grid
    //! \param[in] extents A six-element vector specifying the user coordinates
    //! of the first (first three elements) and last (last three elements) of
    //! the grid points indicated by \p min and \p max, respectively.
    //! The units are degrees, not radians.
    //! \param[in] permutation A three-element array indicating the ordering
    //! of the Longitude, Latitude, and radial dimensions. The array must
    //! contain some permutation of the set (0,1,2). The permuation (0,1,2)
    //! indicates that longitude is the fastest varying dimesion, then latitude
    //! then radius. The permutation (2,1,0) indicates that radius is fastest,
    //! then latitude, and so on.
    //! \param[in] periodic A three-element boolean vector indicating
    //! which i,j,k indecies, respectively, are periodic.
    //! \param[in] blks An array of blocks containing the sampled function.
    //! The dimensions of each block
    //! is given by \p bs. The number of blocks is given by the product
    //! of the terms:
    //!
    //! \code (max[i]/bs[i] - min[i]/bs[i] + 1) \endcode
    //!
    //! over i = 0..2.
    //!
    //! \param[in] coords An array of blocks with dimension and number the
    //! same as \p blks specifying the varying dimension grid point coordinates.
    //! \param[in] varying_dim An enumerant indicating which axis is the
    //! varying dimension: 0 for I, 1 for J, 2 for K
    //!
    SphericalGrid(
        const size_t bs[3],
        const size_t min[3],
        const size_t max[3],
        const double extents[6],
        const size_t permutation[3],
        const bool periodic[3],
        float **blks);

    //!
    //! Construct a spherical grid sampling a 3D or 2D scalar function
    //! that contains missing values.
    //!
    //! This constructor adds a parameter, \p missing_value, that specifies
    //! the value of missing values in the sampled function. When
    //! reconstructing the function at arbitrary coordinates special
    //! consideration is given to grid points with missing values that
    //! are used in the reconstruction.
    //!
    //! \sa GetValue()
    //!
    SphericalGrid(
        const size_t bs[3],
        const size_t min[3],
        const size_t max[3],
        const double extents[6],
        const size_t permutation[3],
        const bool periodic[3],
        float **blks,
        float missing_value);

    //! \copydoc RegularGrid::GetValue()
    //!
    float GetValue(double x, double y, double z) const;

    //! \copydoc RegularGrid::GetUserExtents()
    //!
    //! Return extents in Cartesian coordinates
    //
    virtual void GetUserExtents(double extents[6]) const {
        for (int i = 0; i < 6; i++)
            extents[i] = _extentsC[i];
    };

    //! \copydoc RegularGrid::GetVBoundingBox()
    //!
    virtual void GetBoundingBox(
        const size_t min[3],
        const size_t max[3],
        double extents[6]) const;

    //! \copydoc RegularGrid::GetEnclosingRegion()
    //!
    virtual void GetEnclosingRegion(
        const double minu[3], const double maxu[3],
        size_t min[3], size_t max[3]) const;

    //! \copydoc RegularGrid::GetUserCoordinates()
    //!
    int GetUserCoordinates(
        size_t i, size_t j, size_t k,
        double *x, double *y, double *z) const;

    //! \copydoc RegularGrid::GetIJKIndex()
    //!
    void GetIJKIndex(
        double x, double y, double z,
        size_t *i, size_t *j, size_t *k) const;

    //! \copydoc RegularGrid::GetIJKIndexFloor()
    //!
    void GetIJKIndexFloor(
        double x, double y, double z,
        size_t *i, size_t *j, size_t *k) const;

    //! Return true if the specified point lies inside the grid
    //!
    //! This method can be used to determine if a point expressed in
    //! user coordinates reside inside or outside the grid
    //!
    //! \param[in] x coordinate along fastest varying dimension
    //! \param[in] y coordinate along second fastest varying dimension
    //! \param[in] z coordinate along third fastest varying dimension
    //!
    //! \retval bool True if point is inside the grid
    //!
    bool InsideGrid(double x, double y, double z) const;

    // phi is in range -180 to 180, theta is in range -180/2 to 180/2
    //
    static inline void CartToSph(
        double x, double y, double z, double *phi, double *theta, double *r);

    static inline void SphToCart(
        double phi, double theta, double r,
        double *x, double *y, double *z);

  private:
    double _extentsC[6];            // extents in Cartesian coordinates, ordered x,y,z
    std::vector<long> _permutation; // permutation vector for coordinate ordering

    void _GetUserExtents(double extents[6]) const;

    void _permute(
        const std::vector<long> &permutation,
        double result[3], double x, double y, double z) const;
};
}; // namespace VAPoR
#endif
