#include <iostream>
#include <vector>
#include <cassert>
#include <numeric>
#include <cmath>
#include <time.h>
#ifdef  Darwin
#include <mach/mach_time.h>
#endif
#ifdef _WINDOWS
#include "windows.h"
#include "Winbase.h"
#include <limits>
#endif

#include <vapor/utils.h>
#include <vapor/Grid.h>

using namespace std;
using namespace VAPoR;

Grid::Grid(
    const std::vector <size_t> &dims,
    const std::vector <size_t> &bs,
    const std::vector <float *> &blks,
    size_t topology_dimension
) {
	assert (dims.size() == bs.size());
	assert (dims.size() <= 3);

	for (int i=0; i<bs.size(); i++) {

		assert(bs[i] > 0);
		assert(dims[i] > 0);

		_bdims.push_back(((dims[i]-1) / bs[i]) + 1);
	}
	assert(
		blks.size() == 0 ||	// dataless
		blks.size() == std::accumulate(
			_bdims.begin(), _bdims.end(), 1, std::multiplies<size_t>()
		)
    );

	_dims = dims;
	_bs = bs;
	_periodic = vector <bool>(topology_dimension, false);
	_topologyDimension = topology_dimension;
	_missingValue = INFINITY;
	_hasMissing = false;
	_interpolationOrder = 0;

    //
    // Shallow  copy blocks
    //
    _blks = blks;
}


float Grid::AccessIndex(const std::vector <size_t> &indices) const {
	float *fptr = AccessIndex(_blks, indices);
	if (! fptr) return(GetMissingValue());
	return (*fptr);
}

void Grid::SetValue(const std::vector <size_t> &indices, float v) {
	float *fptr = AccessIndex(_blks, indices);
	if (! fptr) return;
	*fptr = v;
}

float *Grid::AccessIndex(
	const std::vector <float *> &blks,
	const std::vector <size_t> &indices
) const {

	std::vector <size_t> cIndices = indices;
	ClampIndex(cIndices);

	size_t bs[] = {0,0,0};
	size_t bdims[] = {0,0,0};

	if (! blks.size()) return(NULL);

	vector <size_t> dims = GetDimensions();
	size_t ndim = dims.size();
	for (int i=0; i<ndim; i++) {
		if (cIndices[i] >= dims[i]) {
			return(NULL);
		}
		bs[i] = _bs[i];
		bdims[i] = _bdims[i];
	}

	size_t xb = cIndices[0] / bs[0];
	size_t yb = ndim > 1 ? cIndices[1] / bs[1] : 0;
	size_t zb = ndim > 2 ? cIndices[2] / bs[2] : 0;

	size_t x = cIndices[0] % bs[0];
	size_t y = ndim > 1 ? cIndices[1] % bs[1] : 0;
	size_t z = ndim > 2 ? cIndices[2] % bs[2] : 0;

	float *blk = blks[zb*bdims[0]*bdims[1] + yb*bdims[0] + xb];
	return(&blk[z*bs[0]*bs[1] + y*bs[0] + x]);
}

float Grid::AccessIJK(size_t i, size_t j, size_t k) const {
    std::vector <size_t> indices = {i,j,k};
    return(AccessIndex(indices));
}

void Grid::SetValueIJK(size_t i, size_t j, size_t k, float v) {
    std::vector <size_t> indices = {i,j,k};
    return(SetValue(indices, v));
}


void Grid::GetRange(float range[2]) const {

	bool first = true;
	range[0] = range[1] = GetMissingValue();
	float missingValue = GetMissingValue();
	Grid::ConstIterator itr;
	Grid::ConstIterator enditr = this->cend();
	for (itr = this->cbegin(); itr!=enditr; ++itr) {
		if (first && *itr != missingValue) {
			range[0] = range[1] = *itr;
			first = false;
		}

		if (! first) {
			if (*itr < range[0] && *itr != missingValue) range[0] = *itr;
			else if (*itr > range[1] && *itr != missingValue) range[1] = *itr;
		}
	}
}

void Grid::GetRange(
	std::vector <size_t> min, std::vector <size_t> max,
	float range[2]
) const {

	vector <size_t> cMin = min;
	ClampIndex(cMin);

	vector <size_t> cMax = max;
	ClampIndex(cMax);

	const vector <size_t> &dims = GetDimensions();

    assert(cMin.size() == cMax.size());


	float mv = GetMissingValue();

	range[0] = range[1] = mv;

	size_t jmin = 0;
	size_t jmax = 0;
	if (dims.size() > 1) {
		jmin = cMin[1];
		jmax = cMax[1];
	}

	size_t kmin = 0;
	size_t kmax = 0;
	if (dims.size() > 2) {
		kmin = cMin[2];
		kmax = cMax[2];
	}

	bool first = true;
	for (size_t k=kmin; k<=kmax; k++) {
	for (size_t j=jmin; j<=jmax; j++) {
	for (size_t i=cMin[0]; i<=cMax[0]; i++) {
		float v = AccessIJK(i,j,k);
		if (first &&  v != mv) {
			range[0] = range[1] = v;
			first = false;
		}

		if (! first) {
			if (v < range[0] && v != mv) range[0] = v;
			else if (v > range[1] && v != mv) range[1] = v;
		}

	}
	}
	}
}

float Grid::GetValue(const std::vector <double> &coords) const {
	if (!_blks.size()) return(GetMissingValue());

	vector <double> clampedCoords = coords;

    // Clamp coordinates on periodic boundaries to grid extents
    //
    ClampCoord(clampedCoords);

#ifdef	DEAD
    // At this point xyz should be within the grid bounds
    //
    if (! InsideGrid(clampedCoords)) return(_missingValue);
#endif

    if (_interpolationOrder == 0) {
        return (GetValueNearestNeighbor(clampedCoords));
    }
    else {
        return (GetValueLinear(clampedCoords));
    }
}

void Grid::_getUserCoordinatesHelper(
	const vector <double> &coords, double &x, double &y, double &z
) const {
	if (coords.size() >= 1) {
		x = coords[0];
	}
	if (coords.size() >= 2) {
		y = coords[1];
	}
	if (coords.size() >= 3) {
		z = coords[2];
	}
}

void Grid::GetUserCoordinates(
	size_t i, double &x, double &y, double &z
) const {
	x = y = z = 0.0;
	vector <size_t> indices = {i};
	vector <double> coords;
	GetUserCoordinates(indices, coords);
	_getUserCoordinatesHelper(coords, x, y, z);
}

void Grid::GetUserCoordinates(
	size_t i, size_t j, double &x, double &y, double &z
) const {
	x = y = z = 0.0;
	vector <size_t> indices = {i, j};
	vector <double> coords;
	GetUserCoordinates(indices, coords);
	_getUserCoordinatesHelper(coords, x, y, z);
}

void Grid::GetUserCoordinates(
	size_t i, size_t j, size_t k, double &x, double &y, double &z
) const {
	x = y = z = 0.0;
	vector <size_t> indices = {i, j, k};
	vector <double> coords;
	GetUserCoordinates(indices, coords);
	_getUserCoordinatesHelper(coords, x, y, z);
}

void Grid::SetInterpolationOrder(int order) {
	if (order<0 || order>2) order = 1;
	_interpolationOrder = order;
}


/////////////////////////////////////////////////////////////////////////////
//
// Iterators
//
/////////////////////////////////////////////////////////////////////////////



Grid::ConstNodeIteratorSG::ConstNodeIteratorSG(
	const Grid *g, bool begin
) : ConstNodeIteratorAbstract() {

	_dims = g->GetNodeDimensions();
	_index = vector<size_t> (_dims.size(), 0);
	_lastIndex = _index; 
	if (_dims.size()) _lastIndex[_dims.size()-1] = _dims[_dims.size()-1];

	if (! begin) {
		_index = _lastIndex;
	}
}

Grid::ConstNodeIteratorSG::ConstNodeIteratorSG(
	const ConstNodeIteratorSG &rhs
) : ConstNodeIteratorAbstract() {
	_dims = rhs._dims;
	_index = rhs._index;
	_lastIndex = rhs._lastIndex;
}

Grid::ConstNodeIteratorSG::ConstNodeIteratorSG(
) : ConstNodeIteratorAbstract() {

	_dims.clear();
	_index.clear();
	_lastIndex.clear();
}

void Grid::ConstNodeIteratorSG::next() {

	if (! _index.size()) return;

	_index[0]++;
	if (_index[0] < _dims[0] || _dims.size() == 1) {
		return;
	}

	_index[0] = 0;
	_index[1]++;
	if (_index[1] < _dims[1] || _dims.size() == 2) {
		return;
	}

	_index[1] = 0;
	_index[2]++;
	if (_index[2] < _dims[2] || _dims.size() == 3) {
		return;
	}
	assert(0 && "Invalid state");

}

void Grid::ConstNodeIteratorSG::next(const long &offset) {

	if (! _index.size()) return;

	vector <size_t> maxIndex;
	long maxIndexL = Wasp::LinearizeCoords(maxIndex, _dims);
	long newIndexL = Wasp::LinearizeCoords(_index, _dims) + offset;
	if (newIndexL < 0) {
		newIndexL = 0;
	}
	if (newIndexL > maxIndexL) {
		_index = _lastIndex;
		return;
	}

	_index = Wasp::VectorizeCoords(newIndexL, _dims);

}

Grid::ConstNodeIteratorBoxSG::ConstNodeIteratorBoxSG(
	const Grid *g,
	const std::vector <double> &minu, const std::vector <double> &maxu
) : ConstNodeIteratorSG(g, true), _pred(minu, maxu) {

	_coordItr = g->ConstCoordBegin();

	// Advance to first node inside box
	//
	if (! _pred(*_coordItr)) {
		next();
	}
}

Grid::ConstNodeIteratorBoxSG::ConstNodeIteratorBoxSG(
	const ConstNodeIteratorBoxSG &rhs
) : ConstNodeIteratorSG() {
	_coordItr = rhs._coordItr;
	_pred = rhs._pred;
}

Grid::ConstNodeIteratorBoxSG::ConstNodeIteratorBoxSG(
) : ConstNodeIteratorSG() {

}


void Grid::ConstNodeIteratorBoxSG::next() {

	do {
		ConstNodeIteratorSG::next();
		++_coordItr;
	} while (! _pred(*_coordItr) && _index != _lastIndex);
}

void Grid::ConstNodeIteratorBoxSG::next(const long &offset) {

	_coordItr += offset;

	while (! _pred(*_coordItr) && _index != _lastIndex) {
		ConstNodeIteratorSG::next();
		++_coordItr;
	}
}



Grid::ConstCellIteratorSG::ConstCellIteratorSG(
	const Grid *g, bool begin
) : ConstCellIteratorAbstract() {

	_dims = g->GetCellDimensions();
	_index = vector<size_t> (_dims.size(), 0);
	_lastIndex = _index; _lastIndex[_dims.size()-1] = _dims[_dims.size()-1];
	if (! begin) {
		_index = _lastIndex;
	}
}

Grid::ConstCellIteratorSG::ConstCellIteratorSG(
	const ConstCellIteratorSG &rhs
) : ConstCellIteratorAbstract() {
	_dims = rhs._dims;
	_index = rhs._index;
	_lastIndex = rhs._lastIndex;
}

Grid::ConstCellIteratorSG::ConstCellIteratorSG(
) : ConstCellIteratorAbstract() {

	_dims.clear();
	_index.clear();
	_lastIndex.clear();
}

void Grid::ConstCellIteratorSG::next() {

	_index[0]++;
	if (_index[0] < (_dims[0]) || _dims.size() == 1) {
		return;
	}

	_index[0] = 0;
	_index[1]++;
	if (_index[1] < (_dims[1]) || _dims.size() == 2) {
		return;
	}

	_index[1] = 0;
	_index[2]++;
	if (_index[2] < (_dims[2]) || _dims.size() == 3) {
		return;
	}
	assert(0 && "Invalid state");

}

void Grid::ConstCellIteratorSG::next(const long &offset) {

	vector <size_t> maxIndex;
	long maxIndexL = Wasp::LinearizeCoords(maxIndex, _dims);
	long newIndexL = Wasp::LinearizeCoords(_index, _dims) + offset;
	if (newIndexL < 0) {
		newIndexL = 0;
	}
	if (newIndexL > maxIndexL) {
		_index = _lastIndex;
		return;
	}

	_index = Wasp::VectorizeCoords(newIndexL, _dims);

}

Grid::ConstCellIteratorBoxSG::ConstCellIteratorBoxSG(
	const Grid *g,
	const std::vector <double> &minu, const std::vector <double> &maxu
) : ConstCellIteratorSG(g, true), _pred(minu, maxu) {

	_coordItr = g->ConstCoordBegin();

	// Advance to first node inside box
	//
	if (! _pred(*_coordItr)) {
		next();
	}
}

Grid::ConstCellIteratorBoxSG::ConstCellIteratorBoxSG(
	const ConstCellIteratorBoxSG &rhs
) : ConstCellIteratorSG() {
	_coordItr = rhs._coordItr;
	_pred = rhs._pred;
}

Grid::ConstCellIteratorBoxSG::ConstCellIteratorBoxSG(
) : ConstCellIteratorSG() {

}


void Grid::ConstCellIteratorBoxSG::next() {

	do {
		ConstCellIteratorSG::next();
		++_coordItr;
	} while (! _pred(*_coordItr) && _index != _lastIndex);
}

void Grid::ConstCellIteratorBoxSG::next(const long &offset) {

	_coordItr += offset;

	while (! _pred(*_coordItr) && _index != _lastIndex) {
		ConstCellIteratorSG::next();
		++_coordItr;
	}
}




//
//
// Iterators
//
//

template <class T>
Grid::ForwardIterator<T>::ForwardIterator (
	T *rg, bool begin, 
	const vector <double> &minu, const vector <double> &maxu
) : _pred(minu, maxu) {

	const vector <size_t> &dims = rg->GetDimensions();

	_rg = rg;
	_index = vector <size_t> (dims.size(), 0);
	_end_index = vector <size_t> (dims.size(), 0);
	if (dims.size()) _end_index[dims.size()-1] = dims[dims.size()-1];
	if (! begin || ! rg->GetBlks().size()) {
		_index = _end_index;
		return;
	}

	_coordItr = rg->ConstCoordBegin();
	_xb = 0;
	_itr = &rg->GetBlks()[0][0];


	if (! _pred(*_coordItr)) {
		operator++();
	}
}

template <class T>
Grid::ForwardIterator<T>::ForwardIterator(ForwardIterator<T> &&rhs) {
	_rg = rhs._rg; rhs._rg = nullptr;
	_coordItr = std::move(rhs._coordItr);
	_index = rhs._index;
	_end_index = rhs._end_index;
	_xb = rhs._xb;
	_itr = rhs._itr; rhs._itr = nullptr;
	_pred = rhs._pred;
}

template <class T>
Grid::ForwardIterator<T>::ForwardIterator() {
	_rg = nullptr;
	//_coordItr = xx;
	_index.clear();
	_end_index.clear();
	_xb = 0;
	_itr = nullptr;
	//_pred = xx;
}

template <class T>
Grid::ForwardIterator<T>
&Grid::ForwardIterator<T>::operator=(ForwardIterator<T> rhs) {

	swap(*this, rhs);
	return(*this);
}



template <class T> 
Grid::ForwardIterator<T>
&Grid::ForwardIterator<T>::operator++() {

	if (! _rg->GetBlks().size()) return(*this);

	const vector <size_t> &dims = _rg->GetDimensions();
	const vector <size_t> &bdims = _rg->GetDimensionInBlks();
	const vector <size_t> &bs = _rg->GetBlockSize();

	size_t xb = 0;
	size_t yb = 0;
	size_t zb = 0;
	size_t x = 0;
	size_t y = 0;
	size_t z = 0;
	do {

		_xb++;
		_itr++;
		_index[0]++;
		++_coordItr;

		if (_xb < bs[0] && _index[0]<dims[0]) {

			if (_pred(*_coordItr)) {
				return(*this);
			}

			continue;
		}

		_xb = 0;
		if (_index[0] >= dims[0]) {
			if (dims.size() == 1) {
				return(*this);	// last element
			}
			_index[0] = _xb = 0;
			_index[1]++;
		}

		if (_index[1] >= dims[1]) {
			if (dims.size() == 2) {
				return(*this);	// last element
			}
			_index[1] = 0;
			_index[2]++;
		}

		if (dims.size() == 3 && _index[2] >= dims[2]) {
			return(*this);	// last element
		}

		xb = _index[0] / bs[0];
		yb = _index[1] / bs[1];
		zb = 0;
		if (dims.size() == 3) zb = _index[2] / bs[2];

		x = _index[0] % bs[0];
		y = _index[1] % bs[1];
		z = 0;
		if (dims.size() == 3) z = _index[2] % bs[2];

		float *blk = _rg->GetBlks()[zb*bdims[0]*bdims[1] + yb*bdims[0] + xb];
		_itr = &blk[z*bs[0]*bs[1] + y*bs[0] + x];


	} while (! _pred(*_coordItr) && _index != _end_index);

	return(*this);
}

template <class T> 
Grid::ForwardIterator<T> Grid::ForwardIterator<T>::
operator++(int) {

	ForwardIterator temp(*this);
	++(*this);
	return(temp);
}


template <class T> 
Grid::ForwardIterator<T> &Grid::ForwardIterator<T>::
operator+=(const long int &offset) {

	if (! _rg->GetBlks().size()) return(*this);

	const vector <size_t> &dims = _rg->GetDimensions();
	const vector <size_t> &bdims = _rg->GetDimensionInBlks();
	const vector <size_t> &bs = _rg->GetBlockSize();

	vector <size_t> maxIndex;
	for (int i=0; i<dims.size(); i++) maxIndex.push_back(dims[i]-1);

	long maxIndexL = Wasp::LinearizeCoords(maxIndex, dims);
	long newIndexL = Wasp::LinearizeCoords(_index, dims) + offset;
	if (newIndexL < 0) {
		newIndexL = 0;
	}
	if (newIndexL > maxIndexL) {
		_index = _end_index;
		return(*this);
	}

	_index = Wasp::VectorizeCoords(newIndexL, dims);

	size_t xb = 0;
	size_t yb = 0;
	size_t zb = 0;
	size_t x = 0;
	size_t y = 0;
	size_t z = 0;

	_xb = xb = _index[0] / bs[0];
	yb = _index[1] / bs[1];
	zb = 0;
	if (dims.size() == 3) zb = _index[2] / bs[2];

	x = _index[0] % bs[0];
	y = _index[1] % bs[1];
	z = 0;
	if (dims.size() == 3) z = _index[2] % bs[2];

	float *blk = _rg->GetBlks()[zb*bdims[0]*bdims[1] + yb*bdims[0] + xb];
	_itr = &blk[z*bs[0]*bs[1] + y*bs[0] + x];

	_coordItr += offset;

	while (! _pred(*_coordItr) && _index != _end_index) {

		_xb++;
		_itr++;
		_index[0]++;
		++_coordItr;

		if (_xb < bs[0] && _index[0]<dims[0]) {

			if (_pred(*_coordItr)) {
				return(*this);
			}

			continue;
		}

		_xb = 0;
		if (_index[0] >= dims[0]) {
			if (dims.size() == 1) {
				return(*this);	// last element
			}
			_index[0] = _xb = 0;
			_index[1]++;
		}

		if (_index[1] >= dims[1]) {
			if (dims.size() == 2) {
				return(*this);	// last element
			}
			_index[1] = 0;
			_index[2]++;
		}

		if (dims.size() == 3 && _index[2] >= dims[2]) {
			return(*this);	// last element
		}

		xb = _index[0] / bs[0];
		yb = _index[1] / bs[1];
		zb = 0;
		if (dims.size() == 3) zb = _index[2] / bs[2];

		x = _index[0] % bs[0];
		y = _index[1] % bs[1];
		z = 0;
		if (dims.size() == 3) z = _index[2] % bs[2];

		float *blk = _rg->GetBlks()[zb*bdims[0]*bdims[1] + yb*bdims[0] + xb];
		_itr = &blk[z*bs[0]*bs[1] + y*bs[0] + x];


	}

	return(*this);
}

template <class T> 
Grid::ForwardIterator<T> Grid::ForwardIterator<T>::
operator+(const long int &offset) const {

	ForwardIterator temp(*this);

	temp+=offset;
	return(temp);
}


// Need this so that template definitions can be made in .cpp file, not .h file
//
template class Grid::ForwardIterator<Grid>;
template class Grid::ForwardIterator<const Grid>;



namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const Grid &g) {
	o << "Grid" << endl;

    o << " Dimensions ";
	for (int i=0; i<g._dims.size(); i++) {
    	o << g._dims[i] << " ";
	}
    o << endl;

	o << " Block dimensions ";
	for (int i=0; i<g._bs.size(); i++) {
		o << g._bs[i] << " ";
	}
	o << endl;

	o << " Grid dimensions in blocks ";
	for (int i=0; i<g._bdims.size(); i++) {
		o << g._bdims[i] << " ";
	}
	o << endl;

    o << " Topological dimension " << g._topologyDimension << endl;

    o << " Periodicity ";
	for (int i=0; i<g._periodic.size(); i++) {
    	o << g._periodic[i] << " ";
	}
    o << endl;

	o << " Missing value flag " << g._hasMissing << endl;
	o << " Missing value " << g._missingValue << endl;
    o << " Interpolation order " << g._interpolationOrder << endl;

	return(o);
}
};

