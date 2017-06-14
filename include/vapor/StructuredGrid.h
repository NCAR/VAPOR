#ifndef _StructuredGrid_
#define _StructuredGrid_

#include <ostream>
#include <vector>
#include <vapor/common.h>

#ifdef WIN32
#pragma warning(disable : 4661 4251) // needed for template class
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
//! when using the class iterator: StructuredGrid::Iterator.
//!
//! For methods that allow the specification of grid indecies or coordinates
//! as a single parameter tuple (e.g. float coordinate[3]) the first element
//! of the tuple corresponds to the \a I axis, etc.
//!
//! \note Throughout this class grid vertex offsets are specified as
//! \a i, \a j, \a k, where \a i, \a j, \a k are integers. User coordinates
//! are real values denoted \a x, \a y, \a z, and are given by functions
//! \a X(i,j,k), \a Y(i,j,k), \a Z(i,j,k).
//
class VDF_API StructuredGrid {
  public:
    //! Alternate constructor using arrays as arguments.
    //!
    //! The length of parameter arrays \p bs, \p min, \p max, \p periodic
    //! must all be 3.
    //! If the 3rd component of \p min equals the 3rd component of \p max
    //! a 2D structured grid is created, and the 3rd component of all
    //! other parameter arrays is ignored.
    //!
    //! \deprecated This constructor is deprecated.
    //!
    StructuredGrid(
        const size_t bs[3],
        const size_t min[3],
        const size_t max[3],
        const bool periodic[3],
        const std::vector<float *> &blks);

    //!
    //! Construct a structured grid sampling a 3D or 2D scalar function
    //!
    //! The sampled function is represented as a 2D or 3D array, decomposed
    //! into smaller blocks (tiles in 2D). The dimensions of the array are not
    //! constrained to coincide with block (tile) boundaries.
    //!
    //! The length of parameter vectors \p bs, \p min, \p max, \p periodic
    //! must all be either 3 (3D structured grid) or 2 (2D structured grid)
    //!
    //! If \p blks is empty a dataless StructuredGrid object is returned.
    //! Data can not be retrieved from a dataless StructuredGrid. However,
    //! coordinate access methods may still be invoked.
    //!
    //! \param[in] bs A two or three-element vector specifying the dimensions of
    //! each block storing the sampled scalar function.
    //! \param[in] min A two or three-element vector specifying the ijk index
    //! of the first point in the grid. The first grid point need not coincide
    //! with
    //! block boundaries. I.e. the indecies need not be (0,0,0): the first
    //! grid point is not required to be the first element of the array.
    //! \param[in] max A two or three-element vector specifying the ijk index
    //! of the last point in the grid
    //! \param[in] periodic A three-element boolean vector indicating
    //! which i,j,k indecies, respectively, are periodic
    //! \param[in] blks An array of blocks containing the sampled function.
    //! The dimensions of each block
    //! is given by \p bs. The number of blocks is given by the product
    //! of the terms:
    //!
    //! \code (max[i]/bs[i] - min[i]/bs[i] + 1) \endcode
    //!
    //! over i = 0..2 (3D), and i = 0..1 (2D).
    //!
    //! A shallow copy of the blocks is made by the constructor. Memory
    //! referenced by the elements of \p blks should remain valid
    //! until the class instance is destroyed.
    //!
    StructuredGrid(
        const std::vector<size_t> &bs,
        const std::vector<size_t> &min,
        const std::vector<size_t> &max,
        const std::vector<bool> &periodic,
        const std::vector<float *> &blks);

    //! Alternate constructor using arrays as arguments.
    //!
    //! The length of parameter arrays \p bs, \p min, \p max, \p periodic
    //! must all be 3.
    //! If the 3rd component of \p min equals the 3rd component of \p max
    //! a 2D structured grid is created, and the 3rd component of all
    //! other parameter arrays is ignored.
    //!
    //! \deprecated This constructor is deprecated.
    //!
    StructuredGrid(
        const size_t bs[3],
        const size_t min[3],
        const size_t max[3],
        const bool periodic[3],
        const std::vector<float *> &blks,
        float missing_value);

    //!
    //! Construct a regular grid sampling a 3D or 2D scalar function
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
    StructuredGrid(
        const std::vector<size_t> &bs,
        const std::vector<size_t> &min,
        const std::vector<size_t> &max,
        const std::vector<bool> &periodic,
        const std::vector<float *> &blks,
        float missing_value);

    StructuredGrid();

    virtual ~StructuredGrid();

    //! Set or Get the data value at the indicated grid point
    //!
    //! This method provides read access to the scalar data value
    //! defined at the grid point indicated by index(i,j,k). The range
    //! of valid indecies is between zero and \a dim - 1, where \a dim
    //! is the dimesion of the grid returned by GetDimensions()
    //!
    //! If any of the indecies \p i, \p j, or \p k are outside of the
    //! valid range the results are undefined
    //!
    //! For 2D grids the \p k parameter is ignored.
    //!
    //! \param[in] i index of grid point along fastest varying dimension
    //! \param[in] j index of grid point along second fastest varying dimension
    //! \param[in] k index of grid point along third fastest varying dimension
    //!
    float &AccessIJK(size_t i, size_t j, size_t k) const;

    //! Get the reconstructed value of the sampled scalar function
    //!
    //! This method reconstructs the scalar field at an arbitrary point
    //! in space. If the point's coordinates are outside of the grid's
    //! coordinate extents as returned by GetUserExtents(), and the grid
    //! is not periodic along the out-of-bounds axis, the value
    //! returned will be the \a missing_value.
    //!
    //! If the value of any of the grid point samples used in the reconstruction
    //! is the \a missing_value then the result returned is the \a missing_value.
    //!
    //! The reconstruction method used is determined by interpolation
    //! order returned by GetInterpolationOrder()
    //!
    //! \param[in] x coordinate along fastest varying dimension
    //! \param[in] y coordinate along second fastest varying dimension
    //! \param[in] z coordinate along third fastest varying dimension
    //!
    //! \sa GetInterpolationOrder(), HasPeriodic(), GetMissingValue()
    //! \sa GetUserExtents()
    //!
    virtual float GetValue(double x, double y, double z) const;

    //! Return the extents of the user coordinate system
    //!
    //! This pure virtual method returns min and max extents of
    //! the user coordinate
    //! system defined on the grid. The extents of the returned box
    //! are guaranteed to contain all points in the grid.
    //!
    //! \param[out] minu A two or three element array containing the minimum
    //! user coordinates.
    //! \param[out] maxu A two or three element array containing the maximum
    //! user coordinates.
    //!
    //! \sa GetDimensions(), StructuredGrid()
    //!
    virtual void GetUserExtents(
        std::vector<double> &minu, std::vector<double> &maxu) const = 0;

    //! \deprecated  This method is deprecated
    //!
    virtual void GetUserExtents(double extents[6]) const;

    //! Return the extents of the axis-aligned bounding box enclosign a region
    //!
    //! This pure virtual method returns min and max extents, in user coordinates,
    //! of the smallest axis-aligned box enclosing the region defined
    //! by the corner grid points, \p min and \p max.
    //!
    //! \note The results are undefined if any coordinate of \p min is
    //! greater than the coresponding coordinate of \p max, or if \p max
    //! is outside of the valid dimension range (See GetDimension()).
    //!
    //! \param[in] min A two-to-three element array containing the minimum
    //! voxel coordinates (offsets).
    //! \param[in] max A two-to-three element array containing the maximum
    //! voxel coordinates (offsets).
    //! \param[out] minu A two-to-three element array containing the minimum
    //! user coordinates.
    //! \param[out] maxu A two-to-three element array containing the maximum
    //! user coordinates.
    //!
    //! \sa GetDimensions(), StructuredGrid()
    //!
    virtual void GetBoundingBox(
        const std::vector<size_t> &min, const std::vector<size_t> &max,
        std::vector<double> &minu, std::vector<double> &maxu) const = 0;

    //!
    //! Get voxel coordinates of grid containing a region
    //!
    //! Calculates the starting and ending IJK voxel coordinates of the
    //! smallest grid
    //! completely containing the rectangular region defined by the user
    //! coordinates \p minu and \p maxu
    //! If rectangluar region defined by \p minu and \p maxu can
    //! not be contained the
    //! minimum and maximum IJK coordinates are returned in
    //! \p min and \p max, respectively
    //!
    //! \param[in] minu User coordinates of minimum coorner
    //! \param[in] maxu User coordinates of maximum coorner
    //! \param[out] min Integer coordinates of minimum coorner
    //! \param[out] max Integer coordinates of maximum coorner
    //!
    virtual void GetEnclosingRegion(
        const std::vector<double> &minu, const std::vector<double> &maxu,
        std::vector<size_t> &min, std::vector<size_t> &max) const = 0;

    //! \deprecated  This method is deprecated
    //
    virtual void GetEnclosingRegion(
        const double minu[3], const double maxu[3], size_t min[3], size_t max[3]) const;

    //! Return the origin of the grid in global IJK coordinates
    //!
    //! This method returns the value of the \p min parameter passed
    //! to the constructor
    //!
    //! \param[out] min[3] Minimum IJK coordinates in global grid coordinates
    //
    virtual void GetIJKOrigin(std::vector<size_t> &min) const {
        min = _minabs;
    }

    //! \deprecated This method is deprecated
    //
    virtual void GetIJKOrigin(size_t min[3]) const {
        for (int i = 0; i < 3; i++)
            min[i] = 0;
        for (int i = 0; i < _minabs.size(); i++)
            min[i] = _minabs[i];
    }

    //! Return the ijk dimensions of grid
    //!
    //! Returns the number of grid points defined along each axis of the grid
    //!
    //! \param[out] dims A two or three element array containing the
    //! grid dimensions
    //!
    void GetDimensions(std::vector<size_t> &dims) const;
    std::vector<size_t> GetDimensions() const;

    //! \deprecated  This method is deprecated
    //
    void GetDimensions(size_t dims[3]) const;

    //! Return the ijk dimensions of grid in blocks
    //!
    //! Returns the number of blocks defined along each axis of the grid
    //!
    //! \param[out] dims A two or three element array containing the grid
    //! dimension in blocks
    //!
    //! \sa GetBlockSize();
    //
    void GetDimensionInBlks(std::vector<size_t> &bdims) const {
        bdims.clear();
        for (int i = 0; i < _ndim; i++)
            bdims.push_back(_bdims[i]);
    }

    //! Return the value of the \a missing_value parameter
    //!
    //! The missing value is a special value intended to indicate that the
    //! value of the sampled or reconstructed function is unknown at a
    //! particular point.
    //!
    float GetMissingValue() const { return (_missingValue); };

    //! Set the missing value indicator
    //!
    //! This method sets the value of the missing value indicator. The
    //! method does not alter the value of any grid point locations, nor
    //! does it alter the missing data flag set with the constructor.
    //!
    //! \sa HasMissingData(), GetMissingValue()
    //!
    void SetMissingValue(float missing_value) {
        _missingValue = missing_value;
    };

    //! Return missing data flag
    //!
    //! This method returns true iff the class instance was created with
    //! the constructor specifying \p missing_value parameter or if
    //! the SetMissingValue() method has been called. This does not
    //! imply that grid points exist with missing data, only that the
    //! class was constructed with the missing data version of the constructor.
    //
    bool HasMissingData() const { return (_hasMissing); };

    //! Return the interpolation order to be used during function reconstruction
    //!
    //! This method returns the order of the interpolation method that will
    //! be used when reconstructing the sampled scalar function
    //!
    //! \sa SetInterpolationOrder()
    //!
    virtual int GetInterpolationOrder() const { return _interpolationOrder; };

    //! Set the interpolation order to be used during function reconstruction
    //!
    //! This method sets the order of the interpolation method that will
    //! be used when reconstructing the sampled scalar function. Valid values
    //! of \p order are 0 and 1, corresponding to nearest-neighbor and linear
    //! interpolation, respectively. If \p order is invalid it will be silently
    //! set to 1. The default interpolation order is 1
    //!
    //! \param[in] order interpolation order
    //! \sa GetInterpolationOrder()
    //!
    virtual void SetInterpolationOrder(int order);

    //! Return the user coordinates of a grid point
    //!
    //! This method returns the user coordinates of the grid point
    //! specified by index(i,j,k)
    //!
    //! The \p k parameter is ignored, and the value of \p z is
    //! undefined, for 2D grids.
    //!
    //! \param[in] i index of grid point along fastest varying dimension
    //! \param[in] j index of grid point along second fastest varying dimension
    //! \param[in] k index of grid point along third fastest varying dimension
    //! \param[out] x coordinate of grid point along fastest varying dimension
    //! \param[out] y coordinate of grid point along second fastest
    //! varying dimension
    //! \param[out] z coordinate of grid point along third fastest
    //! varying dimension
    //!
    //! \retval status A negative int is returned if index(i,j,k) is out
    //! out of bounds
    //!

    virtual int GetUserCoordinates(
        size_t i, size_t j, size_t k,
        double *x, double *y, double *z) const = 0;

    //! Return the closest grid point to the specified user coordinates
    //!
    //! This method returns the ijk index of the grid point closest to
    //! the specified user coordinates based on Euclidean distance. If any
    //! of the input coordinates correspond to periodic dimensions the
    //! the coordinate(s) are first re-mapped to lie inside the grid
    //! extents as returned by GetUserExtents()
    //!
    //! \param[in] x coordinate along fastest varying dimension
    //! \param[in] y coordinate along second fastest varying dimension
    //! \param[in] z coordinate along third fastest varying dimension
    //! \param[out] i index of grid point along fastest varying dimension
    //! \param[out] j index of grid point along second fastest varying dimension
    //! \param[out] k index of grid point along third fastest varying dimension
    //!
    virtual void GetIJKIndex(
        double x, double y, double z,
        size_t *i, size_t *j, size_t *k) const = 0;

    //! Return the min and max data value
    //!
    //! This method returns the values of grid points with min and max values,
    //! respectively.
    //!
    //! For dataless grids the missing value is returned.
    //!
    //! \param[out] range[2] A two-element array containing the mininum and
    //! maximum values, in that order
    //!
    virtual void GetRange(float range[2]) const;

    //! Return true if the specified point lies inside the grid
    //!
    //! This method can be used to determine if a point expressed in
    //! user coordinates reside inside or outside the grid
    //!
    //! The parameter \p z is ignored for 2D grids
    //!
    //! \param[in] x coordinate along fastest varying dimension
    //! \param[in] y coordinate along second fastest varying dimension
    //! \param[in] z coordinate along third fastest varying dimension
    //!
    //! \retval bool True if point is inside the grid
    //!
    virtual bool InsideGrid(double x, double y, double z) const = 0;

    //! Check for periodic boundaries
    //!
    //! This method returns a boolean for each dimension indicating
    //! whether the data along that dimension has periodic boundaries.
    //!
    //! The parameter \p k is undefined for 2D grids
    //!
    //! \param[out] idim periodicity of fastest varying dimension
    //! \param[out] jdim periodicity of second fastest varying dimension
    //! \param[out] kdim periodicity of third fastest varying dimension
    //
    virtual void HasPeriodic(bool *idim, bool *jdim, bool *kdim) const {
        *idim = *jdim = *kdim = false;
        *idim = _periodic[0];
        *jdim = _periodic[1];
        if (_ndim)
            *kdim = _periodic[2];
    }

    //! Set periodic boundaries
    //!
    //! This method changes the periodicity of boundaries set
    //! by the class constructor
    //!
    //! \param[in] periodic A three-element boolean vector indicating
    //! which i,j,k indecies, respectively, are periodic
    //
    virtual void SetPeriodic(const std::vector<bool> &periodic);

    //! \deprecated This method is deprecated
    //
    virtual void SetPeriodic(const bool periodic[3]);

    //! Return the internal blocking factor
    //!
    //! This method returns the internal blocking factor passed
    //! to the constructor.
    //
    void GetBlockSize(std::vector<size_t> &bs) const {
        bs = _bs;
    }

    //! \deprecated This method is deprecated
    //
    void GetBlockSize(size_t bs[3]) const;

    //! Return the minimum grid spacing between all grid points
    //!
    //! This method returns the minimum distance, in user coordinates,
    //! between adjacent grid points  for all cells in the grid.
    //!
    //! The parameter \p z is undefined for 2D grids
    //!
    //! \param[out] x Minimum distance between grid points along X axis
    //! \param[out] y Minimum distance between grid points along Y axis
    //! \param[out] z Minimum distance between grid points along Z axis
    //!
    //! \note For a regular grid all cells have the same dimensions
    //!
    virtual void GetMinCellExtents(double *x, double *y, double *z) const = 0;

    //! Return the rank of the grid
    //!
    //! This method returns the number of dimensions, 2 or 3.
    //
    virtual int GetRank() {
        return (_ndim);
    }

    //! Return the internal data structure containing a copy of the blocks
    //! passed in by the constructor
    //!
    const std::vector<float *> &GetBlks() const { return (_blks); };

    //! A forward iterator for accessing the data elements of the
    //! structured grid.
    //!
    //! This class provides a C++ STL style Forward Iterator for
    //! accessing grid elements passed to the constructor. All Forward
    //! Iterator expressions are supported. In addition, the following
    //! Random Access Iterator expressions are supported:
    //!
    //! \li \c a + n
    //! \li \c n + a
    //!
    //! where \i a are objects of type ForwardIterator, and \i n is an int.
    //
    template <class T>
    class VDF_API ForwardIterator {
      public:
        ForwardIterator(T *rg);
        ForwardIterator();
        ~ForwardIterator() {}

        inline float &operator*() { return (*_itr); }

        ForwardIterator<T> &operator++();   // ++prefix
        ForwardIterator<T> operator++(int); // postfix++

        ForwardIterator<T> &operator+=(const long int &offset);
        ForwardIterator<T> operator+(const long int &offset) const;

        bool operator==(const ForwardIterator<T> &other);
        bool operator!=(const ForwardIterator<T> &other);

        int GetUserCoordinates(
            double *x, double *y, double *z) const {
            return (
                _rg->GetUserCoordinates(
                    _x - _rg->_min[0], _y - _rg->_min[1],
                    _rg->_ndim == 3 ? _z - _rg->_min[2] : 0,
                    x, y, z));
        };

      private:
        T *_rg;
        size_t _x, _y, _z; // current index into _rg->_min[3]
        size_t _xb;        // x index within a block
        float *_itr;
        bool _end;
    };

    typedef StructuredGrid::ForwardIterator<StructuredGrid> Iterator;
    typedef StructuredGrid::ForwardIterator<StructuredGrid const> ConstIterator;

    Iterator begin() { return (Iterator(this)); }
    Iterator end() { return (Iterator()); }

    ConstIterator begin() const { return (ConstIterator(this)); }
    ConstIterator end() const { return (ConstIterator()); }

    VDF_API friend std::ostream &operator<<(std::ostream &o, const StructuredGrid &rg);

  protected:
    float &_AccessIJK(
        const std::vector<float *> &blks, size_t i, size_t j, size_t k) const;
    std::vector<size_t> _min;    // ijk offset of 1st voxel in ROI (relative to ROI)
    std::vector<size_t> _max;    // ijk offset of last voxel in ROI (relative to ROI)
    std::vector<size_t> _bs;     // dimensions of each block
    std::vector<size_t> _bdims;  // dimensions (specified in blocks) of ROI
    std::vector<size_t> _minabs; // ijk offset of 1st voxel in ROI (abs. coords)
    std::vector<size_t> _maxabs; // ijk offset of last voxel in ROI (abs. coords)
    std::vector<bool> _periodic; // periodicity of boundaries
    float _missingValue;
    bool _hasMissing;
    int _interpolationOrder; // Order of interpolation
    int _ndim;               // Number of dimensions: 1D, 2D, 3D

    virtual void _ClampCoord(double &x, double &y, double &z) const = 0;
    virtual float _GetValueNearestNeighbor(double x, double y, double z) const = 0;
    virtual float _GetValueLinear(double x, double y, double z) const = 0;

  private:
    std::vector<float *> _blks;

    void _StructuredGrid(
        const std::vector<size_t> &bs,
        const std::vector<size_t> &min,
        const std::vector<size_t> &max,
        const std::vector<bool> &periodic,
        const std::vector<float *> &blks);
};
}; // namespace VAPoR
#endif
