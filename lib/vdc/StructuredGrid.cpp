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

float StructuredGrid::AccessIndex(const std::vector<size_t> &indices) const
{
    float *fptr = _AccessIndex(_blks, indices);
    if (!fptr) return (GetMissingValue());
    return (*fptr);
}

void StructuredGrid::SetValue(const std::vector<size_t> &indices, float v)
{
    float *fptr = _AccessIndex(_blks, indices);
    if (!fptr) return;
    *fptr = v;
}

float *StructuredGrid::_AccessIndex(const std::vector<float *> &blks, const std::vector<size_t> &indices) const
{
    assert(indices.size() >= GetTopologyDim());

    if (!blks.size()) return (NULL);

    vector<size_t> dims = GetDimensions();
    size_t         ndim = dims.size();
    for (int i = 0; i < ndim; i++) {
        if (indices[i] >= dims[i]) { return (NULL); }
    }

    size_t xb = indices[0] / _bs[0];
    size_t yb = indices[1] / _bs[1];
    size_t zb = ndim == 3 ? indices[2] / _bs[2] : 0;

    size_t x = indices[0] % _bs[0];
    size_t y = indices[1] % _bs[1];
    size_t z = ndim == 3 ? indices[2] % _bs[2] : 0;

    float *blk = blks[zb * _bdims[0] * _bdims[1] + yb * _bdims[0] + xb];
    return (&blk[z * _bs[0] * _bs[1] + y * _bs[0] + x]);
}

float StructuredGrid::AccessIJK(size_t i, size_t j, size_t k) const
{
    std::vector<size_t> indices = {i, j, k};
    return (AccessIndex(indices));
}

void StructuredGrid::SetValueIJK(size_t i, size_t j, size_t k, float v)
{
    std::vector<size_t> indices = {i, j, k};
    return (SetValue(indices, v));
}

void StructuredGrid::GetRange(float range[2]) const
{
    StructuredGrid::ConstIterator itr;
    bool                          first = true;
    range[0] = range[1] = GetMissingValue();
    float missingValue = GetMissingValue();
    for (itr = this->cbegin(); itr != this->cend(); ++itr) {
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

    // Check if invalid indices
    //
    for (int i = 0; i < cindices.size(); i++) {
        if (cindices[i] > (dims[i] - 2)) return (false);
    }

    // Cells have the same ID's as their first node
    //
    // walk counter-clockwise order
    //
    vector<size_t> indices;

    if (dims.size() == 2) {
        indices = {cindices[0], cindices[1]};
        nodes.push_back(indices);

        indices = {cindices[0] + 1, cindices[1]};
        nodes.push_back(indices);

        indices = {cindices[0] + 1, cindices[1] + 1};
        nodes.push_back(indices);

        indices = {cindices[0], cindices[1] + 1};
        nodes.push_back(indices);

    } else if (dims.size() == 3 && dims[2] > 1) {
        indices = {cindices[0], cindices[1], cindices[2]};
        nodes.push_back(indices);

        indices = {cindices[0] + 1, cindices[1], cindices[2]};
        nodes.push_back(indices);

        indices = {cindices[0] + 1, cindices[1] + 1, cindices[2]};
        nodes.push_back(indices);

        indices = {cindices[0], cindices[1] + 1, cindices[2]};
        nodes.push_back(indices);

        indices = {cindices[0], cindices[1], cindices[2] + 1};
        nodes.push_back(indices);

        indices = {cindices[0] + 1, cindices[1], cindices[2] + 1};
        nodes.push_back(indices);

        indices = {cindices[0] + 1, cindices[1] + 1, cindices[2] + 1};
        nodes.push_back(indices);

        indices = {cindices[0], cindices[1] + 1, cindices[2] + 1};
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

template<class T> StructuredGrid::ForwardIterator<T>::ForwardIterator(T *rg, const vector<double> &minu, const vector<double> &maxu) : _pred(minu, maxu)
{
    if (!rg->GetBlks().size()) {
        _end = true;
        return;
    }

    vector<size_t> dims = rg->GetDimensions();
    vector<size_t> bs = rg->GetBlockSize();
    vector<size_t> bdims = rg->GetDimensionInBlks();
    assert(dims.size() > 1 && dims.size() < 4);
    for (int i = 0; i < dims.size(); i++) {
        _dims[i] = dims[i];
        _bs[i] = bs[i];
        _bdims[i] = bdims[i];
    }

    _rg = rg;
    _coordItr = rg->ConstCoordBegin();
    _x = 0;
    _y = 0;
    _z = 0;
    _xb = 0;
    _itr = &rg->GetBlks()[0][0];
    _ndim = dims.size();
    _end = false;

    if (!_pred(*_coordItr)) { operator++(); }
}

#ifdef DEAD
template<class T> StructuredGrid::ForwardIterator<T>::ForwardIterator(const ForwardIterator<T> &rhs)
{
    _rg = rhs._rg;
    _coordItr = rhs._coordItr.clone();
    _x = rhs._x;
    _y = rhs._y;
    _z = rhs._z;
    _xb = rhs._xb;
    _itr = rhs._itr;
    for (int i = 0; i < 3; i++) {
        _dims[i] = rhs._dims[i];
        _bs[i] = rhs._bs[i];
        _bdims[i] = rhs._bdims[i];
    }
    _ndim = rhs._ndim;
    _end = rhs._end;
    _pred = rhs._pred;
}
#endif

template<class T> StructuredGrid::ForwardIterator<T>::ForwardIterator(ForwardIterator<T> &&rhs)
{
    _rg = rhs._rg;
    rhs._rg = nullptr;
    _coordItr = std::move(rhs._coordItr);
    _x = rhs._x;
    _y = rhs._y;
    _z = rhs._z;
    _xb = rhs._xb;
    _itr = rhs._itr;
    rhs._itr = nullptr;
    for (int i = 0; i < 3; i++) {
        _dims[i] = rhs._dims[i];
        _bs[i] = rhs._bs[i];
        _bdims[i] = rhs._bdims[i];
    }
    _ndim = rhs._ndim;
    _end = rhs._end;
    _pred = rhs._pred;
}

template<class T> StructuredGrid::ForwardIterator<T>::ForwardIterator()
{
    _rg = nullptr;
    //_coordItr = xx;
    _x = 0;
    _y = 0;
    _z = 0;
    _xb = 0;
    _itr = nullptr;
    for (int i = 0; i < 3; i++) {
        _dims[i] = 1;
        _bs[i] = 1;
        _bdims[i] = 1;
    }
    _ndim = 0;
    _end = true;
    //_pred = xx;
}

template<class T> StructuredGrid::ForwardIterator<T> &StructuredGrid::ForwardIterator<T>::operator=(ForwardIterator<T> rhs)
{
    swap(*this, rhs);
    return (*this);
}

template<class T> StructuredGrid::ForwardIterator<T> &StructuredGrid::ForwardIterator<T>::operator++()
{
    if (!_rg->GetBlks().size()) _end = true;
    if (_end) return (*this);

    size_t xb = 0;
    size_t yb = 0;
    size_t zb = 0;
    size_t x = 0;
    size_t y = 0;
    size_t z = 0;
    do {
        _xb++;
        _itr++;
        _x++;
        ++_coordItr;

        if (_xb < _bs[0] && _x < _dims[0]) {
            if (_pred(*_coordItr)) { return (*this); }

            continue;
        }

        _xb = 0;
        if (_x >= _dims[0]) {
            _x = _xb = 0;
            _y++;
        }

        if (_y >= _dims[1]) {
            if (_ndim == 2) {
                _end = true;
                return (*this);
            }
            _y = 0;
            _z++;
        }

        if (_ndim == 3 && _z >= _dims[2]) {
            _end = true;
            return (*this);
        }

        xb = _x / _bs[0];
        yb = _y / _bs[1];
        zb = 0;
        if (_ndim == 3) zb = _z / _bs[2];

        x = _x % _bs[0];
        y = _y % _bs[1];
        z = 0;
        if (_ndim == 3) z = _z % _bs[2];

        float *blk = _rg->GetBlks()[zb * _bdims[0] * _bdims[1] + yb * _bdims[0] + xb];
        _itr = &blk[z * _bs[0] * _bs[1] + y * _bs[0] + x];

    } while (!_pred(*_coordItr));

    return (*this);
}

#ifdef DEAD
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

    float *blk = _rg->GetBlks()[zb * _bdims[0] * _bdims[1] + yb * _bdims[0] + xb];
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

#endif

template<class T> bool StructuredGrid::ForwardIterator<T>::operator!=(const StructuredGrid::ForwardIterator<T> &other)
{
    if (this->_end && other._end) return (false);

    return (!(this->_rg == other._rg && this->_xb == other._xb && this->_x == other._x && this->_y == other._y && this->_z == other._z && this->_itr == other._itr && this->_end == other._end));
}

// Need this so that template definitions can be made in .cpp file, not .h file
//
template class StructuredGrid::ForwardIterator<StructuredGrid>;
template class StructuredGrid::ForwardIterator<const StructuredGrid>;

#ifdef DEAD

// Only partially implmented for 2D cells. Need predicate cell-intersect-box
// functor
//
//	Assumes counter-clockwise node ordering:
//
//	3-------2
//	|       |
//	|       |
//	|       |
//	0-------1
//
//
template<class T> StructuredGrid::CellIterator<T>::CellIterator(T *sg, const vector<double> &minu, const vector<double> &maxu) : _pred(minu, maxu)
{
    _sg = sg;
    _dims = rg->GetDimensions();

    _cellIndex = vector<size_t>(_dims.size(), 0);

    _coordItr0 = rg->begin();
    _coordItr1 = rg->begin();
    ++_coordItr1;

    _coordItr2 = rg->begin();
    _coordItr3 = rg->begin();
    ++_coordItr2;

    for (int i = 0; i < _dims[0]; i++) { ++_coordItr2++ _coordItr3 }

    // Skip to first cell containing point (if any)
    //
    if (!_pred(*_coordItr0, *_coordItr1, *_coordItr2, *_coordItr3)) { operator++(); }
}
#endif

template<class T> StructuredGrid::ForwardCellIterator<T>::ForwardCellIterator(T *sg, bool begin)
{
    _sg = sg;
    _dims = sg->GetDimensions();

    _cellIndex = vector<size_t>(_dims.size(), 0);
    if (!begin) { _cellIndex[_dims.size() - 1] = _dims[_dims.size() - 1] - 1; }
}

template<class T> StructuredGrid::ForwardCellIterator<T>::ForwardCellIterator(ForwardCellIterator<T> &&rhs)
{
    _sg = rhs._sg;
    rhs._sg = nullptr;
    _dims = rhs._dims;
    _cellIndex = rhs._cellIndex;

#ifdef DEAD
    _coordItr0 = std::move(rhs._coordItr0);
    _coordItr1 = std::move(rhs._coordItr1);
    _coordItr2 = std::move(rhs._coordItr2);
    _coordItr3 = std::move(rhs._coordItr3);
    _coordItr4 = std::move(rhs._coordItr4);
    _coordItr5 = std::move(rhs._coordItr5);
    _coordItr6 = std::move(rhs._coordItr6);
    _coordItr7 = std::move(rhs._coordItr7);
#endif
}

template<class T> StructuredGrid::ForwardCellIterator<T>::ForwardCellIterator()
{
    _sg = nullptr;
    _dims.clear();
    _cellIndex.clear();
}

template<class T> StructuredGrid::ForwardCellIterator<T> &StructuredGrid::ForwardCellIterator<T>::operator=(ForwardCellIterator<T> rhs)
{
    swap(*this, rhs);
    return (*this);
}

template<class T> StructuredGrid::ForwardCellIterator<T> &StructuredGrid::ForwardCellIterator<T>::next2d()
{
    if (_cellIndex[1] >= (_dims[1] - 1)) return (*this);

    _cellIndex[0]++;

#ifdef DEAD
    ++_coordItr1;
    ++_coordItr2;
    ++_coordItr3;
    ++_coordItr4;
#endif

    if (_cellIndex[0] < (_dims[0] - 1)) { return (*this); }

    if (_cellIndex[0] >= (_dims[0] - 1)) {
#ifdef DEAD
        ++_coordItr1;
        ++_coordItr2;
        ++_coordItr3;
        ++_coordItr4;
#endif
        _cellIndex[0] = 0;
        _cellIndex[1]++;
    }
    return (*this);
}

template<class T> StructuredGrid::ForwardCellIterator<T> &StructuredGrid::ForwardCellIterator<T>::next3d()
{
    if (_cellIndex[2] >= (_dims[2] - 1)) return (*this);

    _cellIndex[0]++;

#ifdef DEAD
    ++_coordItr1;
    ++_coordItr2;
    ++_coordItr3;
    ++_coordItr4;
#endif

    if (_cellIndex[0] < (_dims[0] - 1)) { return (*this); }

    if (_cellIndex[0] >= (_dims[0] - 1)) {
#ifdef DEAD
        ++_coordItr1;
        ++_coordItr2;
        ++_coordItr3;
        ++_coordItr4;
#endif
        _cellIndex[0] = 0;
        _cellIndex[1]++;
    }

    if (_cellIndex[1] >= (_dims[1] - 1)) {
#ifdef DEAD
        ++_coordItr1;
        ++_coordItr2;
        ++_coordItr3;
        ++_coordItr4;
#endif
        _cellIndex[1] = 0;
        _cellIndex[2]++;
    }

    return (*this);
}

template<class T> StructuredGrid::ForwardCellIterator<T> &StructuredGrid::ForwardCellIterator<T>::operator++()
{
    if (_dims.size() == 2) return (next2d());
    if (_dims.size() == 3) return (next3d());

    assert(_dims.size() >= 2 && _dims.size() <= 3);
    return (*this);
}

// Need this so that template definitions can be made in .cpp file, not .h file
//
template class StructuredGrid::ForwardCellIterator<const StructuredGrid>;

#ifdef DEAD

// Only partially implmented for 2D node. Need predicate node-intersect-box
// functor
//
//
template<class T> StructuredGrid::NodeIterator<T>::NodeIterator(T *sg, const vector<double> &minu, const vector<double> &maxu) : _pred(minu, maxu)
{
    _sg = sg;
    _dims = rg->GetDimensions();

    _cellIndex = vector<size_t>(_dims.size(), 0);

    ....
}
#endif

template<class T> StructuredGrid::ForwardNodeIterator<T>::ForwardNodeIterator(T *sg, bool begin)
{
    _sg = sg;
    _dims = sg->GetDimensions();

    _nodeIndex = vector<size_t>(_dims.size(), 0);
    if (!begin) { _nodeIndex[_dims.size() - 1] = _dims[_dims.size() - 1]; }
}

template<class T> StructuredGrid::ForwardNodeIterator<T>::ForwardNodeIterator(ForwardNodeIterator<T> &&rhs)
{
    _sg = rhs._sg;
    rhs._sg = nullptr;
    _dims = rhs._dims;
    _nodeIndex = rhs._nodeIndex;
}

template<class T> StructuredGrid::ForwardNodeIterator<T>::ForwardNodeIterator()
{
    _sg = nullptr;
    _dims.clear();
    _nodeIndex.clear();
}

template<class T> StructuredGrid::ForwardNodeIterator<T> &StructuredGrid::ForwardNodeIterator<T>::operator=(ForwardNodeIterator<T> rhs)
{
    swap(*this, rhs);
    return (*this);
}

template<class T> StructuredGrid::ForwardNodeIterator<T> &StructuredGrid::ForwardNodeIterator<T>::next2d()
{
    if (_nodeIndex[1] >= (_dims[1])) return (*this);

    _nodeIndex[0]++;

    if (_nodeIndex[0] < (_dims[0])) { return (*this); }

    if (_nodeIndex[0] >= (_dims[0])) {
        _nodeIndex[0] = 0;
        _nodeIndex[1]++;
    }
    return (*this);
}

template<class T> StructuredGrid::ForwardNodeIterator<T> &StructuredGrid::ForwardNodeIterator<T>::next3d()
{
    if (_nodeIndex[2] >= (_dims[2] - 1)) return (*this);

    _nodeIndex[0]++;

#ifdef DEAD
    ++_coordItr1;
    ++_coordItr2;
    ++_coordItr3;
    ++_coordItr4;
#endif

    if (_nodeIndex[0] < (_dims[0])) { return (*this); }

    if (_nodeIndex[0] >= (_dims[0])) {
        _nodeIndex[0] = 0;
        _nodeIndex[1]++;
    }

    if (_nodeIndex[1] >= (_dims[1])) {
        _nodeIndex[1] = 0;
        _nodeIndex[2]++;
    }

    return (*this);
}

template<class T> StructuredGrid::ForwardNodeIterator<T> &StructuredGrid::ForwardNodeIterator<T>::operator++()
{
    if (_dims.size() == 2) return (next2d());
    if (_dims.size() == 3) return (next3d());

    assert(_dims.size() >= 2 && _dims.size() <= 3);
    return (*this);
}

// Need this so that template definitions can be made in .cpp file, not .h file
//
template class StructuredGrid::ForwardNodeIterator<const StructuredGrid>;

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const StructuredGrid &sg)
{
    o << "StructuredGrid " << endl;
    o << " Block dimensions ";
    for (int i = 0; i < sg._bs.size(); i++) { o << sg._bs[i] << " "; }
    o << endl;

    o << " Grid dimensions in blocks ";
    for (int i = 0; i < sg._bdims.size(); i++) { o << sg._bdims[i] << " "; }
    o << endl;

    o << (const Grid &)sg;

    return o;
}
};    // namespace VAPoR
