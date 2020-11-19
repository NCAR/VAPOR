#include <iostream>
#include <omp.h>
#include <vapor/VAssert.h>
#include <vapor/QuadTreeRectangleP.h>

using namespace VAPoR;

template<typename T, typename S> QuadTreeRectangleP<T, S>::QuadTreeRectangleP(T left, T top, T right, T bottom, size_t max_depth, size_t reserve_size) : _left(left), _right(right)
{
    VAssert(left <= right);
    VAssert(top <= bottom);

    int nthreads = 1;
// omp_set_num_threads(1);
#pragma omp parallel
    {
        nthreads = omp_get_num_threads();
    }

    for (int i = 0; i < nthreads; i++) {
        T bin_width = (right - left) / ((T)nthreads);
        T l = left + (i * bin_width);
        T r = l + bin_width;
        _qtrs.push_back(new QuadTreeRectangle<T, S>(l, top, r, bottom, max_depth, reserve_size));
    }
}

template<typename T, typename S> QuadTreeRectangleP<T, S>::QuadTreeRectangleP(size_t max_depth, size_t reserve_size) : _left(0.0), _right(1.0)
{
    int nthreads = 1;
#pragma omp parallel
    {
        nthreads = omp_get_num_threads();
    }

    for (int i = 0; i < nthreads; i++) {
        T bin_width = (1.0 - 0.0) / ((T)nthreads);
        T l = 0.0 + (i * bin_width);
        T r = l + bin_width;
        _qtrs.push_back(new QuadTreeRectangle<T, S>(l, 0.0, r, 1.0, max_depth, reserve_size));
    }
}

template<typename T, typename S> QuadTreeRectangleP<T, S>::QuadTreeRectangleP(const QuadTreeRectangleP &rhs)
{
    _left = rhs._left;
    _right = rhs._right;

    _qtrs.resize(rhs._qtrs.size());
    for (int i = 0; i < _qtrs.size(); i++) { _qtrs[i] = new QuadTreeRectangle<T, S>(*(rhs._qtrs[i])); }
}

template<typename T, typename S> QuadTreeRectangleP<T, S> &QuadTreeRectangleP<T, S>::operator=(const QuadTreeRectangleP<T, S> &rhs)
{
    //	if (*this == rhs) return *this;

    _left = rhs._left;
    _right = rhs._right;
    for (int i = 0; i < _qtrs.size(); i++) {
        if (_qtrs[i]) delete _qtrs[i];
    }

    _qtrs.resize(rhs._qtrs.size());
    for (size_t i = 0; i < rhs._qtrs.size(); i++) { _qtrs[i] = new QuadTreeRectangle<T, S>(*(rhs._qtrs[i])); }
    return *this;
}

template<typename T, typename S> QuadTreeRectangleP<T, S>::~QuadTreeRectangleP()
{
    for (size_t i = 0; i < _qtrs.size(); i++) {
        if (_qtrs[i]) delete _qtrs[i];
    }
    _qtrs.clear();
}

template<typename T, typename S> bool QuadTreeRectangleP<T, S>::Insert(T left, T top, T right, T bottom, S payload)
{
    bool status = false;
    for (int i = 0; i < _qtrs.size(); i++) {
        T bin_width = (_right - _left) / ((T)_qtrs.size());
        T l = _left + (i * bin_width);
        T r = l + bin_width;

        if (left <= r && r > left) { status |= _qtrs[i]->Insert(left, top, right, bottom, payload); }
    }
    return (status);
}

template<typename T, typename S> bool QuadTreeRectangleP<T, S>::Insert(std::vector<class QuadTreeRectangle<T, S>::rectangle_t> rectangles, std::vector<S> payloads)
{
    VAssert(rectangles.size() == payloads.size());
}

template<typename T, typename S> void QuadTreeRectangleP<T, S>::GetPayloadContained(T x, T y, std::vector<S> &payloads) const
{
    payloads.clear();

    T   bin_width = (_right - _left) / ((T)_qtrs.size());
    int bin = (x - _left) / bin_width;

    if (bin < 0 || bin >= _qtrs.size()) return;

    _qtrs[bin]->GetPayloadContained(x, y, payloads);
}

template<typename T, typename S> void QuadTreeRectangleP<T, S>::GetStats(std::vector<size_t> &payload_histo, std::vector<size_t> &level_histo) const
{
    payload_histo.clear();
    level_histo.clear();
}

template class VAPoR::QuadTreeRectangleP<float, size_t>;
