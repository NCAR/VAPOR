#pragma once

#include <vector>
#include <iostream>
#include <cstdint>

namespace VAPoR {

template <typename T, typename S>
class QuadTreeRectangle {
public:

 QuadTreeRectangle(T left, T top, T right, T bottom, size_t reserve_size = 1000) {
	assert(left <= right);
	assert(top <= bottom);
	_nodes.reserve(reserve_size);
	_nodes.push_back(node_t(left, top, right, bottom));
	_rootidx = 0;
 }
 QuadTreeRectangle(size_t reserve_size = 1000) {
	_nodes.reserve(reserve_size);
	_nodes.push_back(node_t(0.0, 0.0, 1.0, 1.0));
	_rootidx = 0;
 }

 bool Insert(T left, T top, T right, T bottom, S payload) {

	node_t &root = _nodes[_rootidx];
    if (! root.intersects(rectangle_t(left, top, right, bottom))) return(false);

    return(root.insert(_nodes, rectangle_t(left, top, right, bottom), payload));

 }

 bool GetPayloadContained(T x, T y, std::vector <S> &payloads) const {
    payloads.clear();

	const node_t &root = _nodes[_rootidx];
    return(root.get_payload_contains(_nodes, x,y,payloads));
 }

 friend std::ostream& operator<<(std::ostream &os, const QuadTreeRectangle& q) {
    os << "Num nodes : " << q._nodes.size() << std::endl;
	const node_t &root = q._nodes[q._rootidx];
	root.print(q._nodes, os);
	return(os);
 }

private:
 class rectangle_t {
 public:
  rectangle_t() : _left(0.0f), _top(0.0f), _right(0.0f), _bottom(0.0f) {}
  rectangle_t(T x1, T y1, T x2, T y2)
        : _left(x1), _top(y1), _right(x2), _bottom(y2)
    {}

  // return true iff other intersects us
  //
  bool intersects(rectangle_t const& other) const {
	if (_left > other._right || _top > other._bottom) return(false);

	if (_right < other._left || _bottom < other._top) return(false);
	return(true);
  }

  // return true iff other is completely contained inside or on boundary
  //
  bool contains(rectangle_t const& other) const {
    return ((_left <= other._left)
        && (_right >= other._right)
        && (_top <= other._top)
        && (_bottom >= other._bottom));
  }

  // return true iff point(x,y) is completely contained inside or on boundary
  //
  bool contains(T x, T y) const {
    return ((_left <= x)
        && (_right >= x)
        && (_top <= y)
        && (_bottom >= y));
  }

  bool touches(rectangle_t const& other) const {
    return ((_left == other._right)
        || (_right == other._left)
        || (_top == other._bottom)
        || (_bottom == other._top));
  }

  T width() const {
	return(_right - _left);
  }
  T height() const {
	return(_bottom - _top);
  }


  rectangle_t quadrant(uint32_t n) const {
    T const center_x((_left + _right) / 2);
    T const center_y((_top + _bottom) / 2);
    switch (n & 0x03) {
		case 0: return rectangle_t(_left, _top, center_x, center_y);
		case 1: return rectangle_t(center_x, _top, _right, center_y);
		case 2: return rectangle_t(_left, center_y, center_x, _bottom);
		case 3: return rectangle_t(center_x, center_y, _right, _bottom);
    }
    return *this; // Can't happen since we mask n
  }

  friend std::ostream& operator<<(std::ostream &os, const rectangle_t& rec) {
	os << "left-top, right-bottom : " << 
	"(" << rec._left << ", " << rec._top << ") " <<
	"(" << rec._right << ", " << rec._bottom << ")" << std::endl;
	return(os);
  }

 private:
    T _left, _top, _right, _bottom;
 };


 class node_t {
 public:

  node_t(int level=0) :
   _level(level),
   _is_leaf(true),
   _child0(0),
   _rectangle(0.0, 0.0, 1.0, 1.0) {}

  node_t(
	T left, T top, T right, T bottom, int level = 0
  ) : 
   _level(level),
   _is_leaf(true),
   _child0(0),
   _rectangle(left, top, right, bottom) {}

  node_t(const rectangle_t &rec, int level = 0) : 
   _level(level),
   _is_leaf(true),
   _child0(0),
   _rectangle(rec) {}

  rectangle_t& bounds() {return (_rectangle); }
  rectangle_t const& bounds() const {return(_rectangle); }

  bool intersects(rectangle_t const& other) const {
	return(_rectangle.intersects(other));
  }
  bool contains(rectangle_t const& other) const {
	return(_rectangle.contains(other));
  }
  bool contains(T x, T y) const {
	return(_rectangle.contains(x,y));
  }
  bool touches(rectangle_t const& other) const {
	return(_rectangle.touches(other));
  }

  void subdivide(std::vector <node_t> &nodes) {
 	if (! _is_leaf) return;

	_is_leaf = false;
	_child0 = nodes.size();
	nodes.push_back( node_t(_rectangle.quadrant(0), _level+1));
	nodes.push_back( node_t(_rectangle.quadrant(1), _level+1));
	nodes.push_back( node_t(_rectangle.quadrant(2), _level+1));
	nodes.push_back( node_t(_rectangle.quadrant(3), _level+1));
  }

  const node_t &quadrant(const std::vector <node_t> &nodes, uint32_t n) const {
    switch (n & 0x03) {
		case 0: return nodes[_child0 + 0];
		case 1: return nodes[_child0 + 1];
		case 2: return nodes[_child0 + 2];
		case 3: return nodes[_child0 + 3];
    }
	return nodes[_child0 + 0];
	assert(0);
  }
  node_t &quadrant(std::vector <node_t> &nodes, uint32_t n) const {
    switch (n & 0x03) {
		case 0: return nodes[_child0 + 0];
		case 1: return nodes[_child0 + 1];
		case 2: return nodes[_child0 + 2];
		case 3: return nodes[_child0 + 3];
    }
	return nodes[_child0 + 0];
	assert(0);
  }

  bool insert(std::vector <node_t> &nodes, const rectangle_t &rec, S payload) { 
	if (! _rectangle.intersects(rec)) return(false);

if (_level > 74730) {
std::cout << "oh no mr bill" << std::endl;
}

	// if rec is larger than a quadrant (half the width and height of this
	// node) there is no point in refining. I.e. stop descending the
	// tree and store the payload here.
	//
	if (
		(_rectangle.width() / 2.0 < rec.width() && 
		_rectangle.height() / 2.0 < rec.height()) ||
		_level > 16
	) {
		_payloads.push_back(payload);
		return(true);
	}
		
	// This is a no-op if node has already been subdivided
	//
	subdivide(nodes);

	// Recursively insert in each child node that intersects rec
	//
	for (int q=0; q<4; q++) {
		node_t &child = quadrant(nodes, q);
		if (child.intersects(rec)) {
			bool ok = child.insert(nodes, rec, payload);
			assert(ok);
		}
	}

	return(true);
  }
	
  bool get_payload_contains(
	const std::vector <node_t> &nodes,
	T x, T y, std::vector <S> &payloads
  ) const {

	if (! _rectangle.contains(x,y)) return(false);

	if (_payloads.size()) {
		payloads.insert(payloads.end(), _payloads.begin(), _payloads.end());
	}
	if (_is_leaf) return(true);

	for (int q=0; q<4; q++) {
		const node_t &node = quadrant(nodes, q);
		if (node._rectangle.contains(x,y)) { 
			return(node.get_payload_contains(nodes, x,y,payloads));
		}
	}
	assert(0);
  }
		
	

  void print(const std::vector <node_t> &nodes, std::ostream &os) const {
	for (int i=0; i<_level; i++) os << " ";
	os << _rectangle;

	for (int i=0; i<_level; i++) os << " ";
	os << "payload : ";
	for (int i=0; i<_payloads.size(); i++) {
		os << _payloads[i] << " ";
	}
	os << std::endl;
	if (! _is_leaf) {
		for (int q=0; q<4; q++) {
			const node_t &child = quadrant(nodes, q);
			child.print(nodes, os);
		}
	}
 }

 private:
  int _level;
  bool _is_leaf;
  size_t _child0;
  rectangle_t _rectangle;
  std::vector <S> _payloads;
 };

 std::vector <node_t> _nodes;
 size_t _rootidx;

};
};
