#ifndef _StructuredGrid_
#define _StructuredGrid_

#include <ostream>
#include <vector>
#include <memory>
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
    StructuredGrid(
        const std::vector<size_t> &dims,
        const std::vector<size_t> &bs,
        const std::vector<float *> &blks);

    StructuredGrid() = default;
    virtual ~StructuredGrid() = default;

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

    virtual void ClampCoord(std::vector<double> &coords) const;

    //! A forward iterator for accessing the cell IDs
    //! of a structured grid.
    //!
    //! This class provides a C++ STL style Forward Iterator for
    //! accessing grid cell IDs.
    //! Iterator expressions are supported.
    //!
    //
    template <class T>
    class VDF_API ForwardCellIterator {
      public:
        ForwardCellIterator(
            T *sg,
            const std::vector<double> &minu = {},
            const std::vector<double> &maxu = {});
        ForwardCellIterator(T *sg, bool begin);
        ForwardCellIterator();
        ForwardCellIterator(const ForwardCellIterator<T> &) = default;
        ForwardCellIterator(ForwardCellIterator<T> &&rhs);
        ~ForwardCellIterator() {}

        std::vector<size_t> &operator*() { return (_cellIndex); }

        ForwardCellIterator<T> &operator++(); // ++prefix

#ifdef DEAD
        ForwardCellIterator<T> operator++(int); // postfix++

        ForwardCellIterator<T> &operator+=(const long int &offset);
        ForwardCellIterator<T> operator+(const long int &offset) const;
#endif

        ForwardCellIterator<T> &operator=(ForwardCellIterator<T> rhs);
        ForwardCellIterator<T> &operator=(ForwardCellIterator<T> &rhs) = delete;

        bool operator==(const ForwardCellIterator<T> &rhs) {
            return (_cellIndex == rhs._cellIndex);
        }
        bool operator!=(const ForwardCellIterator<T> &rhs) {
            return (!(*this == rhs));
        }

        friend void swap(
            StructuredGrid::ForwardCellIterator<T> &a,
            StructuredGrid::ForwardCellIterator<T> &b) {
#ifdef DEAD
            std::swap(a._coordItr0, b._coordItr0);
            std::swap(a._coordItr1, b._coordItr1);
            std::swap(a._coordItr2, b._coordItr2);
            std::swap(a._coordItr3, b._coordItr3);
            std::swap(a._coordItr4, b._coordItr4);
            std::swap(a._coordItr5, b._coordItr5);
            std::swap(a._coordItr6, b._coordItr6);
            std::swap(a._coordItr7, b._coordItr7);
            std::swap(a._pred, b._pred);
#endif
            std::swap(a._sg, b._sg);
            std::swap(a._dims, b._dims);
            std::swap(a._cellIndex, b._cellIndex);
        }

      private:
#ifdef DEAD
        InsideBox _pred;
        ConstCoordItr _coordItr0;
        ConstCoordItr _coordItr1;
        ConstCoordItr _coordItr2;
        ConstCoordItr _coordItr3;
        ConstCoordItr _coordItr4;
        ConstCoordItr _coordItr5;
        ConstCoordItr _coordItr6;
        ConstCoordItr _coordItr7;
#endif
        T *_sg;
        std::vector<size_t> _dims;
        std::vector<size_t> _cellIndex;

        ForwardCellIterator<T> &next2d();
        ForwardCellIterator<T> &next3d();
    };

    typedef StructuredGrid::ForwardCellIterator<StructuredGrid const> ConstCellIterator;

#ifdef DEAD
    //! Construct a begin iterator that will iterate through elements
    //! inside or on the box defined by \p minu and \p maxu
    //
    CellIterator ConstCellBegin(
        const std::vector<double> &minu, const std::vector<double> &maxu) {
        return (CellIterator(this, minu, maxu));
    }
#endif

    ConstCellIterator ConstCellBegin() const {
        return (ConstCellIterator(this, true));
    }

    ConstCellIterator ConstCellEnd() const {
        return (ConstCellIterator(this, false));
    }

    //! A forward iterator for accessing the node IDs
    //! of a structured grid.
    //!
    //! This class provides a C++ STL style Forward Iterator for
    //! accessing grid node IDs.
    //! Iterator expressions are supported.
    //!
    //
    template <class T>
    class VDF_API ForwardNodeIterator {
      public:
        ForwardNodeIterator(
            T *sg,
            const std::vector<double> &minu = {},
            const std::vector<double> &maxu = {});
        ForwardNodeIterator(T *sg, bool begin);
        ForwardNodeIterator();
        ForwardNodeIterator(const ForwardNodeIterator<T> &) = default;
        ForwardNodeIterator(ForwardNodeIterator<T> &&rhs);
        ~ForwardNodeIterator() {}

        std::vector<size_t> &operator*() { return (_nodeIndex); }

        ForwardNodeIterator<T> &operator++(); // ++prefix

#ifdef DEAD
        ForwardNodeIterator<T> operator++(int); // postfix++

        ForwardNodeIterator<T> &operator+=(const long int &offset);
        ForwardNodeIterator<T> operator+(const long int &offset) const;
#endif

        ForwardNodeIterator<T> &operator=(ForwardNodeIterator<T> rhs);
        ForwardNodeIterator<T> &operator=(ForwardNodeIterator<T> &rhs) = delete;

        bool operator==(const ForwardNodeIterator<T> &rhs) {
            return (_nodeIndex == rhs._nodeIndex);
        }
        bool operator!=(const ForwardNodeIterator<T> &rhs) {
            return (!(*this == rhs));
        }

        friend void swap(
            StructuredGrid::ForwardNodeIterator<T> &a,
            StructuredGrid::ForwardNodeIterator<T> &b) {
            std::swap(a._sg, b._sg);
            std::swap(a._dims, b._dims);
            std::swap(a._nodeIndex, b._nodeIndex);
        }

      private:
#ifdef DEAD
        InsideBox _pred;
#endif
        T *_sg;
        std::vector<size_t> _dims;
        std::vector<size_t> _nodeIndex;

        ForwardNodeIterator<T> &next2d();
        ForwardNodeIterator<T> &next3d();
    };

    typedef StructuredGrid::ForwardNodeIterator<StructuredGrid const> ConstNodeIterator;

#ifdef DEAD
    //! Construct a begin iterator that will iterate through elements
    //! inside or on the box defined by \p minu and \p maxu
    //
    NodeIterator ConstNodeBegin(
        const std::vector<double> &minu, const std::vector<double> &maxu) {
        return (NodeIterator(this, minu, maxu));
    }
#endif

    ConstNodeIterator ConstNodeBegin() const {
        return (ConstNodeIterator(this, true));
    }

    ConstNodeIterator ConstNodeEnd() const {
        return (ConstNodeIterator(this, false));
    }

    VDF_API friend std::ostream &operator<<(std::ostream &o, const StructuredGrid &sg);

  protected:
  private:
};
}; // namespace VAPoR
#endif
