#include <iostream>
#include <vector>
#include "vapor/VAssert.h"
#include <numeric>
#include <cmath>
#include <cassert>
#include <algorithm>
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
#include <vapor/Grid.h>
#include <vapor/OpenMPSupport.h>

using namespace std;
using namespace VAPoR;

Grid::Grid() { _dims = {1, 1, 1}; }

void Grid::_grid(const DimsType &dims, const DimsType &bs, const std::vector<float *> &blks, size_t topology_dimension)
{
    for (int i = 0; i < bs.size(); i++) {
        VAssert(bs[i] > 0);
        VAssert(dims[i] > 0);

        _bdims[i] = ((dims[i] - 1) / bs[i]) + 1;
        _bs[i] = bs[i];
        _bdimsDeprecated.push_back(_bdims[i]);
        _bsDeprecated.push_back(_bs[i]);
    }
    VAssert(blks.size() == 0 ||    // dataless
            blks.size() == std::accumulate(_bdims.begin(), _bdims.end(), 1, std::multiplies<size_t>()));

    _dims = dims;
    _periodic = vector<bool>(topology_dimension, false);
    _topologyDimension = topology_dimension;
    _missingValue = INFINITY;
    _hasMissing = false;
    _interpolationOrder = 0;
    _nodeIDOffset = 0;
    _cellIDOffset = 0;
    _minAbs = {0, 0, 0};

    //
    // Shallow  copy blocks
    //
    _blks = blks;
}

Grid::Grid(const DimsType &dims, const DimsType &bs, const std::vector<float *> &blks, size_t topology_dimension) { _grid(dims, bs, blks, topology_dimension); }

Grid::Grid(const std::vector<size_t> &dimsv, const std::vector<size_t> &bsv, const std::vector<float *> &blks, size_t topology_dimension)
{
    VAssert(dimsv.size() <= 3);
    VAssert(dimsv.size() == bsv.size());

    DimsType dims = {1, 1, 1};
    DimsType bs = {1, 1, 1};
    CopyToArr3(dimsv, dims);
    CopyToArr3(bsv, bs);
    _grid(dims, bs, blks, topology_dimension);
}

size_t Grid::GetNumDimensions(DimsType dims)
{
    size_t nDims = 0;
    for (size_t i = 0; i < dims.size(); i++) {
        VAssert(dims[i] > 0);
        if (dims[i] > 1) nDims++;
    }
    return (nDims);
}


float Grid::GetMissingValue() const { return (_missingValue); }

void Grid::GetUserExtents(CoordType &minu, CoordType &maxu) const
{
    size_t n = min(GetGeometryDim(), _minuCache.size());
    auto   p = [](double v) { return (v == std::numeric_limits<double>::infinity()); };
    if (std::any_of(_minuCache.begin(), _minuCache.begin() + n, p) || std::any_of(_maxuCache.begin(), _maxuCache.begin() + n, p)) {
        _minuCache = {0.0, 0.0, 0.0};
        _maxuCache = {0.0, 0.0, 0.0};
        GetUserExtentsHelper(_minuCache, _maxuCache);
    }

    minu = _minuCache;
    maxu = _maxuCache;
}

float Grid::GetValueAtIndex(const DimsType &indices) const
{
    float *fptr = GetValuePtrAtIndex(_blks, indices);
    if (!fptr) return (GetMissingValue());
    return (*fptr);
}

void Grid::SetValue(const DimsType &indices, float v)
{
    float *fptr = GetValuePtrAtIndex(_blks, indices);
    if (!fptr) return;
    *fptr = v;
}

float *Grid::GetValuePtrAtIndex(const std::vector<float *> &blks, const DimsType &indices) const
{
    if (!blks.size()) return (NULL);

    DimsType cIndices;
    ClampIndex(indices, cIndices);

    size_t xb = cIndices[0] / _bs[0];
    size_t yb = cIndices[1] / _bs[1];
    size_t zb = cIndices[2] / _bs[2];

    size_t x = cIndices[0] % _bs[0];
    size_t y = cIndices[1] % _bs[1];
    size_t z = cIndices[2] % _bs[2];

    float *blk = blks[zb * _bdims[0] * _bdims[1] + yb * _bdims[0] + xb];
    return (&blk[z * _bs[0] * _bs[1] + y * _bs[0] + x]);
}

float Grid::AccessIJK(size_t i, size_t j, size_t k) const
{
    DimsType indices = {i, j, k};
    return (GetValueAtIndex(indices));
}

void Grid::SetValueIJK(size_t i, size_t j, size_t k, float v)
{
    DimsType indices = {i, j, k};
    return (SetValue(indices, v));
}

void Grid::GetRange(float range[2]) const
{
    const auto missingVal = GetMissingValue();
    const auto num_vals = _dims[0] * _dims[1] * _dims[2];
    auto num_threads = size_t{1};
#pragma omp parallel
    {
      if (omp_get_thread_num() == 0)
        num_threads = omp_get_num_threads();
    }
    const auto stride_size = num_vals / num_threads;

    auto min_vec = std::vector<float>(num_threads, missingVal);
    auto max_vec = std::vector<float>(num_threads, missingVal);

#pragma omp parallel for
    for (size_t i = 0; i < num_threads; i++) {
      auto beg = this->cbegin() + i * stride_size;
      auto end = beg;
      if (i < num_threads - 1) 
        end += stride_size;
      else
        end = this->cend();

      while (*beg == missingVal && beg != end)
        ++beg;

      if (beg != end) {
        auto min = *beg;
        auto max = *beg;
        for (auto itr  = beg + 1; itr != end; ++itr) {
          if (*itr != missingVal) {
            min = std::min(*itr, min);      
            max = std::max(*itr, max);      
          }
        }
        min_vec[i] = min;
        max_vec[i] = max;
      }
    } // finish omp section

    if (std::all_of(min_vec.begin(), min_vec.end(), 
        [missingVal](float v){return v == missingVal;})) {
      range[0] = missingVal;
      range[1] = missingVal;
    }
    else {
      auto it = std::remove(min_vec.begin(), min_vec.end(), missingVal);
      range[0] = *std::min_element(min_vec.begin(), it);
      it = std::remove(max_vec.begin(), max_vec.end(), missingVal);
      range[1] = *std::max_element(max_vec.begin(), it);
    }
}

void Grid::GetRange(const DimsType &min, const DimsType &max, float range[2]) const
{
    DimsType cMin;
    ClampIndex(min, cMin);

    DimsType cMax;
    ClampIndex(max, cMax);

    float mv = GetMissingValue();

    range[0] = range[1] = mv;

    bool first = true;
    for (size_t k = cMin[2]; k <= cMax[2]; k++) {
        for (size_t j = cMin[1]; j <= cMax[1]; j++) {
            for (size_t i = cMin[0]; i <= cMax[0]; i++) {
                float v = AccessIJK(i, j, k);
                if (first && v != mv) {
                    range[0] = range[1] = v;
                    first = false;
                }

                if (!first) {
                    if (v < range[0] && v != mv)
                        range[0] = v;
                    else if (v > range[1] && v != mv)
                        range[1] = v;
                }
            }
        }
    }
}

float Grid::GetValue(const CoordType &coords) const
{
    if (!_blks.size()) return (GetMissingValue());

    // Clamp coordinates on periodic boundaries to grid extents
    //
    CoordType cCoords;
    ClampCoord(coords, cCoords);

#ifdef VAPOR3_0_0_ALPHA
    // At this point xyz should be within the grid bounds
    //
    if (!InsideGrid(cCoords)) return (_missingValue);
#endif

    if (_interpolationOrder == 0) {
        return (GetValueNearestNeighbor(cCoords));
    } else {
        return (GetValueLinear(cCoords));
    }
}


void Grid::GetUserCoordinates(size_t i, double &x, double &y, double &z) const
{
    x = y = z = 0.0;
    vector<size_t> indices = {i};
    vector<double> coords;
    GetUserCoordinates(indices, coords);
    x = coords[0];
    y = coords[1];
    z = coords[2];
}

void Grid::GetUserCoordinates(size_t i, size_t j, double &x, double &y, double &z) const
{
    x = y = z = 0.0;
    vector<size_t> indices = {i, j};
    vector<double> coords;
    GetUserCoordinates(indices, coords);
    x = coords[0];
    y = coords[1];
    z = coords[2];
}

void Grid::GetUserCoordinates(size_t i, size_t j, size_t k, double &x, double &y, double &z) const
{
    x = y = z = 0.0;
    vector<size_t> indices = {i, j, k};
    vector<double> coords;
    GetUserCoordinates(indices, coords);
    x = coords[0];
    y = coords[1];
    z = coords[2];
}

void Grid::SetInterpolationOrder(int order)
{
    if (order < 0 || order > 2) order = 1;
    _interpolationOrder = order;
}

DimsType Grid::Dims(const DimsType &min, const DimsType &max)
{
    DimsType dims;

    for (int i = 0; i < min.size(); i++) { dims[i] = (max[i] - min[i] + 1); }
    return (dims);
}

/////////////////////////////////////////////////////////////////////////////
//
// Iterators
//
/////////////////////////////////////////////////////////////////////////////

Grid::ConstNodeIteratorSG::ConstNodeIteratorSG(const Grid *g, bool begin) : ConstNodeIteratorAbstract()
{
    _dims = g->GetNodeDimensions();
    _index = {0, 0, 0};
    _lastIndex = {0, 0, _dims[_dims.size() - 1]};

    if (!begin) { _index = _lastIndex; }
}

Grid::ConstNodeIteratorSG::ConstNodeIteratorSG(const ConstNodeIteratorSG &rhs) : ConstNodeIteratorAbstract()
{
    _dims = rhs._dims;
    _index = rhs._index;
    _lastIndex = rhs._lastIndex;
}

Grid::ConstNodeIteratorSG::ConstNodeIteratorSG() : ConstNodeIteratorAbstract()
{
    _dims = {1, 1, 1};
    _index = {0, 0, 0};
    _lastIndex = {0, 0, 0};
}

void Grid::ConstNodeIteratorSG::next()
{
    _index[0]++;
    if (_index[0] < _dims[0]) { return; }

    _index[0] = 0;
    _index[1]++;
    if (_index[1] < _dims[1]) { return; }

    _index[1] = 0;
    _index[2]++;
    if (_index[2] < _dims[2]) { return; }

    _index[2] = _dims[2];    // last index;
}

void Grid::ConstNodeIteratorSG::next(const long &offset)
{
    if (!_index.size()) return;

    long maxIndexL = Wasp::VProduct(_dims.data(), _dims.size()) - 1;
    long newIndexL = Wasp::LinearizeCoords(_index.data(), _dims.data(), _dims.size()) + offset;
    if (newIndexL < 0) { newIndexL = 0; }
    if (newIndexL > maxIndexL) {
        _index = _lastIndex;
        return;
    }

    Wasp::VectorizeCoords(newIndexL, _dims.data(), _index.data(), _dims.size());
}

Grid::ConstNodeIteratorBoxSG::ConstNodeIteratorBoxSG(const Grid *g, const CoordType &minu, const CoordType &maxu) : ConstNodeIteratorSG(g, true), _pred(minu, maxu)
{
    _coordItr = g->ConstCoordBegin();

    // Advance to first node inside box
    //
    if (!_pred(*_coordItr)) { next(); }
}

Grid::ConstNodeIteratorBoxSG::ConstNodeIteratorBoxSG(const ConstNodeIteratorBoxSG &rhs) : ConstNodeIteratorSG()
{
    _coordItr = rhs._coordItr;
    _pred = rhs._pred;
}

Grid::ConstNodeIteratorBoxSG::ConstNodeIteratorBoxSG() : ConstNodeIteratorSG() {}

void Grid::ConstNodeIteratorBoxSG::next()
{
    do {
        ConstNodeIteratorSG::next();
        ++_coordItr;
    } while (_index != _lastIndex && !_pred(*_coordItr));
}

void Grid::ConstNodeIteratorBoxSG::next(const long &offset)
{
    _coordItr += offset;

    while (_index != _lastIndex && !_pred(*_coordItr)) {
        ConstNodeIteratorSG::next();
        ++_coordItr;
    }
}

Grid::ConstCellIteratorSG::ConstCellIteratorSG(const Grid *g, bool begin) : ConstCellIteratorAbstract()
{
    _dims = g->GetCellDimensions();
    _index = {0, 0, 0};
    _lastIndex = {0, 0, _dims[_dims.size() - 1]};

    if (!begin) { _index = _lastIndex; }
}

Grid::ConstCellIteratorSG::ConstCellIteratorSG(const ConstCellIteratorSG &rhs) : ConstCellIteratorAbstract()
{
    _dims = rhs._dims;
    _index = rhs._index;
    _lastIndex = rhs._lastIndex;
}

Grid::ConstCellIteratorSG::ConstCellIteratorSG() : ConstCellIteratorAbstract()
{
    _dims = {1, 1, 1};
    _index = {0, 0, 0};
    _lastIndex = {0, 0, 0};
}

void Grid::ConstCellIteratorSG::next()
{
    _index[0]++;
    if (_index[0] < (_dims[0])) { return; }

    _index[0] = 0;
    _index[1]++;
    if (_index[1] < (_dims[1])) { return; }

    _index[1] = 0;
    _index[2]++;
    if (_index[2] < (_dims[2])) { return; }

    _index[2] = _dims[2];    // last index;
}

void Grid::ConstCellIteratorSG::next(const long &offset)
{
    long maxIndexL = Wasp::VProduct(_dims.data(), _dims.size()) - 1;
    long newIndexL = Wasp::LinearizeCoords(_index.data(), _dims.data(), _dims.size()) + offset;
    if (newIndexL < 0) { newIndexL = 0; }
    if (newIndexL > maxIndexL) {
        _index = _lastIndex;
        return;
    }

    Wasp::VectorizeCoords(newIndexL, _dims.data(), _index.data(), _dims.size());
}

bool Grid::ConstCellIteratorBoxSG::_cellInsideBox(const size_t cindices[]) const
{
#ifdef VAPOR3_0_0
    size_t  maxNodes = _g->GetMaxVertexPerCell();
    size_t  nodeDim = _g->GetNodeDimensions().size();
    size_t *nodes = (size_t *)alloca(sizeof(size_t) * maxNodes * nodeDim);
    size_t  coordDim = _g->GetGeometryDim();
    double *coord = (double *)alloca(sizeof(double) * coordDim);

    int  numNodes;
    bool status = _g->GetCellNodes(cindices, nodes, numNodes);
    if (!status) return (false);

    for (int i = 0; i < numNodes; i++) {
        _g->GetUserCoordinates(&nodes[i * nodeDim], coord);
        if (!_pred(coord)) return (false);
    }
#endif

    return (true);
}

Grid::ConstCellIteratorBoxSG::ConstCellIteratorBoxSG(const Grid *g, const CoordType &minu, const CoordType &maxu) : ConstCellIteratorSG(g, true), _pred(minu, maxu)
{
#ifdef VAPOR3_0_0
    // Advance to first node inside box
    //
    _coordItr = g->ConstCoordBegin();

    if (!_pred(*_coordItr)) { next(); }
#else
    _g = g;

    // Advance to first node inside box
    //
    if (!_cellInsideBox(_index.data())) { next(); }
#endif
}

Grid::ConstCellIteratorBoxSG::ConstCellIteratorBoxSG(const ConstCellIteratorBoxSG &rhs) : ConstCellIteratorSG()
{
#ifdef VAPOR3_0_0
    _coordItr = rhs._coordItr;
#endif
    _pred = rhs._pred;
}

Grid::ConstCellIteratorBoxSG::ConstCellIteratorBoxSG() : ConstCellIteratorSG() {}

void Grid::ConstCellIteratorBoxSG::next()
{
#ifdef VAPOR3_0_0
    do {
        ConstCellIteratorSG::next();
        ++_coordItr;
    } while (!_pred(*_coordItr) && _index != _lastIndex);
#else
    do {
        ConstCellIteratorSG::next();
    } while (!_cellInsideBox(_index.data()) && _index != _lastIndex);
#endif
}

void Grid::ConstCellIteratorBoxSG::next(const long &offset)
{
#ifdef VAPOR3_0_0
    _coordItr += offset;
    while (!_pred(*_coordItr) && _index != _lastIndex) {
        ConstCellIteratorSG::next();
        ++_coordItr;
    }

#else

    long count = offset;
    while (!_cellInsideBox(_index.data()) && _index != _lastIndex && count > 0) {
        ConstCellIteratorSG::next();
        count--;
    }
#endif
}

//
//
// Iterators
//
//

template<class T> Grid::ForwardIterator<T>::ForwardIterator(T *rg, bool begin, const CoordType &minu, const CoordType &maxu) : _pred(minu, maxu)
{
    _blks = rg->GetBlks();

    _dims3d = rg->GetDimensions();
    _bdims3d = {1, 1, 1};
    _bs3d = {1, 1, 1};
    CopyToArr3(rg->GetDimensionInBlks(), _bdims3d);
    CopyToArr3(rg->GetBlockSize(), _bs3d);

    _blocksize = Wasp::VProduct(_bs3d.data(), _bs3d.size());

    _index = {0, 0, 0};
    _lastIndex = {0, 0, _dims3d[_dims3d.size() - 1]};

    if (!begin || !_blks.size()) {
        _index = _lastIndex;
        return;
    }

    _coordItr = rg->ConstCoordBegin();
    _xb = 0;
    _itr = &_blks[0][0];

    if (!_pred(*_coordItr)) { operator++(); }
}

template<class T> Grid::ForwardIterator<T>::ForwardIterator(ForwardIterator<T> &&rhs)
{
    _blks = rhs._blks;
    _dims3d = rhs._dims3d;
    _bdims3d = rhs._bdims3d;
    _bs3d = rhs._bs3d;
    _blocksize = rhs._blocksize;
    _coordItr = std::move(rhs._coordItr);
    _index = rhs._index;
    _xb = rhs._xb;
    _itr = rhs._itr;
    rhs._itr = nullptr;
    _pred = rhs._pred;
}

template<class T> Grid::ForwardIterator<T>::ForwardIterator()
{
    _blks.clear();
    _dims3d = {1, 1, 1};
    _bdims3d = {1, 1, 1};
    _bs3d = {1, 1, 1};
    _blocksize = 1;
    _index = {0, 0, 0};
    _xb = 0;
    _itr = nullptr;
}

template<class T> Grid::ForwardIterator<T> &Grid::ForwardIterator<T>::operator=(ForwardIterator<T> rhs)
{
    swap(*this, rhs);
    return (*this);
}

template<class T> Grid::ForwardIterator<T> &Grid::ForwardIterator<T>::operator++()
{
    if (!_blks.size()) return (*this);

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
        if (_pred.Enabled()) ++_coordItr;

        if (_xb < _bs3d[0] && _index[0] < _dims3d[0]) {
            if (_pred(*_coordItr)) { return (*this); }

            continue;
        }

        _xb = 0;
        if (_index[0] >= _dims3d[0]) {
            _index[0] = _xb = 0;
            _index[1]++;
        }

        if (_index[1] >= _dims3d[1]) {
            _index[1] = 0;
            _index[2]++;
        }

        if (_index == _lastIndex) {
            return (*this);    // last element
        }

        x = _index[0] % _bs3d[0];
        xb = _index[0] / _bs3d[0];
        y = _index[1] % _bs3d[1];
        yb = _index[1] / _bs3d[1];
        z = _index[2] % _bs3d[2];
        zb = _index[2] / _bs3d[2];

        float *blk = _blks[zb * _bdims3d[0] * _bdims3d[1] + yb * _bdims3d[0] + xb];
        _itr = &blk[z * _bs3d[0] * _bs3d[1] + y * _bs3d[0] + x];

    } while (_index != _lastIndex && !_pred(*_coordItr));

    return (*this);
}

template<class T> Grid::ForwardIterator<T> Grid::ForwardIterator<T>::operator++(int)
{
    ForwardIterator temp(*this);
    ++(*this);
    return (temp);
}

template<class T> Grid::ForwardIterator<T> &Grid::ForwardIterator<T>::operator+=(const long int &offset)
{
    VAssert(offset >= 0);

    if (!_blks.size()) return (*this);

    long maxIndexL = Wasp::VProduct(_dims3d.data(), _dims3d.size()) - 1;
    long indexL = Wasp::LinearizeCoords(_index.data(), _dims3d.data(), _index.size()) + offset;

    // Check for overflow
    //
    if (indexL < 0) { indexL = 0; }
    if (indexL > maxIndexL) {
        _index = _lastIndex;
        return (*this);
    }

    Wasp::VectorizeCoords(indexL, _dims3d.data(), _index.data(), _index.size());
    if (_pred.Enabled()) _coordItr += offset;

    size_t x = _index[0] % _bs3d[0];
    size_t xb = _index[0] / _bs3d[0];
    size_t y = _index[1] % _bs3d[1];
    size_t yb = _index[1] / _bs3d[1];
    size_t z = _index[2] % _bs3d[2];
    size_t zb = _index[2] / _bs3d[2];

    _xb = x;

    float *blk = _blks[zb * _bdims3d[0] * _bdims3d[1] + yb * _bdims3d[0] + xb];
    _itr = &blk[z * _bs3d[0] * _bs3d[1] + y * _bs3d[0] + x];

    // If no predicate, or if there is a predicate and it evaluates to
    // true, we're done.
    //
    if (!_pred.Enabled() || _pred(*_coordItr)) { return (*this); }

    // Let operator++ increment until predicate passes (or end of list)
    //
    return (++(*this));
}

template<class T> Grid::ForwardIterator<T> Grid::ForwardIterator<T>::operator+(const long int &offset) const
{
    ForwardIterator temp(*this);

    temp += offset;
    return (temp);
}

// Need this so that template definitions can be made in .cpp file, not .h file
//
template class Grid::ForwardIterator<Grid>;
template class Grid::ForwardIterator<const Grid>;

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const Grid &g)
{
    o << "Grid" << endl;

    o << " Dimensions ";
    for (int i = 0; i < g._dims.size(); i++) { o << g._dims[i] << " "; }
    o << endl;

    o << " Block dimensions ";
    for (int i = 0; i < g._bs.size(); i++) { o << g._bs[i] << " "; }
    o << endl;

    o << " Grid dimensions in blocks ";
    for (int i = 0; i < g._bdims.size(); i++) { o << g._bdims[i] << " "; }
    o << endl;

    o << " Topological dimension " << g._topologyDimension << endl;

    o << " Periodicity ";
    for (int i = 0; i < g._periodic.size(); i++) { o << g._periodic[i] << " "; }
    o << endl;

    o << " Missing value flag " << g._hasMissing << endl;
    o << " Missing value " << g._missingValue << endl;
    o << " Interpolation order " << g._interpolationOrder << endl;

    return (o);
}
};    // namespace VAPoR
