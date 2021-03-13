#pragma once

#include <vector>
#include <cassert>
#include <cmath>
#include <iostream>
#include <cstdint>
#include <vapor/VAssert.h>

namespace VAPoR {

// Maximum aspect ratio of a rectangle before it is split
//
const float maxAspectRatio = 2.0;

//
//! \class QuadTreeRectangle
//! \brief This class implements a 2D quad tree space partitioning tree
//! that operates on rectangular regions.
//!
//
template<typename T, typename S> class QuadTreeRectangle {
public:
    class rectangle_t {
    public:
        rectangle_t() : _left(0.0f), _top(0.0f), _right(0.0f), _bottom(0.0f) {}
        rectangle_t(T x1, T y1, T x2, T y2) : _left(x1), _top(y1), _right(x2), _bottom(y2) {}

        // return true iff other intersects us
        //
        bool intersects(rectangle_t const &other) const
        {
            if (_left > other._right || _top > other._bottom) return (false);

            if (_right < other._left || _bottom < other._top) return (false);
            return (true);
        }

        // return true iff other is completely contained inside or on boundary
        //
        bool contains(rectangle_t const &other) const { return ((_left <= other._left) && (_right >= other._right) && (_top <= other._top) && (_bottom >= other._bottom)); }

        // return true iff point(x,y) is completely contained inside or on boundary
        //
        bool contains(T x, T y) const { return ((_left <= x) && (_right >= x) && (_top <= y) && (_bottom >= y)); }

        bool touches(rectangle_t const &other) const { return ((_left == other._right) || (_right == other._left) || (_top == other._bottom) || (_bottom == other._top)); }

        T width() const { return (_right - _left); }
        T height() const { return (_bottom - _top); }

        // return the sub-rectangle for the specified quadrant
        //
        rectangle_t quadrant(uint32_t n)
        {
            T const center_x((_left + _right) / 2);
            T const center_y((_top + _bottom) / 2);
            switch (n & 0x03) {
            case 0: return rectangle_t(_left, _top, center_x, center_y);
            case 1: return rectangle_t(center_x, _top, _right, center_y);
            case 2: return rectangle_t(_left, center_y, center_x, _bottom);
            case 3: return rectangle_t(center_x, center_y, _right, _bottom);
            }
            VAssert(0);
            return *this;    // Can't happen since we mask n
        }

        // Horizontal aspect ratio
        //
        float hAspectRatio() const { return (std::fabs((_right - _left) / (_bottom - _top))); }

        friend std::ostream &operator<<(std::ostream &os, const rectangle_t &rec)
        {
            os << "left-top, right-bottom : "
               << "(" << rec._left << ", " << rec._top << ") "
               << "(" << rec._right << ", " << rec._bottom << ")" << std::endl;
            return (os);
        }

        T _left, _top, _right, _bottom;
    };

    //! Construct a QuadTreeRectangle instance for a defined 2D region
    //!
    //! This contstructor initiates a 2D quad tree with specified min
    //! and max bounds. Subsequent insertions into the tree will only
    //! succeed for regions that intersect the tree bounds
    //!
    //! \param[in] left Minimum X coordinate bound.
    //! \param[in] top Minimum Y coordinate bound.
    //! \param[in] right Maximum X coordinate bound. Must be greater than
    //! or equal to \p left.
    //! \param[in] bottom Maximum Y coordinate bound. Must be greater than
    //! or equal to \p top.
    //! \param[in] max_depth The maximum permitted depth of the tree. The
    //! tree will not be refined beyond \p max_depth levels.
    //! \param[in] reserve_size A hint indicating the antcipated number of
    //! nodes in the tree. Accurate estimates will increase performance
    //! of tree insertions
    //
    QuadTreeRectangle(T left, T top, T right, T bottom, size_t max_depth = 12, size_t reserve_size = 1000)
    {
        VAssert(left <= right);
        VAssert(top <= bottom);
        _nodes.reserve(reserve_size);
        _nodes.push_back(node_t(left, top, right, bottom));
        _rootidx = 0;
        _maxDepth = max_depth;
    }

    //! Construct a QuadTreeRectangle instance for a unit 2D region
    //!
    //! Default contructor definining a quad tree covering the region
    //! (.0, .0) to (1. ,1.)
    //!
    QuadTreeRectangle(size_t max_depth = 12, size_t reserve_size = 1000)
    {
        _nodes.reserve(reserve_size);
        _nodes.push_back(node_t(0.0, 0.0, 1.0, 1.0));
        _rootidx = 0;
        _maxDepth = max_depth;
    }

    QuadTreeRectangle(const QuadTreeRectangle &rhs)
    {
        _nodes.resize(rhs._nodes.size());
        for (size_t i = 0; i < rhs._nodes.size(); i++) { _nodes[i] = node_t((rhs._nodes[i])); }
        _rootidx = rhs._rootidx;
        _maxDepth = rhs._maxDepth;
    }

    QuadTreeRectangle &operator=(const QuadTreeRectangle &rhs)
    {
        if (*this == rhs) return *this;

        _nodes.resize(rhs._nodes.size());
        for (size_t i = 0; i < rhs._nodes.size(); i++) { _nodes[i] = node_t((rhs._nodes[i])); }
        _rootidx = rhs._rootidx;
        _maxDepth = rhs._maxDepth;
        return *this;
    }

    ~QuadTreeRectangle() { _nodes.clear(); }

    //! Insert an element into the tree
    //!
    //! This method inserts a payload, \p payload, into the tree contained
    //! in the rectangular region defined by \p left, \p top, \p right,
    //! \p bottom. If the region to be inserted does not intersect
    //! the tree bound defined by the contructor method fails and returns
    //! false. Otherwise, the tree is subdivided as necessary and the defined
    //! region along with its payload are inserted. The refinement algorithm
    //! for subdividing the tree operates as follows: The tree nodes that
    //! intersect the region are subdivided until both the width and height
    //! of the region to be inserted are larger than the respective
    //! width and height of the node intersecting the region, or the
    //! maximum depth of the tree is reached.
    //!
    //! \retval status Return true on success, or false if region to be inserted
    //! does not overlap the region managed by the tree.
    //
    bool Insert(const rectangle_t &rectangle, const S &payload)
    {
        if (!_nodes[_rootidx].intersects(rectangle)) return (false);

        float ar = rectangle.hAspectRatio();
        if (ar <= maxAspectRatio && ar >= (1.0 / maxAspectRatio)) {
            return (node_t::insert(_nodes, _rootidx, rectangle, payload, _maxDepth));
        } else if (ar >= maxAspectRatio) {
            // Horizontal split
            //
            bool   status = true;
            size_t n = (size_t)ar;
            float  split_width = (rectangle._right - rectangle._left) / (float)n;
            float  splitLeft = rectangle._left;
            for (int i = 0; i < n; i++) {
                float splitRight = splitLeft + split_width;
                if (i == (n-1)) splitRight = rectangle._right;

                status &= node_t::insert(_nodes, _rootidx, rectangle_t(splitLeft, rectangle._top, splitRight, rectangle._bottom), payload, _maxDepth);
                splitLeft = splitRight;
            }
            return (status);
        } else {
            // Vertical split
            //
            bool   status = true;
            size_t n = (size_t) (1.0 / ar);
            float  split_width = (rectangle._bottom - rectangle._top) / (float)n;
            float  splitTop = rectangle._top;
            for (int i = 0; i < n; i++) {
                float splitBottom = splitTop + split_width;
                if (i == (n-1)) splitBottom = rectangle._bottom;

                status &= node_t::insert(_nodes, _rootidx, rectangle_t(rectangle._left, splitTop, rectangle._right, splitBottom), payload, _maxDepth);
                splitTop = splitBottom;
            }
            return (status);
        }
    }

    bool Insert(T left, T top, T right, T bottom, S payload) { return (Insert(rectangle_t(left, top, right, bottom), payload)); }

    //! Return a list of payloads that intersect a specified point
    //!
    //! This method searches the tree for all nodes whose associated regions
    //! intersect the point (\p x, \p y), and returns any payload found
    //! at those nodes.
    //!
    //! \p param[in] x X coordinate of point
    //! \p param[in] y Y coordinate of point
    //! \p payloads[out] A vector of payloads whose regions intersect
    //! \p x and \p y.
    //!
    void GetPayloadContained(T x, T y, std::vector<S> &payloads) const
    {
        payloads.clear();

        node_t::get_payload_contains(_nodes, _rootidx, x, y, payloads);
    }

    //! Return informational statistics about the current tree
    //!
    //! This method returns stats about the tree
    //!
    //! \param[out] payload_histo Returns a histogram in the form of a vector
    //! that gives a count of the number of payloads. For example,
    //! the ith element of \p payload_histo provides the count of nodes
    //! that contain i number of payloads.
    //!
    //! \param[out] level_histo Returns a histogram in the form of a vector
    //! that gives a count of cells at each level in the tree.
    //
    void GetStats(std::vector<size_t> &payload_histo, std::vector<size_t> &level_histo) const
    {
        payload_histo.clear();
        level_histo.clear();

        for (size_t i = 0; i < _nodes.size(); i++) {
            size_t b = _nodes[i].get_payloads().size();
            if (b >= payload_histo.size()) { payload_histo.resize(b + 1, 0); }
            payload_histo[b] += 1;

            b = _nodes[i].get_level();
            if (b >= level_histo.size()) { level_histo.resize(b + 1, 0); }
            level_histo[b] += 1;
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const QuadTreeRectangle &q)
    {
        os << "Num nodes : " << q._nodes.size() << std::endl;
        const node_t &root = q._nodes[q._rootidx];
        root.print(q._nodes, q._rootidx, os);
        return (os);
    }

private:
    class node_t {
    public:
        node_t(int level = 0) : _level(level), _is_leaf(true), _child0(0), _rectangle(0.0, 0.0, 1.0, 1.0) {}

        node_t(T left, T top, T right, T bottom, int level = 0) : _level(level), _is_leaf(true), _child0(0), _rectangle(left, top, right, bottom) {}

        node_t(const rectangle_t &rec, int level = 0) : _level(level), _is_leaf(true), _child0(0), _rectangle(rec) {}

        rectangle_t &      bounds() { return (_rectangle); }
        rectangle_t const &bounds() const { return (_rectangle); }

        bool intersects(rectangle_t const &other) const { return (_rectangle.intersects(other)); }
        bool contains(rectangle_t const &other) const { return (_rectangle.contains(other)); }
        bool contains(T x, T y) const { return (_rectangle.contains(x, y)); }
        bool touches(rectangle_t const &other) const { return (_rectangle.touches(other)); }

        static void subdivide(std::vector<node_t> &nodes, size_t nidx)
        {
            node_t &node = nodes[nidx];

            if (!node._is_leaf) return;

            node._is_leaf = false;
            node._child0 = nodes.size();

            node_t n0(node._rectangle.quadrant(0), node._level + 1);
            node_t n1(node._rectangle.quadrant(1), node._level + 1);
            node_t n2(node._rectangle.quadrant(2), node._level + 1);
            node_t n3(node._rectangle.quadrant(3), node._level + 1);

            nodes.push_back(n0);
            nodes.push_back(n1);
            nodes.push_back(n2);
            nodes.push_back(n3);
        }

        static size_t quadrant(const std::vector<node_t> &nodes, size_t nidx, uint32_t n)
        {
            const node_t &node = nodes[nidx];
            switch (n & 0x03) {
            case 0: return node._child0 + 0;
            case 1: return node._child0 + 1;
            case 2: return node._child0 + 2;
            case 3: return node._child0 + 3;
            }
            VAssert(0);
            return node._child0 + 0;
        }
        static size_t quadrant(std::vector<node_t> &nodes, size_t nidx, uint32_t n)
        {
            const node_t &node = nodes[nidx];
            VAssert(node._child0 < nodes.size());
            switch (n & 0x03) {
            case 0: return node._child0 + 0;
            case 1: return node._child0 + 1;
            case 2: return node._child0 + 2;
            case 3: return node._child0 + 3;
            }
            VAssert(0);
            return node._child0 + 0;
        }

        static bool insert(std::vector<node_t> &nodes, size_t nidx, const rectangle_t &rec, S payload, size_t maxDepth)
        {
            if (!nodes[nidx]._rectangle.intersects(rec)) return (false);

            // if rec is larger than a quadrant (half the width and height of this
            // node) there is no point in refining. I.e. stop descending the
            // tree and store the payload here.
            //
            if (nodes[nidx]._rectangle.width() <= rec.width() || nodes[nidx]._rectangle.height() <= rec.height() || nodes[nidx]._level >= maxDepth) {
                nodes[nidx]._payloads.push_back(payload);
                return (true);
            }

            // This is a no-op if node has already been subdivided
            //
            subdivide(nodes, nidx);

            // Recursively insert in each child node that intersects rec
            //
            for (int q = 0; q < 4; q++) {
                size_t child = node_t::quadrant(nodes, nidx, q);
                if (nodes[child].intersects(rec)) {
                    bool ok = node_t::insert(nodes, child, rec, payload, maxDepth);
                    VAssert(ok);
                }
            }

            return (true);
        }

        static void get_payload_contains(const std::vector<node_t> &nodes, size_t nidx, T x, T y, std::vector<S> &payloads)
        {
            const node_t &node = nodes[nidx];

            if (!node._rectangle.contains(x, y)) return;

            if (node._payloads.size()) { payloads.insert(payloads.end(), node._payloads.begin(), node._payloads.end()); }
            if (node._is_leaf) return;

            for (int q = 0; q < 4; q++) {
                size_t child = node_t::quadrant(nodes, nidx, q);
                if (nodes[child]._rectangle.contains(x, y)) { node_t::get_payload_contains(nodes, child, x, y, payloads); }
            }
        }

        static void print(const std::vector<node_t> &nodes, size_t nidx, std::ostream &os)
        {
            const node_t &node = nodes[nidx];
            for (int i = 0; i < node._level; i++) os << " ";
            os << node._rectangle;

            for (int i = 0; i < node._level; i++) os << " ";
            os << "payload : ";
            for (int i = 0; i < node._payloads.size(); i++) { os << node._payloads[i] << " "; }
            os << std::endl;
            if (!node._is_leaf) {
                for (int q = 0; q < 4; q++) {
                    size_t childidx = quadrant(nodes, nidx, q);
                    node_t::print(nodes, childidx, os);
                }
            }
        }
        const std::vector<S> &get_payloads() const { return (_payloads); }
        size_t                get_level() const { return (_level); }

    private:
        int            _level;
        bool           _is_leaf;
        size_t         _child0;
        rectangle_t    _rectangle;
        std::vector<S> _payloads;
    };

    std::vector<node_t> _nodes;
    size_t              _rootidx;
    size_t              _maxDepth;
};
};    // namespace VAPoR
