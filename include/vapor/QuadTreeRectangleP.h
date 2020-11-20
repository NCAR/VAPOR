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
//! \class QuadTreeRectanglePP
//! \brief This class wraps QuadTreeRectangleP with parallel
//! tree construction
//!
//
class QuadTreeRectangleP {
public:
    QuadTreeRectangleP(float left, float top, float right, float bottom, size_t max_depth = 12, size_t reserve_size = 1000);

    QuadTreeRectangleP(size_t max_depth = 12, size_t reserve_size = 1000);

    QuadTreeRectangleP(const QuadTreeRectangleP &rhs);

    QuadTreeRectangleP &operator=(const QuadTreeRectangleP &rhs);

    ~QuadTreeRectangleP();

    bool Insert(float left, float top, float right, float bottom, Size_tArr3 payload);

    bool Insert(std::vector<class QuadTreeRectangle<float, pType>::rectangle_t> rectangles, std::vector<pType> payloads);

    bool Insert(const Grid *grid);

    void GetPayloadContained(float x, float y, std::vector<Size_tArr3> &payloads) const;

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
