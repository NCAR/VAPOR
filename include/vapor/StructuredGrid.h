#ifndef _StructuredGrid_
#define _StructuredGrid_

#include <ostream>
#include <vector>
#include <vapor/common.h>
#include <vapor/Grid.h>

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
    //!
    //! Construct a structured grid sampling a 3D or 2D scalar function
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
    //! \param[in] bs A two or three-element vector specifying the dimensions of
    //! each block storing the sampled scalar function.
    //! \param[in] min A two or three-element vector specifying the ijk index
    //! of the first point in the grid. The first grid point need not coincide
    //! with
    //! block boundaries. I.e. the indecies need not be (0,0,0): the first
    //! grid point is not required to be the first element of the array.
    //! \param[in] max A two or three-element vector specifying the ijk index
    //! of the last point in the grid
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
        const std::vector<size_t> &dims,
        const std::vector<size_t> &bs,
        const std::vector<float *> &blks);

    StructuredGrid();

    virtual ~StructuredGrid();

    //! \copydoc Grid::AccessIndex()
    //
    float AccessIndex(const std::vector<size_t> &indices) const;

    //! Return value of grid at specified location
    //!
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
        for (int i = 0; i < _bdims.size(); i++)
            bdims.push_back(_bdims[i]);
    }
    std::vector<size_t> GetDimensionInBlks() const {
        return (_bdims);
    }

    //! \copydoc Grid::GetRange()
    //!
    virtual void GetRange(float range[2]) const;

    //! \copydoc Grid::GetCellNodes()
    //!
    virtual bool GetCellNodes(
        const std::vector<size_t> &cindices,
        std::vector<std::vector<size_t>> &nodes) const;

    //! \copydoc Grid::GetCellNeighbors()
    //!
    virtual bool GetCellNeighbors(
        const std::vector<size_t> &cindices,
        std::vector<std::vector<size_t>> &cells) const;

    //! \copydoc Grid::GetNodeCells()
    //!
    virtual bool GetNodeCells(
        const std::vector<size_t> &indices,
        std::vector<std::vector<size_t>> &cells) const;

    //! Return the internal blocking factor
    //!
    //! This method returns the internal blocking factor passed
    //! to the constructor.
    //
    void GetBlockSize(std::vector<size_t> &bs) const {
        bs = _bs;
    }
    std::vector<size_t> GetBlockSize() const {
        return (_bs);
    }

    //! Return the internal data structure containing a copy of the blocks
    //! passed in by the constructor
    //!
    const std::vector<float *> &GetBlks() const { return (_blks); };

    virtual void ClampCoord(std::vector<double> &coords) const;

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

        void GetUserCoordinates(std::vector<double> &coords) const {
            coords.clear();
            std::vector<size_t> indices;
            indices.push_back(_x);
            indices.push_back(_y);
            indices.push_back(_z);
            return (_rg->GetUserCoordinates(indices, coords));
        };

      private:
        T *_rg;
        size_t _x, _y, _z; // current index into _rg->_min[3]
        size_t _xb;        // x index within a block
        float *_itr;
        size_t _max[3];
        size_t _bs[3];
        size_t _bdims[3];
        int _ndim;
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
    float _AccessIndex(
        const std::vector<float *> &blks, const std::vector<size_t> &indices) const;

  private:
    std::vector<size_t> _bs;    // dimensions of each block
    std::vector<size_t> _bdims; // dimensions (specified in blocks) of ROI
    std::vector<float *> _blks;

    void _StructuredGrid(
        const std::vector<size_t> &dims,
        const std::vector<size_t> &bs,
        const std::vector<float *> &blks);
};
}; // namespace VAPoR
#endif
