#ifndef _LayeredGrid_
#define _LayeredGrid_
#include <vapor/common.h>
#include "RegularGrid.h"

namespace VAPoR {

//! \class LayeredGrid
//!
//! \brief This class implements a 2D or 3D layered grid.
//!
//! This class implements a 2D or 3D layered grid: a generalization
//! of a regular grid where the spacing of grid points along the K dimension
//! varies at each grid point. The spacing along the remaining I and J
//! dimensions is invariant between grid points. I.e.
//! z coordinate is given by some
//! function f(i,j,k):
//!
//! z = f(i,j,k)
//!
//! where f() is monotonically increasing (or decreasing) with k.
//! The remaining x and y coordinates are givey by (i*dx, j*dy)
//! for some real dx and dy .
//!
//
class VDF_API LayeredGrid : public RegularGrid {
public:

 //!
 //! Construct a layered grid sampling a 3D or 2D scalar function
 //!
 //! \param[in] bs A three-element vector specifying the dimensions of
 //! each block storing the sampled scalar function.
 //! \param[in] min A three-element vector specifying the ijk index
 //! of the first point in the grid. The first grid point need not coincide 
 //! with
 //! block boundaries. I.e. the indecies need not be (0,0,0)
 //! \param[in] max A three-element vector specifying the ijk index
 //! of the last point in the grid
 //! \param[in] minu A two-element vector specifying the X and Y user 
 //! coordinates
 //! of the first grid point.
 //! \param[in] maxu A two-element vector specifying the X and Y user 
 //! coordinates
 //! of the last grid point.
 //! \param[in] periodic A three-element boolean vector indicating
 //! which i,j,k indecies, respectively, are periodic. The varying 
 //! dimension may not be periodic.
 //! \param[in] blks An array of blocks containing the sampled function.
 //! The dimensions of each block
 //! is given by \p bs. The number of blocks is given by the product
 //! of the terms:
 //!
 //! \code (max[i]/bs[i] - min[i]/bs[i] + 1) \endcode
 //!
 //! over i = 0..2.
 //!
 //! \param[in] rg A RegularGrid instance with the same dimensionality and
 //! min/max offsets as specified by \p bs, \p min, and \p max. The 
 //! data values of \p rg provide the user coordinates for the Z dinmension.
 //!
 LayeredGrid(
	const std::vector <size_t> &bs,
	const std::vector <size_t> &min,
	const std::vector <size_t> &max,
	const std::vector <double> &minu,
	const std::vector <double> &maxu,
	const std::vector <bool> &periodic,
	const std::vector <float *> &blks,
	const RegularGrid &rg
);

#ifdef	DEAD
 //! \deprecated This method is deprecated.
 //!
 LayeredGrid(
	const size_t bs[3],
	const size_t min[3],
	const size_t max[3],
	const double extents[6],
	const bool periodic[3],
	const std::vector <float *> &blks,
	const RegularGrid &rg
);
#endif

 //! 
 //! Construct a layered grid sampling a 3D or 2D scalar function
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
 LayeredGrid(
	const std::vector <size_t> &bs,
	const std::vector <size_t> &min,
	const std::vector <size_t> &max,
	const std::vector <double> &minu,
	const std::vector <double> &maxu,
	const std::vector <bool> &periodic,
	const std::vector <float *> &blks,
	const RegularGrid &rg,
	float missing_value
 );

#ifdef	DEAD
 //! \deprecated This method is deprecated.
 //!
 LayeredGrid(
	const size_t bs[3],
	const size_t min[3],
	const size_t max[3],
	const double extents[6],
	const bool periodic[3],
	const std::vector <float *> &blks,
	const RegularGrid &rg,
	float missing_value
 );
#endif

 virtual ~LayeredGrid();


 //! \copydoc RegularGrid::GetValue()
 //!
 float GetValue(double x, double y, double z) const;

 //! \copydoc RegularGrid::GetUserExtents()
 //!
 virtual void GetUserExtents(
	std::vector <double> &minu, std::vector <double> &maxu
) const;


 //! \copydoc RegularGrid::GetBoundingBox()
 //!
 virtual void GetBoundingBox(
	const std::vector <size_t> &min, const std::vector <size_t> &max,
	std::vector <double> &minu, std::vector <double> &maxu
 ) const;

 //! \copydoc RegularGrid::GetEnclosingRegion()
 //!
 virtual void    GetEnclosingRegion(
    const std::vector <double> &minu, const std::vector <double> &maxu,
    std::vector <size_t> &min, std::vector <size_t> &max
 ) const;


 //! \copydoc RegularGrid::GetUserCoordinates()
 //!
 int GetUserCoordinates(
	size_t i, size_t j, size_t k, 
	double *x, double *y, double *z
 ) const;

 //! \copydoc RegularGrid::GetIJKIndex()
 //!
 void GetIJKIndex(
	double x, double y, double z,
	size_t *i, size_t *j, size_t *k
 ) const;

 //! \copydoc RegularGrid::GetIJKIndexFloor()
 //!
 void GetIJKIndexFloor(
	double x, double y, double z,
	size_t *i, size_t *j, size_t *k
 ) const;

 //! \copydoc RegularGrid::Reshape()
 //!
 int Reshape(
	const size_t min[3],
	const size_t max[3],
	const bool periodic[3]
 );

 //! \copydoc RegularGrid::InsideGrid()
 //!
 bool InsideGrid(double x, double y, double z) const;


 //! Set periodic boundaries
 //!
 //! This method changes the periodicity of boundaries set 
 //! by the class constructor. It overides the base class method
 //! to ensure the varying dimension is not periodic.
 //!
 //! \param[in] periodic A three-element boolean vector indicating
 //! which i,j,k indecies, respectively, are periodic. The varying dimension
 //! may not be periodic.
 //
 virtual void SetPeriodic(const bool periodic[3]);
 virtual void SetPeriodic(const std::vector <bool> &periodic);

 //! \copydoc RegularGrid::GetMinCellExtents()
 //!
 virtual void GetMinCellExtents(double *x, double *y, double *z) const;

 //! Return the internal data structure containing a copy of the coordinate
 //! blocks passed in by the constructor
 //!
 const RegularGrid &GetZRG() const { return(_rg); };


private:
 RegularGrid _rg;
 std::vector <double> _minu;
 std::vector <double> _maxu;

 void _layeredGrid(
	const std::vector <bool> &periodic,
	const std::vector <double> &minu,
	const std::vector <double> &maxu,
	const RegularGrid &rg
 );

 void _GetUserExtents(
	std::vector <double> &minu, std::vector <double> &maxu
 ) const;

 float _GetValueNearestNeighbor(double x, double y, double z) const;
 float _GetValueLinear(double x, double y, double z) const;


 //! void _getBilinearWeights(double x, double y, size_t k,
 //!						double *iwgt, double *jwgt) const;
 //! 
 //! Return the bilinear interpolation weights of a point given in user
 //! coordinates.  These weights apply to the x (iwgt) and y (jwgt) axes.
 //!
 //! This function applies the bilinear interpolation method to derive
 //! a the x and y axis weights of a point in user coordinates.
 //!
 //! \param[in] x coordinate of grid point along fastest varying dimension
 //! \param[in] y coordinate of grid point along second fastest 
 //! varying dimension
 //! \param[out] a bilinearly calculated weight for the x axis
 //! \param[out] a bilinearly calculated weight for the y axis
 //
 void _getBilinearWeights(double x, double y, double z,
						double &iwgt, double &jwgt) const;

 //! double _bilinearElevation(double x, double y, size_t k,
 //!						double *iwgt, double *jwgt) const;
 //!
 //! Return the bilinearly interpolated elevation of a point given in user
 //! coordinates. 
 //!
 //! This function applies the bilinear interpolation method to derive
 //! a the elevation from x and y axis weights of a point in user coordinates.
 //!
 //! \param[in] i index of bottom left cell corner 
 //! \param[in] i index of top right cell corner
 //! \param[in] j index of bottom left cell corner 
 //! \param[in] j index of top right cell corner
 //! \param[in] k index of the level to interpolate upon
 //! \param[in] the i-axis weight for bilinear interpolation
 //! \param[in] the j-axis weight for bilinear interpolation
 //! \param[out] a bilinearly calculated elevation value
 
 double _bilinearElevation(size_t i0, size_t i1, size_t j0, size_t j1,
						size_t k0, double iwgt, double jwgt) const;

 //! double _bilinearInterpolation(double x, double y, size_t k,
 //!						double *iwgt, double *jwgt) const;
 //!
 //! Return the bilinearly interpolated value of the currently opened variable
 //! of a point given in user coordinates. 
 //!
 //! This function applies the bilinear interpolation method to derive
 //! a variable value from x and y axis weights of a point in user coordinates.
 //!
 //! \param[in] i index of bottom left cell corner 
 //! \param[in] i index of top right cell corner
 //! \param[in] j index of bottom left cell corner 
 //! \param[in] j index of top right cell corner
 //! \param[in] k index of the level to interpolate upon
 //! \param[in] the i-axis weight for bilinear interpolation
 //! \param[in] the j-axis weight for bilinear interpolation
 //! \param[out] a bilinearly calculated value of the currently open variable
 double _bilinearInterpolation(size_t i0, size_t i1, size_t j0, size_t j1,
						size_t k0, double iwgt, double jwgt) const;

 //! Return the interpolated value of a point in user
 //! coordinates.  This only interpolates in the vertical (z) direction.
 //!
 //! Return the quadratically interpolated value of a point in user
 //! coordinates.
 //!
 //! This function applies the quadratic interpolation method to derive
 //! a the value of a variable in user coordinates from its neighboring
 //! points in ijk space.  Linear interpolation is applied at the boundaries
 //! of the domain.
 //!
 //! \param[in] x coordinate of grid point along fastest varying dimension
 //! \param[in] y coordinate of grid point along second fastest 
 //! varying dimension
 //! \param[in] z coordinate of grid point along third fastest 
 //! varying dimension
 //! \param[out] a quadratically interpolated value of a point in user
 //! coordinates
 //!
 float _getValueQuadratic(double x, double y, double z) const;

 //! Return the linearly interpolated value of a point in user
 //! coordinates.  This only interpolates in the vertical (z) direction.
 //!
 //! \param[in] x coordinate of grid point along fastest varying dimension
 //! \param[in] y coordinate of grid point along second fastest 
 //! varying dimension
 //! \param[in] z coordinate of grid point along third fastest 
 //! varying dimension
 //! \param[out] a linearly interpolated value of a point in user
 //! coordinates.
 //!
 double _verticalLinearInterpolation(double x, double y, double z) const;

 double _interpolateVaryingCoord(
	size_t i0, size_t j0, size_t k0,
	double x, double y, double z 
 ) const;

};
};
#endif
