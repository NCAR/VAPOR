#ifndef _Grid_
#define _Grid_

#include <ostream>
#include <vector>
#include <cassert>
#include <vapor/common.h>

#ifdef WIN32
#pragma warning(disable : 4661 4251) // needed for template class
#endif

namespace VAPoR {

//! \class Grid
//! \brief Abstract base class for a 2D or 3D structured or unstructured
//!  grid.
//! \author John Clyne
//!
//! This abstract base class defines a 2D or 3D structured or unstructured
//! grid.
//!
//! The grid samples a scalar function at each grid point.  Each
//! grid point can be addressed by a multi-dimensional
//! vector <size_t> indices. The fastest varying dimension is
//! given by indices[0], etc.
//!
//! Because grid samples are repesented internally as arrays, when accessing
//! multiple grid points better performance is achieved by using
//! unit stride.
//!
//! \param indices A vector of integer indices i in the range 0..max,
//! where max is one minus the value of the corresponding element
//! returned by GetDimensions().
//!
//! \param coords A vector of floating point values with size given by
//! GetTopologyDim() containig the coordinates of a point in user-defined
//! coordinates.
//!
//
class VDF_API Grid {
  public:
    //!
    //! Construct a structured or unstructured grid sampling a 3D or
    //! 2D scalar function
    //!
    //! \param[in] dims Dimensions of arrays containing grid data.
    //!
    //! \param[in] bs A vector with size matching \p dims, specifying the
    //! dimensions of
    //! each block storing the sampled scalar function.
    //!
    //! \param[in] blks An array of blocks containing the sampled function.
    //! The dimensions of each block
    //! is given by \p bs. The number of blocks is given by the product
    //! of the terms:
    //!
    //! \code (((dims[i]-1) / bs[i]) + 1) \endcode
    //!
    //! over i = 0..dims.size()-1
    //!
    //! A shallow copy of the blocks is made by the constructor. Memory
    //! referenced by the elements of \p blks should remain valid
    //! until the class instance is destroyed.
    //!
    //! \param[in] topology_dimension Topological dimension of
    //! grid: 2 or 3, for 2D or 3D, respectively. Grids with 2D
    //! topology are described by 2D spatial coordiantes, while
    //! grids with 2D topology are described by 3D spatial coordinates.
    //!
    Grid(
        const std::vector<size_t> &dims,
        const std::vector<size_t> &bs,
        const std::vector<float *> &blks,
        size_t topology_dimension);

    Grid() = default;
    virtual ~Grid() = default;

    //! Return the dimensions of grid connectivity array
    //!
    //! \param[out] dims The value of \p dims parameter provided to
    //! the constructor.
    //!
    const std::vector<size_t> &GetDimensions() const {
        return (_dims);
    }

    //! Return the ijk dimensions of grid in blocks
    //!
    //! Returns the number of blocks defined along each axis of the grid
    //!
    //! \param[out] dims A two or three element array containing the grid
    //! dimension in blocks
    //!
    //! \sa GetBlockSize();
    //
    const std::vector<size_t> &GetDimensionInBlks() const {
        return (_bdims);
    }

    //! Return the internal blocking factor
    //!
    //! This method returns the internal blocking factor passed
    //! to the constructor.
    //
    const std::vector<size_t> &GetBlockSize() const {
        return (_bs);
    }

    //! Return the internal data structure containing a copy of the blocks
    //! passed in by the constructor
    //!
    const std::vector<float *> &GetBlks() const { return (_blks); };

    //! Get the data value at the indicated grid point
    //!
    //! This method provides read access to the scalar data value
    //! defined at the grid point indicated by \p indices. The range
    //! of valid indices is between zero and \a dim - 1, where \a dim
    //! is the dimesion of the grid returned by GetDimensions()
    //!
    //! If any of the \p indecies are outside of the
    //! valid range the results are undefined
    //!
    //! \param[in] indices of grid point along fastest varying dimension. The
    //! size of \p indices must be equal to that of the \p dims vector
    //! returned by GetDimensions()
    //!
    virtual float AccessIndex(const std::vector<size_t> &indices) const;

    //! Set the data value at the indicated grid point
    //!
    //! This method sets the data value of the grid point indexed by
    //! \p indices to \p value.
    //!
    virtual void SetValue(const std::vector<size_t> &indices, float value);

    //! This method provides an alternate interface to Grid::AccessIndex()
    //! If the dimensionality of the grid as determined by GetDimensions() is
    //! less than three subsequent parameters are ignored. Parameters
    //! that are outside of range are clamped to boundaries.
    //!
    //! \param[in] i Index into first fastest varying dimension
    //! \param[in] j Index into second fastest varying dimension
    //! \param[in] k Index into third fastest varying dimension
    //
    virtual float AccessIJK(size_t i, size_t j, size_t k) const;

    void SetValueIJK(size_t i, size_t j, size_t k, float v);

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
    //! \param[in] coords A vector of size matching the topology dimension
    //! of the mesh whose contents specify the coordinates of a point in space.
    //!
    //! \sa GetInterpolationOrder(), HasPeriodic(), GetMissingValue()
    //! \sa GetUserExtents()
    //!
    virtual float GetValue(const std::vector<double> &coords) const;

    virtual float GetValue(double x, double y) const {
        std::vector<double> coords = {x, y};
        return (GetValue(coords));
    }
    virtual float GetValue(double x, double y, double z) const {
        std::vector<double> coords = {x, y, z};
        return (GetValue(coords));
    }

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
    //! \sa GetDimensions(), Grid()
    //!
    virtual void GetUserExtents(
        std::vector<double> &minu, std::vector<double> &maxu) const = 0;

    //! Return the extents of the axis-aligned bounding box enclosign a region
    //!
    //! This pure virtual method returns min and max extents, in user coordinates,
    //! of the smallest axis-aligned box enclosing the region defined
    //! by the grid indices, \p min and \p max. Every grid point in
    //! the range \p min to \p max will be contained in, or reside on, the
    //! box (rectangle) whose extents are given by \p minu and \p maxu.
    //!
    //! \note The results are undefined if any index of \p min is
    //! greater than the coresponding coordinate of \p max, or if \p max
    //! is outside of the valid dimension range (See GetDimension()).
    //!
    //! The size of \p min and \p max must match the grid's dimension
    //! as returned by GetDimension()
    //!
    //! \param[in] min An array containing the minimum
    //! grid indices (offsets).
    //! \param[in] max An array containing the maximum
    //! grid indices (offsets).
    //! \param[out] minu A two-to-three element array containing the minimum
    //! user coordinates.
    //! \param[out] maxu A two-to-three element array containing the maximum
    //! user coordinates.
    //!
    //! \sa GetDimensions(), Grid()
    //!
    virtual void GetBoundingBox(
        const std::vector<size_t> &min, const std::vector<size_t> &max,
        std::vector<double> &minu, std::vector<double> &maxu) const = 0;

    //!
    //! Get bounding indices of grid containing a region
    //!
    //! Calculates the starting and ending grid indices of the
    //! smallest grid
    //! completely containing the rectangular region defined by the user
    //! coordinates \p minu and \p maxu
    //! If rectangluar region defined by \p minu and \p maxu can
    //! not be contained the
    //! minimum and maximum indices are returned in
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

    //! Set missing data flag
    //!
    //! Set this flag if missing values may exist in the data. This missing
    //! values are denoted by the value GetMissingValue(). Subsequent processing
    //! of grid data will be compared against the value of GetMissingValue()
    //! if this flag is set.
    //
    void SetHasMissingValues(bool flag) {
        _hasMissing = flag;
    }

    //! Return missing data flag
    //!
    //! This method returns true if the missing data flag is set.
    //! This does not
    //! imply that grid points exist with missing data, only that the
    //! the grid points should be compared against the value of
    //! GetMissingValue() whenever operations are performed on them.
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
    //! specified by \p indices
    //!
    //! Results are undefined if \p indices is out of range.
    //!
    //! \param[in] indices Array of grid indices.
    //! \param[out] coord User coordinates of grid point with indices
    //! given by \p indices.
    //!
    virtual void GetUserCoordinates(
        const std::vector<size_t> &indices,
        std::vector<double> &coords) const = 0;

    virtual void GetUserCoordinates(
        size_t i, double &x, double &y, double &z) const;
    virtual void GetUserCoordinates(
        size_t i, size_t j, double &x, double &y, double &z) const;
    virtual void GetUserCoordinates(
        size_t i, size_t j, size_t k, double &x, double &y, double &z) const;

    //! Return the closest grid point to the specified user coordinates
    //!
    //! This method returns the indices of the grid point closest to
    //! the specified user coordinates based on Euclidean distance. If any
    //! of the input coordinates correspond to periodic dimensions the
    //! the coordinate(s) are first re-mapped to lie inside the grid
    //! extents as returned by GetUserExtents()
    //!
    //! \param[in] coords User coordinates of grid point with indices
    //! given by \p indices.
    //! \param[out] indices Array of grid indices.
    //!
    virtual void GetIndices(
        const std::vector<double> &coords,
        std::vector<size_t> &indices) const = 0;

    //! Return the indices of the cell containing the
    //! specified user coordinates
    //!
    //! This method returns the cell ID (index) of the cell containing
    //! the specified user coordinates. If any
    //! of the input coordinates correspond to periodic dimensions the
    //! the coordinate(s) are first re-mapped to lie inside the grid
    //! extents as returned by GetUserExtents()
    //!
    //! If the specified coordinates lie outside of the grid (are not
    //! contained by any cell) the method returns false, otherwise true is
    //! returned.
    //!
    //! \retval status true on success, false if the point is not contained
    //! by any cell.
    //!
    virtual bool GetIndicesCell(
        const std::vector<double> &coords,
        std::vector<size_t> &indices) const = 0;

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
    //! \param[in] coords User coordinates of point in space
    //!
    //! \retval bool True if point is inside the grid
    //!
    virtual bool InsideGrid(const std::vector<double> &coords) const = 0;

    //! Get the indices of the nodes that define a cell
    //!
    //! This method returns a vector of index vectors. Each index vector
    //! contains the indices for a node that defines the given cell ID
    //! \p indices.
    //!
    //! For 2D cells the node IDs are returned in counter-clockwise order.
    //! For 3D cells the ordering is dependent on the shape of the node. TBD.
    //!
    //!
    //! \param[in] cindices An ordered vector of multi-dimensional cell
    //! indices.
    //! \param[out] nodes A vector of index vectors . Each index vector
    //! has size given by GetDimensions.size()
    //!
    virtual bool GetCellNodes(
        const std::vector<size_t> &cindices,
        std::vector<std::vector<size_t>> &nodes) const = 0;

    //! Get the IDs (indices) of all of the cells that border a cell
    //!
    //! This method returns a vector of index vectors. Each index vector
    //! contains the indices of a cell that borders the cell given by
    //! \p indices. If a
    //! cell edge is a boundary edge, having no neighbors, the associated
    //! index vector for that border will be empty.
    //! The cell IDs are returned in counter-clockwise order
    //!
    //! \param[in] cindices An ordered vector of multi-dimensional cell
    //! indices.
    //! \param[out] cells A vector of index vectors. Each index vector
    //! has size given by GetDimensions.size()
    //!
    virtual bool GetCellNeighbors(
        const std::vector<size_t> &cindices,
        std::vector<std::vector<size_t>> &cells) const = 0;

    //! Get the IDs (indices) of all of the cells that share a node
    //!
    //! This method returns a vector of index vectors. Each index vector
    //! contains the indices of a cell that share the node given by
    //! \p indices.
    //! The cell IDs are returned in counter-clockwise order
    //!
    //! \param[out] nodes A vector of index vectors . Each index vector
    //! has size given by GetDimensions.size()
    //!
    virtual bool GetNodeCells(
        const std::vector<size_t> &indices,
        std::vector<std::vector<size_t>> &cells) const = 0;

    virtual void ClampCoord(std::vector<double> &coords) const = 0;

    //! Set periodic boundaries
    //!
    //! This method changes the periodicity of boundaries set
    //! by the class constructor
    //!
    //! \param[in] periodic A boolean vector of size given by
    //! GetTopologyDim() indicating
    //! which coordinate axes are periodic.
    //
    virtual void SetPeriodic(const std::vector<bool> &periodic) {
        assert(periodic.size() == _topologyDimension);
        _periodic = periodic;
    }

    //! Check for periodic boundaries
    //!
    //
    virtual std::vector<bool> GetPeriodic() const {
        return (_periodic);
    }

    //! Return the topological dimension of the grid
    //!
    //! This method returns the number of topological dimensions, 2 or 3.
    //! The topological dimension determines the number of coordinates
    //! needed to describe the position of each node
    //
    virtual size_t GetTopologyDim() const {
        return (_topologyDimension);
    }

    //! Get the linear offset to the node IDs
    //!
    //! Return the smallest node ID. The default is zero
    //
    virtual size_t GetNodeOffset() const { return (_nodeIDOffset); }

    //! Get the linear offset to the cell IDs
    //!
    //! Return the smallest Cell ID. The default is zero
    //
    virtual size_t GetCellOffset() const { return (_cellIDOffset); }

    VDF_API friend std::ostream &operator<<(std::ostream &o, const Grid &g);

    /////////////////////////////////////////////////////////////////////////////
    //
    // Iterators
    //
    /////////////////////////////////////////////////////////////////////////////

    // Inside a box functor
    //
    // operator() returns true if point pt is on or inside the axis-aligned
    // box defined by min and max
    //
    class InsideBox {
      public:
        InsideBox(
            const std::vector<double> &min, const std::vector<double> &max) : _min(min), _max(max) {}
        InsideBox() {}

        bool operator()(const std::vector<double> &pt) const {
            for (int i = 0; i < _min.size(); i++) {
                if (pt[i] < _min[i] || pt[i] > _max[i])
                    return (false);
            }
            return (true);
        }

      private:
        std::vector<double> _min;
        std::vector<double> _max;
    };

    //
    // Define polymorphic iterator that can be used with any
    // class derived from this class
    //
    //

    // Interface for iterator specializations
    //
    template <typename T>
    class AbstractIterator {
      public:
        virtual ~AbstractIterator() {}
        virtual void next() = 0;
        virtual T &deref() const = 0;
        virtual const void *address() const = 0;
        virtual bool equal(const void *other) const = 0;
        virtual std::unique_ptr<AbstractIterator> clone() const = 0;
    };

    // Overloaded operators that will act on spealizations of
    // AbstractIterator
    //
    template <typename T>
    class PolyIterator {
      public:
        PolyIterator(std::unique_ptr<AbstractIterator<T>> it) : _impl(std::move(it)) {}

        PolyIterator(PolyIterator const &rhs) : _impl(rhs._impl->clone()) {}

        PolyIterator &operator=(PolyIterator const &rhs) {
            _impl = rhs._impl->clone();
            return *this;
        }

        // PolyIterator has a unique_ptr member so we must provide
        // std::move constructors
        //
        PolyIterator(PolyIterator &&rhs) {
            _impl = std::move(rhs._impl);
        }

        PolyIterator &operator=(PolyIterator &&rhs) {
            if (this != &rhs) {
                _impl = std::move(rhs._impl);
            }
            return (*this);
        }

        PolyIterator() : _impl(nullptr) {}

        PolyIterator &operator++() { // ++prefix
            _impl->next();
            return *this;
        };
        PolyIterator operator++(int) { // postfix++
            assert(false && "Not implemented");
            //return(*this);
            return (PolyIterator());
        };

        const T &operator*() const {
            return _impl->deref();
        }

        bool operator==(const PolyIterator &rhs) const {
            return (_impl->equal(rhs._impl->address()));
        }

        bool operator!=(const PolyIterator &rhs) const {
            return (!(*this == rhs));
        }

      private:
        std::unique_ptr<AbstractIterator<T>> _impl;
    };

    // Coordinate iterator. Iterates over grid node/cell coordinates
    //
    typedef const std::vector<double> ConstCoordType;
    typedef Grid::PolyIterator<ConstCoordType> ConstCoordItr;
    typedef Grid::AbstractIterator<ConstCoordType> ConstCoordItrAbstract;

    virtual ConstCoordItr ConstCoordBegin() const = 0;
    virtual ConstCoordItr ConstCoordEnd() const = 0;

    // Node index iterator. Iterates over grid node indices
    //
    typedef const std::vector<size_t> ConstIndexType;
    typedef Grid::PolyIterator<ConstIndexType> ConstNodeIterator;
    typedef Grid::AbstractIterator<ConstIndexType> ConstNodeIteratorAbstract;

    virtual ConstNodeIterator ConstNodeBegin() const = 0;
    virtual ConstNodeIterator ConstNodeBegin(
        const std::vector<double> &minu, const std::vector<double> &maxu) const = 0;
    virtual ConstNodeIterator ConstNodeEnd() const = 0;

    // Cell index iterator. Iterates over grid cell indices
    //
    typedef Grid::PolyIterator<ConstIndexType> ConstCellIterator;
    typedef Grid::AbstractIterator<ConstIndexType> ConstCellIteratorAbstract;

    virtual ConstCellIterator ConstCellBegin() const = 0;
    virtual ConstCellIterator ConstCellBegin(
        const std::vector<double> &minu, const std::vector<double> &maxu) const = 0;
    virtual ConstCellIterator ConstCellEnd() const = 0;

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
        ForwardIterator(
            T *rg,
            bool begin = true,
            const std::vector<double> &minu = {},
            const std::vector<double> &maxu = {});
        ForwardIterator();
        ForwardIterator(const ForwardIterator<T> &) = default;
        ForwardIterator(ForwardIterator<T> &&rhs);
        ~ForwardIterator() {}

        float &operator*() { return (*_itr); }

        ForwardIterator<T> &operator++(); // ++prefix

#ifdef DEAD
        ForwardIterator<T> operator++(int); // postfix++

        ForwardIterator<T> &operator+=(const long int &offset);
        ForwardIterator<T> operator+(const long int &offset) const;
#endif

        ForwardIterator<T> &operator=(ForwardIterator<T> rhs);
        ForwardIterator<T> &operator=(ForwardIterator<T> &rhs) = delete;

        bool operator==(const ForwardIterator<T> &rhs) {
            return (_index == rhs._index && _rg == rhs._rg);
        }
        bool operator!=(const ForwardIterator<T> &rhs) {
            return (!(*this == rhs));
        }

        const ConstCoordItr &GetCoordItr() {
            return (_coordItr);
        }

        friend void swap(
            Grid::ForwardIterator<T> &a,
            Grid::ForwardIterator<T> &b) {
            std::swap(a._rg, b._rg);
            std::swap(a._coordItr, b._coordItr);
            std::swap(a._index, b._index);
            std::swap(a._end_index, b._end_index);
            std::swap(a._xb, b._xb);
            std::swap(a._itr, b._itr);
            std::swap(a._pred, b._pred);
        }

      private:
        T *_rg;
        ConstCoordItr _coordItr;
        std::vector<size_t> _index;     // current index into grid
        std::vector<size_t> _end_index; // Last valid index
        size_t _xb;                     // x index within a block
        float *_itr;
        InsideBox _pred;
    };

    typedef Grid::ForwardIterator<Grid> Iterator;
    typedef Grid::ForwardIterator<Grid const> ConstIterator;

    //! Construct a begin iterator that will iterate through elements
    //! inside or on the box defined by \p minu and \p maxu
    //
    Iterator begin(
        const std::vector<double> &minu, const std::vector<double> &maxu) {
        return (Iterator(this, true, minu, maxu));
    }
    Iterator begin() { return (Iterator(this, true)); }

    Iterator end() { return (Iterator(this, false)); }

    ConstIterator cbegin(
        const std::vector<double> &minu, const std::vector<double> &maxu) const {
        return (ConstIterator(this, true, minu, maxu));
    }
    ConstIterator cbegin() const { return (ConstIterator(this, true)); }

    ConstIterator cend() const { return (ConstIterator(this, false)); }

  protected:
    virtual float GetValueNearestNeighbor(
        const std::vector<double> &coords) const = 0;

    virtual float GetValueLinear(
        const std::vector<double> &coords) const = 0;

    float *AccessIndex(
        const std::vector<float *> &blks, const std::vector<size_t> &indices) const;

    void SetNodeOffset(size_t offset) {
        _nodeIDOffset = offset;
    }

    void SetCellOffset(size_t offset) {
        _cellIDOffset = offset;
    }

  private:
    std::vector<size_t> _dims;  // dimensions of grid arrays
    std::vector<size_t> _bs;    // dimensions of each block
    std::vector<size_t> _bdims; // dimensions (specified in blocks) of ROI
    std::vector<float *> _blks;
    std::vector<bool> _periodic; // periodicity of boundaries
    size_t _topologyDimension;
    float _missingValue;
    bool _hasMissing;
    int _interpolationOrder; // Order of interpolation
    size_t _nodeIDOffset;
    size_t _cellIDOffset;

    virtual void _getUserCoordinatesHelper(
        const std::vector<double> &coords, double &x, double &y, double &z) const;
};
}; // namespace VAPoR
#endif
