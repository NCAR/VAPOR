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
    //! \param[in] node_dims Dimensions of grid nodes. The product of the
    //! elements of \p node_dims gives the total number of nodes in the mesh.
    //! If the rank of \p node_dims is greater than one the mesh is assumed to
    //! be structured along the slower varying dimensions (e.g. a layered mesh)
    //!
    //! \param[in] cell_dims Dimensions of grid cells. The product of the
    //! elements of \p cells gives the total number of cells in the mesh.
    //! If the rank of \p cells is greater than one the mesh is assumed to
    //! be structured along the slower varying dimensions (e.g. a layered mesh).
    //! Moreover, the slowest varying dimensions must be one less than the
    //! corresponding dimension of \p node_dims.
    //!
    //! \param[in] edge_dims Dimensions of grid edges. The product of the
    //! elements of \p edges gives the total number of edges in the mesh.
    //! If the rank of \p edges is greater than one the mesh is assumed to
    //! be structured along the slower varying dimensions (e.g. a layered mesh).
    //! Moreover, the slowest varying dimensions must be one less than the
    //! corresponding dimension of \p node_dims.
    //!
    //! \param[in] blks Grid data values. The location of the data values
    //! (node, cell, edge) is determined by \p location. If the dimensions
    //! (\p node_dims, etc) are multi-dimensional the size of \p blks must
    //! match the size of the slowest varying dimension. Each element of
    //! \p blks must point to an area of memory of size \b n elements, where
    //! \b n is the first element of \p node_dims, \p cell_dims, or \p edge_dims
    //! as indicated by \p location.
    //!
    //! \param[in] face_node_conn An array with dimensions:
    //!
    //! \p cell_dims[0] * max_nodes_per_face
    //! that provides for each cell the 1D node IDs of each corner node.
    //! If the number of corner nodes is less than \p max_nodes_per_face
    //! the missing node indices will be equal to GetMissingIndex();
    //! The ordering of the nodes is counter-clockwise.
    //!
    //! \param[in] face_face_conn An array with dimensions:
    //!
    //! \p cell_dims[0] * max_nodes_per_face
    //! that provides for each cell the 1D cell IDs of border cell
    //! If the number of corner nodes is less than \p max_nodes_per_face
    //! the missing node indices will be equal to GetMissingIndex(). If an
    //! edge is on the mesh boundary the index will be set to the value
    //! GetBoundaryIndex().
    //! The ordering of the neighboring faces is counter-clockwise.
    //!
    //! \param[in] location The location of grid data: at the nodes, edge-centered,
    //! or cell-centered
    //!
    //! \param[in] max_nodes_per_face The maxium number of nodes that a face
    //! may have.
    //
    UnstructuredGrid(
        const std::vector<size_t> &node_dims,
        const std::vector<size_t> &cell_dims,
        const std::vector<size_t> &edge_dims,
        const std::vector<float *> &blks,
        const size_t *face_node_conn,
        const size_t *face_face_conn,
        Location location, // node,face, edge
        size_t max_nodes_per_face

    );

    UnstructuredGrid() = default;
    virtual ~UnstructuredGrid() = default;

    //! \copydoc Grid::AccessIndex()
    //
    float AccessIndex(const std::vector<size_t> &indices) const;

    //! \copydoc Grid::SetValue()
    //
    void SetValue(const std::vector<size_t> &indices, float value);

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

    void SetValueIJK(size_t i, size_t j, size_t k, float v);

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

    //
    // Define polymorphic iterator that can be used with any
    // class derived from this class
    //
    //

    // Interface for iterator specializations
    //
    class ConstCoordItrAbstract {
      public:
        virtual ~ConstCoordItrAbstract() {}
        virtual void next() = 0;
        virtual const std::vector<double> &deref() const = 0;
        virtual const void *address() const = 0;
        virtual bool equal(const void *other) const = 0;
        virtual std::unique_ptr<ConstCoordItrAbstract> clone() const = 0;
    };

    // Overloaded operators that will act on spealizations of
    // ConstCoordItrAbstract
    //
    class ConstCoordItr {
      public:
        ConstCoordItr(std::unique_ptr<ConstCoordItrAbstract> it) : _impl(std::move(it)) {}

        ConstCoordItr(ConstCoordItr const &rhs) : _impl(rhs._impl->clone()) {}

        ConstCoordItr &operator=(ConstCoordItr const &rhs) {
            _impl = rhs._impl->clone();
            return *this;
        }

        // ConstCoordItr has a unique_ptr member so we must provide
        // std::move constructors
        //
        ConstCoordItr(ConstCoordItr &&rhs) {
            _impl = std::move(rhs._impl);
        }

        ConstCoordItr &operator=(ConstCoordItr &&rhs) {
            if (this != &rhs) {
                _impl = std::move(rhs._impl);
            }
            return (*this);
        }

        ConstCoordItr() : _impl(nullptr) {}

        ConstCoordItr &operator++() { // ++prefix
            _impl->next();
            return *this;
        };
        ConstCoordItr operator++(int) { // postfix++
            assert(false && "Not implemented");
            //return(*this);
            return (ConstCoordItr());
        };

        const std::vector<double> &operator*() const {
            return _impl->deref();
        }

        bool operator==(const ConstCoordItr &rhs) const {
            return (_impl->equal(rhs._impl->address()));
        }

        bool operator!=(const ConstCoordItr &rhs) const {
            return (!(*this == rhs));
        }

      private:
        std::unique_ptr<ConstCoordItrAbstract> _impl;
    };

    virtual ConstCoordItr ConstCoordBegin() const = 0;
    virtual ConstCoordItr ConstCoordEnd() const = 0;

    // Inside a box functor
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

    //! A forward iterator for accessing the data elements of the
    //! unstructured grid.
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
            const std::vector<double> &minu = {},
            const std::vector<double> &maxu = {});
        ForwardIterator();
        ForwardIterator(const ForwardIterator<T> &) = default;
        ForwardIterator(ForwardIterator<T> &&rhs);
        ~ForwardIterator() {}

        inline float &operator*() { return (*_itr); }

        ForwardIterator<T> &operator++(); // ++prefix

#ifdef DEAD
        ForwardIterator<T> operator++(int); // postfix++

        ForwardIterator<T> &operator+=(const long int &offset);
        ForwardIterator<T> operator+(const long int &offset) const;
#endif

        ForwardIterator<T> &operator=(ForwardIterator<T> rhs);
        ForwardIterator<T> &operator=(ForwardIterator<T> &rhs) = delete;

        bool operator==(const ForwardIterator<T> &other);
        bool operator!=(const ForwardIterator<T> &other);

        const ConstCoordItr &GetCoordItr() {
            return (_coordItr);
        }

        friend void swap(
            UnstructuredGrid::ForwardIterator<T> &a,
            UnstructuredGrid::ForwardIterator<T> &b) {
            std::swap(a._rg, b._rg);
            std::swap(a._coordItr, b._coordItr);
            std::swap(a._x, b._x);
            std::swap(a._y, b._y);
            std::swap(a._z, b._z);
            std::swap(a._xb, b._xb);
            std::swap(a._itr, b._itr);
            for (int i = 0; i < 3; i++) {
                std::swap(a._dims[i], b._dims[i]);
                std::swap(a._bs[i], b._bs[i]);
                std::swap(a._bdims[i], b._bdims[i]);
            }
            std::swap(a._ndim, b._ndim);
            std::swap(a._end, b._end);
            std::swap(a._pred, b._pred);
        }

      private:
        T *_rg;
        ConstCoordItr _coordItr;
        size_t _x, _y, _z; // current index into _rg->_min[3]
        size_t _xb;        // x index within a block
        float *_itr;
        size_t _dims[3];
        size_t _bs[3];
        size_t _bdims[3];
        int _ndim;
        bool _end;
        InsideBox _pred;
    };

    typedef UnstructuredGrid::ForwardIterator<UnstructuredGrid> Iterator;
    typedef UnstructuredGrid::ForwardIterator<UnstructuredGrid const> ConstIterator;

    //! Construct a begin iterator that will iterate through elements
    //! inside or on the box defined by \p minu and \p maxu
    //
    Iterator begin(
        const std::vector<double> &minu, const std::vector<double> &maxu) {
        return (Iterator(this, minu, maxu));
    }
    Iterator begin() { return (Iterator(this)); }

    Iterator end() { return (Iterator()); }

    ConstIterator cbegin(
        const std::vector<double> &minu, const std::vector<double> &maxu) const {
        return (ConstIterator(this, minu, maxu));
    }
    ConstIterator cbegin() const { return (ConstIterator(this)); }

    ConstIterator cend() const { return (ConstIterator()); }

    //! A forward iterator for accessing the cell IDs
    //! of a unstructured grid.
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
            UnstructuredGrid::ForwardCellIterator<T> &a,
            UnstructuredGrid::ForwardCellIterator<T> &b) {
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

    typedef UnstructuredGrid::ForwardCellIterator<UnstructuredGrid const> ConstCellIterator;

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
    //! of a unstructured grid.
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
            UnstructuredGrid::ForwardNodeIterator<T> &a,
            UnstructuredGrid::ForwardNodeIterator<T> &b) {
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

    typedef UnstructuredGrid::ForwardNodeIterator<UnstructuredGrid const> ConstNodeIterator;

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

    VDF_API friend std::ostream &operator<<(std::ostream &o, const UnstructuredGrid &sg);

  protected:
    float *_AccessIndex(
        const std::vector<float *> &blks, const std::vector<size_t> &indices) const;

  private:
    std::vector<size_t> _bs;    // dimensions of each block
    std::vector<size_t> _bdims; // dimensions (specified in blocks) of ROI
    std::vector<float *> _blks;
    const std::vector<size_t *> _face_node_conn;
    const std::vector<size_t *> _face_face_conn;
    size_t _max_nodes_per_face;
};
}; // namespace VAPoR
#endif
