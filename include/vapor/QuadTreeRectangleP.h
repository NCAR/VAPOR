#pragma once

#include <vector>
#include <iostream>
#include <cstdint>
#include <vapor/VAssert.h>
#include <vapor/QuadTreeRectangle.hpp>

namespace VAPoR {

//
//! \class QuadTreeRectanglePP
//! \brief This class wraps QuadTreeRectangleP with parallel
//! tree construction
//!
//
template<typename T, typename S> class QuadTreeRectangleP {
public:
    QuadTreeRectangleP(T left, T top, T right, T bottom, size_t max_depth = 12, size_t reserve_size = 1000);

    QuadTreeRectangleP(size_t max_depth = 12, size_t reserve_size = 1000);

    QuadTreeRectangleP(const QuadTreeRectangleP &rhs);

    QuadTreeRectangleP &operator=(const QuadTreeRectangleP &rhs);

    ~QuadTreeRectangleP();

    bool Insert(T left, T top, T right, T bottom, S payload);

    bool Insert(std::vector<class QuadTreeRectangle<T, S>::rectangle_t> rectangles, std::vector<S> payloads);

    void GetPayloadContained(T x, T y, std::vector<S> &payloads) const;

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
    T                                      _left;
    T                                      _right;
    std::vector<QuadTreeRectangle<T, S> *> _qtrs;
};
};    // namespace VAPoR
