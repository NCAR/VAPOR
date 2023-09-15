#pragma once

#include <vector>
#include <iostream>
#pragma once

#include <cstdint>
#include <vapor/VAssert.h>
#include <vapor/Grid.h>
#include <vapor/QuadTreeRectangle.hpp>

using UInt32_tArr2 = std::array<uint32_t, 2>;
using pType = UInt32_tArr2;

namespace VAPoR {

//
//! \class QuadTreeRectangleP
//! \brief This class wraps QuadTreeRectangleP with parallel
//! tree construction
//!
//
class QuadTreeRectangleP {
public:
    //! \copydoc QuadTreeRectangle::QuadTreeRectangle()
    //
    QuadTreeRectangleP(float left, float top, float right, float bottom, size_t max_depth = 12, size_t reserve_size = 1000);

    //! \copydoc QuadTreeRectangle::QuadTreeRectangle()
    //
    QuadTreeRectangleP(size_t max_depth = 12, size_t reserve_size = 1000);

    //! \copydoc QuadTreeRectangle::QuadTreeRectangle()
    //
    QuadTreeRectangleP(const QuadTreeRectangleP &rhs);

    //! \copydoc QuadTreeRectangle::QuadTreeRectangle()
    //
    QuadTreeRectangleP &operator=(const QuadTreeRectangleP &rhs);

    ~QuadTreeRectangleP();

    //! \copydoc QuadTreeRectangle::Insert()
    //
    bool Insert(float left, float top, float right, float bottom, DimsType payload);

    //! Parallel tree creation
    //!
    //! Inserts multiple rectangles into the quad tree in parallel
    //!
    //! \sa QuadTreeRectangle::Insert()
    //
    bool Insert(std::vector<class QuadTreeRectangle<float, pType>::rectangle_t> rectangles, std::vector<pType> payloads);

    //! Constructs a quadtree from the cells contained in a Grid class
    //!
    //! This method iterates over all of the cells found in \p grid and
    //! constructs a Quadtree. The construction is performed in parallel.
    //! The topological dimesion of \p grid must be two.
    //!
    //! \param[in] grid The grid from which to construct the tree
    //! \param[in] ncells If non-zero specifies the number of cells to
    //! insert via iterating over the cells contained in the grid. If zero,
    //! all of the cells are inserted.
    //!
    bool Insert(const Grid *grid, size_t ncells = 0);

    //! \copydoc QuadTreeRectangle::GetPayloadContained()
    //
    void GetPayloadContained(float x, float y, std::vector<DimsType> &payloads) const;

    //! \copydoc QuadTreeRectangle::GetStats()
    //
    void GetStats(std::vector<size_t> &payload_histo, std::vector<size_t> &level_histo) const;

    friend std::ostream &operator<<(std::ostream &os, const QuadTreeRectangleP &q)
    {
        for (int i = 0; i < q._qtrs.size(); i++) {
            os << "Bin " << i << std::endl;
            os << q._qtrs[i] << std::endl;
        }
        return (os);
    }

private:
    float                                          _left;
    float                                          _right;
    std::vector<QuadTreeRectangle<float, pType> *> _qtrs;
};
};    // namespace VAPoR
