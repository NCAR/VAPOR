#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include <time.h>
#ifdef Darwin
#include <mach/mach_time.h>
#endif
#ifdef _WINDOWS
#include "windows.h"
#include "Winbase.h"
#include <limits>
#endif

#include <vapor/utils.h>
#include <vapor/StructuredGrid.h>

using namespace std;
using namespace VAPoR;

void StructuredGrid::_StructuredGrid(
    const vector<size_t> &bs,
    const vector<size_t> &min,
    const vector<size_t> &max,
    const vector<bool> &periodic,
    const vector<float *> &blks) {
    assert(bs.size() == 2 || bs.size() == 3);
    assert(bs.size() == min.size());
    assert(bs.size() == max.size());
    assert(bs.size() == periodic.size());

    _min.clear();
    _max.clear();
    _bs.clear();
    _bdims.clear();
    _minabs.clear();
    _maxabs.clear();
    _periodic.clear();
    _blks.clear();

    size_t nblocks = 1;
    for (int i = 0; i < bs.size(); i++) {

        assert(max[i] >= min[i]);
        assert(bs[i] > 0);

        _bs.push_back(bs[i]);
        _bdims.push_back((max[i] / bs[i]) - (min[i] / bs[i]) + 1);
        nblocks *= _bdims[i];
        _minabs.push_back(min[i]);
        _maxabs.push_back(max[i]);
        _min.push_back(min[i] % bs[i]);
        _max.push_back(_min[i] + (max[i] - min[i]));
        _periodic.push_back(periodic[i]);
    }

    _ndim = _bs.size();

    //
    // Shallow  copy blocks
    //
    _blks = blks;

    _hasMissing = false;
    _missingValue = INFINITY;
    _interpolationOrder = 1;
}

StructuredGrid::StructuredGrid(
    const size_t bs[3],
    const size_t min[3],
    const size_t max[3],
    const bool periodic[3],
    const std::vector<float *> &blks) {
    vector<size_t> bs_v;
    vector<size_t> min_v;
    vector<size_t> max_v;
    vector<bool> periodic_v;

    for (int i = 0; i < 3; i++) {
        if (min[i] == max[i])
            break;
        bs_v.push_back(bs[i]);
        min_v.push_back(min[i]);
        max_v.push_back(max[i]);
        periodic_v.push_back(periodic[i]);
    }

    _StructuredGrid(bs_v, min_v, max_v, periodic_v, blks);
}

StructuredGrid::StructuredGrid(
    const vector<size_t> &bs,
    const vector<size_t> &min,
    const vector<size_t> &max,
    const vector<bool> &periodic,
    const vector<float *> &blks) {
    _StructuredGrid(bs, min, max, periodic, blks);
}

StructuredGrid::StructuredGrid(
    const size_t bs[3],
    const size_t min[3],
    const size_t max[3],
    const bool periodic[3],
    const vector<float *> &blks,
    float missing_value) {
    vector<size_t> bs_v;
    vector<size_t> min_v;
    vector<size_t> max_v;
    vector<bool> periodic_v;

    for (int i = 0; i < 3; i++) {
        if (min[i] == max[i])
            break;
        bs_v.push_back(bs[i]);
        min_v.push_back(min[i]);
        max_v.push_back(max[i]);
        periodic_v.push_back(periodic[i]);
    }

    _StructuredGrid(bs_v, min_v, max_v, periodic_v, blks);

    _missingValue = missing_value;
    _hasMissing = true;
}

StructuredGrid::StructuredGrid(
    const std::vector<size_t> &bs,
    const std::vector<size_t> &min,
    const std::vector<size_t> &max,
    const std::vector<bool> &periodic,
    const vector<float *> &blks,
    float missing_value) {
    _StructuredGrid(bs, min, max, periodic, blks);

    _missingValue = missing_value;
    _hasMissing = true;
}

StructuredGrid::StructuredGrid() {
    _min.clear();
    _max.clear();
    _bs.clear();
    _bdims.clear();
    _minabs.clear();
    _maxabs.clear();
    _periodic.clear();

    _missingValue = INFINITY;
    _hasMissing = false;
    _interpolationOrder = 0;
    _ndim = 0;
    _blks.clear();
}

StructuredGrid::~StructuredGrid() {
}

float &StructuredGrid::AccessIJK(size_t x, size_t y, size_t z) const {
    return (_AccessIJK(_blks, x, y, z));
}

float &StructuredGrid::_AccessIJK(
    const vector<float *> &blks, size_t x, size_t y, size_t z) const {

    if (!blks.size())
        return ((float &)_missingValue);

    if (x > (_max[0] - _min[0]))
        return ((float &)_missingValue);
    if (y > (_max[1] - _min[1]))
        return ((float &)_missingValue);
    if (_ndim == 3) {
        if (z > (_max[2] - _min[2]))
            return ((float &)_missingValue);
    }

    // x,y,z are specified relative to _min[i]
    //
    x += _min[0];
    y += _min[1];
    if (_ndim == 3)
        z += _min[2];

    size_t xb = x / _bs[0];
    size_t yb = y / _bs[1];
    size_t zb = 0;
    if (_ndim == 3)
        zb = z / _bs[2];

    x = x % _bs[0];
    y = y % _bs[1];
    if (_ndim == 3)
        z = z % _bs[2];
    float *blk = blks[zb * _bdims[0] * _bdims[1] + yb * _bdims[0] + xb];
    return (blk[z * _bs[0] * _bs[1] + y * _bs[0] + x]);
}

float StructuredGrid::GetValue(double x, double y, double z) const {

    // Clamp coordinates on periodic boundaries to grid extents
    //
    _ClampCoord(x, y, z);

    // At this point xyz should be within the grid bounds
    //
    if (!InsideGrid(x, y, z))
        return (_missingValue);

    if (_interpolationOrder == 0) {
        return (_GetValueNearestNeighbor(x, y, z));
    } else {
        return (_GetValueLinear(x, y, z));
    }
}
void StructuredGrid::GetUserExtents(double extents[6]) const {
    for (int i = 0; i < 6; i++)
        extents[i] = 0.0;

    vector<double> minu, maxu;
    GetUserExtents(minu, maxu);
    assert(minu.size() == maxu.size());
    assert(minu.size() == _ndim);

    for (int i = 0; i < minu.size(); i++) {
        extents[i] = minu[i];
        extents[i + 3] = maxu[i];
    }
}

void StructuredGrid::GetEnclosingRegion(
    const double minu[3], const double maxu[3], size_t min[3], size_t max[3]) const {
    for (int i = 0; i < 3; i++) {
        min[i] = 0.0;
        max[i] = 0.0;
    }

    vector<double> v_minu;
    vector<double> v_maxu;
    for (int i = 0; i < _ndim; i++) {
        v_minu.push_back(minu[i]);
        v_maxu.push_back(maxu[i]);
    }

    vector<size_t> v_min;
    vector<size_t> v_max;
    GetEnclosingRegion(v_minu, v_maxu, v_min, v_max);

    assert(v_min.size() == v_max.size());
    for (int i = 0; i < v_min.size(); i++) {
        min[i] = v_min[i];
        max[i] = v_max[i];
    }
}

void StructuredGrid::GetDimensions(size_t dims[3]) const {
    vector<size_t> dims_v = StructuredGrid::GetDimensions();

    for (int i = 0; i < 3; i++)
        dims[i] = 1;
    for (int i = 0; i < dims_v.size(); i++)
        dims[i] = dims_v[i];
}

void StructuredGrid::GetDimensions(std::vector<size_t> &dims) const {
    dims.clear();

    for (int i = 0; i < _min.size(); i++)
        dims.push_back(_max[i] - _min[i] + 1);
}

vector<size_t> StructuredGrid::GetDimensions() const {
    vector<size_t> dims;

    for (int i = 0; i < _min.size(); i++)
        dims.push_back(_max[i] - _min[i] + 1);
    return (dims);
}

void StructuredGrid::SetInterpolationOrder(int order) {
    if (order < 0 || order > 2)
        order = 1;
    _interpolationOrder = order;
}

void StructuredGrid::GetRange(float range[2]) const {

    StructuredGrid::ConstIterator itr;
    bool first = true;
    range[0] = range[1] = _missingValue;
    for (itr = this->begin(); itr != this->end(); ++itr) {
        if (first && *itr != _missingValue) {
            range[0] = range[1] = *itr;
            first = false;
        }

        if (!first) {
            if (*itr < range[0] && *itr != _missingValue)
                range[0] = *itr;
            else if (*itr > range[1] && *itr != _missingValue)
                range[1] = *itr;
        }
    }
}

void StructuredGrid::SetPeriodic(const std::vector<bool> &periodic) {
    assert(periodic.size() == _ndim);

    _periodic = periodic;
}

void StructuredGrid::SetPeriodic(const bool periodic[3]) {
    _periodic.clear();

    for (int i = 0; i < _ndim; i++)
        _periodic.push_back(periodic[i]);
}

void StructuredGrid::GetBlockSize(size_t bs[3]) const {
    for (int i = 0; i < 3; i++)
        bs[i] = 1;

    for (int i = 0; i < _bs.size(); i++)
        bs[i] = _bs[i];
}

template <class T>
StructuredGrid::ForwardIterator<T>::ForwardIterator(T *rg) {
    if (!rg->_blks.size()) {
        _end = true;
        return;
    }
    _z = 0;

    _rg = rg;
    _xb = rg->_min[0];
    _x = rg->_min[0];
    _y = rg->_min[1];
    if (_rg->_ndim == 3)
        _z = rg->_min[2];
    _itr = &rg->_blks[0][_z * rg->_bs[0] * rg->_bs[1] + _y * rg->_bs[0] + _x];
    _end = false;
}

template <class T>
StructuredGrid::ForwardIterator<T>::ForwardIterator() {
    _rg = NULL;
    _xb = 0;
    _x = 0;
    _y = 0;
    _z = 0;
    _itr = NULL;
    _end = true;
}

template <class T>
StructuredGrid::ForwardIterator<T> &StructuredGrid::ForwardIterator<T>::
operator++() {

    if (!_rg->_blks.size())
        _end = true;
    if (_end)
        return (*this);

    _xb++;
    _itr++;
    _x++;
    if (_xb < _rg->_bs[0] && _x < _rg->_max[0]) {
        return (*this);
    }

    _xb = 0;
    if (_x > _rg->_max[0]) {
        _x = _xb = _rg->_min[0];
        _y++;
    }
    if (_y > _rg->_max[1]) {
        if (_rg->_ndim == 2) {
            _end = true;
            return (*this);
        }
        _y = _rg->_min[1];
        _z++;
    }

    if (_rg->_ndim == 3 && _z > _rg->_max[2]) {
        _end = true;
        return (*this);
    }

    size_t xb = _x / _rg->_bs[0];
    size_t yb = _y / _rg->_bs[1];
    size_t zb = 0;
    if (_rg->_ndim == 3)
        zb = _z / _rg->_bs[2];

    size_t x = _x % _rg->_bs[0];
    size_t y = _y % _rg->_bs[1];
    size_t z = 0;
    if (_rg->_ndim == 3)
        z = _z % _rg->_bs[2];
    float *blk = _rg->_blks[zb * _rg->_bdims[0] * _rg->_bdims[1] + yb * _rg->_bdims[0] + xb];
    _itr = &blk[z * _rg->_bs[0] * _rg->_bs[1] + y * _rg->_bs[0] + x];
    return (*this);
}

template <class T>
StructuredGrid::ForwardIterator<T> StructuredGrid::ForwardIterator<T>::
operator++(int) {

    if (_end)
        return (*this);

    ForwardIterator temp(*this);
    ++(*this);
    return (temp);
}

template <class T>
StructuredGrid::ForwardIterator<T> &StructuredGrid::ForwardIterator<T>::
operator+=(const long int &offset) {

    _end = false;

    vector<size_t> min, max;
    for (int i = 0; i < _rg->_ndim; i++) {
        min.push_back(_rg->_min[i]);
        max.push_back(_rg->_max[i]);
    }

    vector<size_t> xyz;
    if (_rg->_ndim > 0)
        xyz.push_back(_x);
    if (_rg->_ndim > 1)
        xyz.push_back(_y);
    if (_rg->_ndim > 2)
        xyz.push_back(_z);

    long newoffset = Wasp::LinearizeCoords(xyz, min, max) + offset;

    size_t maxoffset = Wasp::LinearizeCoords(max, min, max);

    if (newoffset < 0 || newoffset > maxoffset) {
        _end = true;
        return (*this);
    }

    xyz = Wasp::VectorizeCoords(offset, min, max);
    _x = _y = _z = 0;

    if (_rg->_ndim > 0)
        _x = xyz[0];
    if (_rg->_ndim > 1)
        _y = xyz[1];
    if (_rg->_ndim > 2)
        _z = xyz[2];
    _xb = _x % _rg->_bs[0];

    size_t xb = _x / _rg->_bs[0];

    size_t yb = _y / _rg->_bs[1];
    size_t zb = 0;
    if (_rg->_ndim == 3)
        zb = _z / _rg->_bs[2];

    size_t x = _x % _rg->_bs[0];
    size_t y = _y % _rg->_bs[1];
    size_t z = 0;
    if (_rg->_ndim == 3)
        z = _z % _rg->_bs[2];

    float *blk = _rg->_blks[zb * _rg->_bdims[0] * _rg->_bdims[1] + yb * _rg->_bdims[0] + xb];
    _itr = &blk[z * _rg->_bs[0] * _rg->_bs[1] + y * _rg->_bs[0] + x];
    return (*this);
}

template <class T>
StructuredGrid::ForwardIterator<T> StructuredGrid::ForwardIterator<T>::
operator+(const long int &offset) const {

    ForwardIterator temp(*this);

    if (_end)
        return (temp);

    temp += offset;
    return (temp);
}

template <class T>
bool StructuredGrid::ForwardIterator<T>::
operator!=(const StructuredGrid::ForwardIterator<T> &other) {

    if (this->_end && other._end)
        return (false);

    return (!(
        this->_rg == other._rg &&
        this->_xb == other._xb &&
        this->_x == other._x &&
        this->_y == other._y &&
        this->_z == other._z &&
        this->_itr == other._itr &&
        this->_end == other._end));
}

// Need this so that template definitions can be made in .cpp file, not .h file
//
template class StructuredGrid::ForwardIterator<StructuredGrid>;
template class StructuredGrid::ForwardIterator<const StructuredGrid>;

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const StructuredGrid &rg) {
    o << "StructuredGrid " << endl
      << " Dimensions "
      << rg._max[0] - rg._min[0] + 1 << " "
      << rg._max[1] - rg._min[1] + 1 << " ";
    if (rg._ndim == 3)
        o << rg._max[2] - rg._min[2] + 1;
    o << endl;

    o << " Min voxel offset "
      << rg._min[0] << " "
      << rg._min[1] << " ";
    if (rg._ndim == 3)
        o << rg._min[2];
    o << endl;

    o << " Max voxel offset "
      << rg._max[0] << " "
      << rg._max[1] << " ";
    if (rg._ndim == 3)
        o << rg._max[2];
    o << endl;

    o << " Block dimensions"
      << rg._bs[0] << " "
      << rg._bs[1] << " ";
    if (rg._ndim == 3)
        o << rg._bs[2];
    o << endl;

    o << " Periodicity "
      << rg._periodic[0] << " "
      << rg._periodic[1] << " ";
    if (rg._ndim == 3)
        o << rg._periodic[2];
    o << endl;

    o << " Missing value " << rg._missingValue << endl
      << " Interpolation order " << rg._interpolationOrder << endl;

    return o;
}
}; // namespace VAPoR
