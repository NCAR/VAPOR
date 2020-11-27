#pragma once

#include <vector>
#include <iostream>
#include <cstdint>
#include <vapor/VAssert.h>

namespace VAPoR {

//
//! \class QuadTreeRectangle
//! \brief This class implements a 2D quad tree space partitioning tree
//! that operates on rectangular regions.
//!
//
template<typename T, typename S> class QuadTreeRectangle {
public:
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
        _nodes.push_back(new node_t(left, top, right, bottom));
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
        _nodes.push_back(new node_t(0.0, 0.0, 1.0, 1.0));
        _rootidx = 0;
        _maxDepth = max_depth;
    }

    QuadTreeRectangle(const QuadTreeRectangle &rhs)
    {
        _nodes.resize(rhs._nodes.size());
        for (size_t i = 0; i < rhs._nodes.size(); i++) { _nodes[i] = new node_t(*(rhs._nodes[i])); }
        _rootidx = rhs._rootidx;
        _maxDepth = rhs._maxDepth;
    }

    QuadTreeRectangle &operator=(const QuadTreeRectangle &rhs)
    {
        if (*this == rhs) return *this;

        for (size_t i = 0; i < _nodes.size(); i++) {
            if (_nodes[i]) delete _nodes[i];
        }

        _nodes.resize(rhs._nodes.size());
        for (size_t i = 0; i < rhs._nodes.size(); i++) { _nodes[i] = new node_t(*(rhs._nodes[i])); }
        _rootidx = rhs._rootidx;
        _maxDepth = rhs._maxDepth;
        return *this;
    }

    ~QuadTreeRectangle()
    {
        for (size_t i = 0; i < _nodes.size(); i++) {
            if (_nodes[i]) delete _nodes[i];
        }
        _nodes.clear();
    }

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
    bool Insert(T left, T top, T right, T bottom, S payload)
    {
        node_t *root = _nodes[_rootidx];
        if (!root->intersects(rectangle_t(left, top, right, bottom))) return (false);

        return (root->insert(_nodes, rectangle_t(left, top, right, bottom), payload, _maxDepth));
    }

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

        const node_t *root = _nodes[_rootidx];
        root->get_payload_contains(_nodes, x, y, payloads);
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
            size_t b = _nodes[i]->get_payloads().size();
            if (b >= payload_histo.size()) { payload_histo.resize(b + 1, 0); }
            payload_histo[b] += 1;

            b = _nodes[i]->get_level();
            if (b >= level_histo.size()) { level_histo.resize(b + 1, 0); }
            level_histo[b] += 1;
        }
    }

    friend std::ostream &operator<<(std::ostream &os, const QuadTreeRectangle &q)
    {
        os << "Num nodes : " << q._nodes.size() << std::endl;
        const node_t *root = q._nodes[q._rootidx];
        root->print(q._nodes, os);
        return (os);
    }

private:
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
        rectangle_t quadrant(uint32_t n) const
        {
            T const center_x((_left + _right) / 2);
            T const center_y((_top + _bottom) / 2);
            switch (n & 0x03) {
            case 0: return rectangle_t(_left, _top, center_x, center_y);
            case 1: return rectangle_t(center_x, _top, _right, center_y);
            case 2: return rectangle_t(_left, center_y, center_x, _bottom);
            case 3: return rectangle_t(center_x, center_y, _right, _bottom);
            }
            return *this;    // Can't happen since we mask n
        }

        friend std::ostream &operator<<(std::ostream &os, const rectangle_t &rec)
        {
            os << "left-top, right-bottom : "
               << "(" << rec._left << ", " << rec._top << ") "
               << "(" << rec._right << ", " << rec._bottom << ")" << std::endl;
            return (os);
        }

    private:
        T _left, _top, _right, _bottom;
    };

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

        void subdivide(std::vector<node_t *> &nodes)
        {
            if (!_is_leaf) return;

            _is_leaf = false;
            _child0 = nodes.size();
            nodes.push_back(new node_t(_rectangle.quadrant(0), _level + 1));
            nodes.push_back(new node_t(_rectangle.quadrant(1), _level + 1));
            nodes.push_back(new node_t(_rectangle.quadrant(2), _level + 1));
            nodes.push_back(new node_t(_rectangle.quadrant(3), _level + 1));
        }

        const node_t *quadrant(const std::vector<node_t *> &nodes, uint32_t n) const
        {
            switch (n & 0x03) {
            case 0: return nodes[_child0 + 0];
            case 1: return nodes[_child0 + 1];
            case 2: return nodes[_child0 + 2];
            case 3: return nodes[_child0 + 3];
            }
            return nodes[_child0 + 0];
            VAssert(0);
        }
        node_t *quadrant(std::vector<node_t *> &nodes, uint32_t n) const
        {
            switch (n & 0x03) {
            case 0: return nodes[_child0 + 0];
            case 1: return nodes[_child0 + 1];
            case 2: return nodes[_child0 + 2];
            case 3: return nodes[_child0 + 3];
            }
            return nodes[_child0 + 0];
            VAssert(0);
        }

        bool insert(std::vector<node_t *> &nodes, const rectangle_t &rec, S payload, size_t maxDepth)
        {
            if (!_rectangle.intersects(rec)) return (false);

            // if rec is larger than a quadrant (half the width and height of this
            // node) there is no point in refining. I.e. stop descending the
            // tree and store the payload here.
            //
            if (_rectangle.width() < rec.width() || _rectangle.height() < rec.height() || _level >= maxDepth) {
                _payloads.push_back(payload);
                return (true);
            }

            // This is a no-op if node has already been subdivided
            //
            subdivide(nodes);

            // Recursively insert in each child node that intersects rec
            //
            for (int q = 0; q < 4; q++) {
                node_t *child = quadrant(nodes, q);
                if (child->intersects(rec)) {
                    bool ok = child->insert(nodes, rec, payload, maxDepth);
                    VAssert(ok);
                }
            }

            return (true);
        }

        void get_payload_contains(const std::vector<node_t *> &nodes, T x, T y, std::vector<S> &payloads) const
        {
            if (!_rectangle.contains(x, y)) return;

            if (_payloads.size()) { payloads.insert(payloads.end(), _payloads.begin(), _payloads.end()); }
            if (_is_leaf) return;

            for (int q = 0; q < 4; q++) {
                const node_t *node = quadrant(nodes, q);
                if (node->_rectangle.contains(x, y)) { node->get_payload_contains(nodes, x, y, payloads); }
            }
        }

        void print(const std::vector<node_t *> &nodes, std::ostream &os) const
        {
            for (int i = 0; i < _level; i++) os << " ";
            os << _rectangle;

            for (int i = 0; i < _level; i++) os << " ";
            os << "payload : ";
            for (int i = 0; i < _payloads.size(); i++) { os << _payloads[i] << " "; }
            os << std::endl;
            if (!_is_leaf) {
                for (int q = 0; q < 4; q++) {
                    const node_t *child = quadrant(nodes, q);
                    child->print(nodes, os);
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

    std::vector<node_t *> _nodes;
    size_t                _rootidx;
    size_t                _maxDepth;
};
};    // namespace VAPoR
