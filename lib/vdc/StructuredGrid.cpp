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

void StructuredGrid::_StructuredGrid(const vector<size_t> &dims, const vector<size_t> &bs, const vector<float *> &blks)
{
    assert(bs.size() == 2 || bs.size() == 3);
    assert(bs.size() == dims.size());

    _bs.clear();
    _bdims.clear();
    _blks.clear();

    for (int i = 0; i < bs.size(); i++) {
        assert(bs[i] > 0);
        assert(dims[i] > 0);

        _bs.push_back(bs[i]);
        _bdims.push_back(((dims[i] - 1) / bs[i]) + 1);
    }

    //
    // Shallow  copy blocks
    //
    _blks = blks;
}

StructuredGrid::StructuredGrid(const vector<size_t> &dims, const vector<size_t> &bs, const vector<float *> &blks) : Grid(dims, dims.size()) { _StructuredGrid(dims, bs, blks); }

StructuredGrid::StructuredGrid()
{
    _bs.clear();
    _bdims.clear();
    _blks.clear();
}

StructuredGrid::~StructuredGrid() {}

float StructuredGrid::AccessIndex(const std::vector<size_t> &indices) const { return (_AccessIndex(_blks, indices)); }

float StructuredGrid::_AccessIndex(const std::vector<float *> &blks, const std::vector<size_t> &indices) const
{
    assert(indices.size() == GetTopologyDim());

    if (!blks.size()) return (GetMissingValue());

    vector<size_t> dims = GetDimensions();
    size_t         ndim = dims.size();
    for (int i = 0; i < ndim; i++) {
        if (indices[i] >= dims[i]) { return (GetMissingValue()); }
    }

    size_t xb = indices[0] / _bs[0];
    size_t yb = indices[1] / _bs[1];
    size_t zb = ndim == 3 ? indices[2] / _bs[2] : 0;

    size_t x = indices[0] % _bs[0];
    size_t y = indices[1] % _bs[1];
    size_t z = ndim == 3 ? indices[2] % _bs[2] : 0;

    float *blk = blks[zb * _bdims[0] * _bdims[1] + yb * _bdims[0] + xb];
    return (blk[z * _bs[0] * _bs[1] + y * _bs[0] + x]);
}

float StructuredGrid::AccessIJK(size_t i, size_t j, size_t k) const
{
    std::vector<size_t> indices;
    indices.push_back(i);
    indices.push_back(j);
    if (GetTopologyDim() == 3) { indices.push_back(k); }
    return (AccessIndex(indices));
}

void StructuredGrid::GetRange(float range[2]) const
{
    StructuredGrid::ConstIterator itr;
    bool                          first = true;
    range[0] = range[1] = GetMissingValue();
    float missingValue = GetMissingValue();
    for (itr = this->begin(); itr != this->end(); ++itr) {
        if (first && *itr != missingValue) {
            range[0] = range[1] = *itr;
            first = false;
        }

        if (!first) {
            if (*itr < range[0] && *itr != missingValue)
                range[0] = *itr;
            else if (*itr > range[1] && *itr != missingValue)
                range[1] = *itr;
        }
    }
}

bool StructuredGrid::GetCellNodes(const std::vector<size_t> &cindices, std::vector<vector<size_t>> &nodes) const
{
    nodes.clear();

    vector<size_t> dims = GetDimensions();
    assert(cindices.size() == dims.size());

    assert((dims.size() == 2) && "3D cells not yet supported");

    // Check if invalid indices
    //
    for (int i = 0; i < cindices.size(); i++) {
        if (cindices[i] > (dims[i] - 2)) return (false);
    }

    // Cells have the same ID's as their first node
    //
    // walk counter-clockwise order
    //
    if (dims.size() == 2) {
        vector<size_t> indices;

        indices = {cindices[0], cindices[1]};
        nodes.push_back(indices);

        indices = {cindices[0] + 1, cindices[1]};
        nodes.push_back(indices);

        indices = {cindices[0] + 1, cindices[1] + 1};
        nodes.push_back(indices);

        indices = {cindices[0], cindices[1] + 1};
        nodes.push_back(indices);
    }

    return (true);
}

bool StructuredGrid::GetCellNeighbors(const std::vector<size_t> &cindices, std::vector<vector<size_t>> &cells) const
{
    cells.clear();

    vector<size_t> dims = GetDimensions();
    assert(cindices.size() == dims.size());

    assert((dims.size() == 2) && "3D cells not yet supported");

    // Check if invalid indices
    //
    for (int i = 0; i < cindices.size(); i++) {
        if (cindices[i] > (dims[i] - 2)) return (false);
    }

    // Cells have the same ID's as their first node
    //
    // walk counter-clockwise order
    //
    if (dims.size() == 2) {
        vector<size_t> indices;

        if (cindices[1] != 0) {    // below
            indices = {cindices[0], cindices[1] - 1};
        }
        cells.push_back(indices);

        if (cindices[0] != dims[0] - 2) {    // right
            indices = {cindices[0] + 1, cindices[1]};
        }
        cells.push_back(indices);

        if (cindices[1] != dims[1] - 2) {    // top
            indices = {cindices[0], cindices[1] + 1};
        }
        cells.push_back(indices);

        if (cindices[0] != 0) {    // left
            indices = {cindices[0] - 1, cindices[1]};
        }
        cells.push_back(indices);
    }
    return (true);
}

bool StructuredGrid::GetNodeCells(const std::vector<size_t> &indices, std::vector<vector<size_t>> &cells) const
{
    cells.clear();

    vector<size_t> dims = GetDimensions();
    assert(indices.size() == dims.size());

    assert((dims.size() == 2) && "3D cells not yet supported");

    // Check if invalid indices
    //
    for (int i = 0; i < indices.size(); i++) {
        if (indices[i] > (dims[i] - 1)) return (false);
    }

    if (dims.size() == 2) {
        vector<size_t> indices;

        if (indices[0] != 0 && indices[1] != 0) {    // below, left
            indices = {indices[0] - 1, indices[1] - 1};
            cells.push_back(indices);
        }

        if (indices[1] != 0) {    // below, right
            indices = {indices[0], indices[1] - 1};
            cells.push_back(indices);
        }

        if (indices[0] != (dims[0] - 1) && indices[1] != (dims[1])) {    // top, right
            indices = {indices[0], indices[1]};
            cells.push_back(indices);
        }

        if (indices[0] != 0) {    // top, top
            indices = {indices[0] - 1, indices[1]};
            cells.push_back(indices);
        }
    }
    return (true);
}

void StructuredGrid::ClampCoord(std::vector<double> &coords) const
{
    assert(coords.size() >= GetTopologyDim());

    while (coords.size() > GetTopologyDim()) { coords.pop_back(); }

    vector<bool>   periodic = GetPeriodic();
    vector<size_t> dims = GetDimensions();

    vector<double> minu, maxu;
    GetUserExtents(minu, maxu);

    for (int i = 0; i < coords.size(); i++) {
        //
        // Handle coordinates for dimensions of length 1
        //
        if (dims[i] == 1) {
            coords[i] = minu[i];
            continue;
        }

        if (coords[i] < minu[i] && periodic[i]) {
            while (coords[i] < minu[i]) coords[i] += maxu[i] - minu[i];
        }
        if (coords[i] > maxu[i] && periodic[i]) {
            while (coords[i] > maxu[i]) coords[i] -= maxu[i] - minu[i];
        }
    }
}

template<class T> StructuredGrid::ForwardIterator<T>::ForwardIterator(T *rg)
{
    if (!rg->_blks.size()) {
        _end = true;
        return;
    }

    vector<size_t> dims = rg->GetDimensions();
    vector<size_t> bs = rg->GetBlockSize();
    vector<size_t> bdims = rg->GetDimensionInBlks();
    assert(dims.size() > 1 && dims.size() < 4);
    for (int i = 0; i < dims.size(); i++) {
        _max[i] = dims[i] - 1;
        _bs[i] = bs[i];
        _bdims[i] = bdims[i];
    }

    _rg = rg;
    _x = 0;
    _y = 0;
    _z = 0;
    _xb = 0;
    _itr = &rg->_blks[0][0];
    _ndim = dims.size();
    _end = false;
}

template<class T> StructuredGrid::ForwardIterator<T>::ForwardIterator()
{
    _rg = NULL;
    _xb = 0;
    _x = 0;
    _y = 0;
    _z = 0;
    _itr = NULL;
    _end = true;
}

template<class T> StructuredGrid::ForwardIterator<T> &StructuredGrid::ForwardIterator<T>::operator++()
{
    if (!_rg->_blks.size()) _end = true;
    if (_end) return (*this);

    _xb++;
    _itr++;
    _x++;
    if (_xb < _bs[0] && _x < _max[0]) { return (*this); }

    _xb = 0;
    if (_x > _max[0]) {
        _x = _xb = 0;
        _y++;
    }

    if (_y > _max[1]) {
        if (_ndim == 2) {
            _end = true;
            return (*this);
        }
        _y = 0;
        _z++;
    }

    if (_ndim == 3 && _z > _max[2]) {
        _end = true;
        return (*this);
    }

    size_t xb = _x / _bs[0];
    size_t yb = _y / _bs[1];
    size_t zb = 0;
    if (_ndim == 3) zb = _z / _bs[2];

    size_t x = _x % _bs[0];
    size_t y = _y % _bs[1];
    size_t z = 0;
    if (_ndim == 3) z = _z % _bs[2];
    float *blk = _rg->_blks[zb * _bdims[0] * _bdims[1] + yb * _bdims[0] + xb];
    _itr = &blk[z * _bs[0] * _bs[1] + y * _bs[0] + x];
    return (*this);
}

template<class T> StructuredGrid::ForwardIterator<T> StructuredGrid::ForwardIterator<T>::operator++(int)
{
    if (_end) return (*this);

    ForwardIterator temp(*this);
    ++(*this);
    return (temp);
}

template<class T> StructuredGrid::ForwardIterator<T> &StructuredGrid::ForwardIterator<T>::operator+=(const long int &offset)
{
    _end = false;

    vector<size_t> min, max;
    for (int i = 0; i < _ndim; i++) {
        min.push_back(0);
        max.push_back(0);
    }

    vector<size_t> xyz;
    if (_ndim > 0) xyz.push_back(_x);
    if (_ndim > 1) xyz.push_back(_y);
    if (_ndim > 2) xyz.push_back(_z);

    long newoffset = Wasp::LinearizeCoords(xyz, min, max) + offset;

    size_t maxoffset = Wasp::LinearizeCoords(max, min, max);

    if (newoffset < 0 || newoffset > maxoffset) {
        _end = true;
        return (*this);
    }

    xyz = Wasp::VectorizeCoords(offset, min, max);
    _x = _y = _z = 0;

    if (_ndim > 0) _x = xyz[0];
    if (_ndim > 1) _y = xyz[1];
    if (_ndim > 2) _z = xyz[2];
    _xb = _x % _bs[0];

    size_t xb = _x / _bs[0];

    size_t yb = _y / _bs[1];
    size_t zb = 0;
    if (_ndim == 3) zb = _z / _bs[2];

    size_t x = _x % _bs[0];
    size_t y = _y % _bs[1];
    size_t z = 0;
    if (_ndim == 3) z = _z % _bs[2];

    float *blk = _rg->_blks[zb * _bdims[0] * _bdims[1] + yb * _bdims[0] + xb];
    _itr = &blk[z * _bs[0] * _bs[1] + y * _bs[0] + x];
    return (*this);
}

template<class T> StructuredGrid::ForwardIterator<T> StructuredGrid::ForwardIterator<T>::operator+(const long int &offset) const
{
    ForwardIterator temp(*this);

    if (_end) return (temp);

    temp += offset;
    return (temp);
}

template<class T> bool StructuredGrid::ForwardIterator<T>::operator!=(const StructuredGrid::ForwardIterator<T> &other)
{
    if (this->_end && other._end) return (false);

    return (!(this->_rg == other._rg && this->_xb == other._xb && this->_x == other._x && this->_y == other._y && this->_z == other._z && this->_itr == other._itr && this->_end == other._end));
}

// Need this so that template definitions can be made in .cpp file, not .h file
//
template class StructuredGrid::ForwardIterator<StructuredGrid>;
template class StructuredGrid::ForwardIterator<const StructuredGrid>;

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const StructuredGrid &sg)
{
    o << "StructuredGrid " << endl;
    o << " Block dimensions";
    for (int i = 0; i < sg._bs.size(); i++) { o << sg._bs[i] << " "; }
    o << endl;

    o << " Grid dimensions in blocks";
    for (int i = 0; i < sg._bdims.size(); i++) { o << sg._bdims[i] << " "; }
    o << endl;

    o << (const Grid &)sg;

    return o;
}
};    // namespace VAPoR
