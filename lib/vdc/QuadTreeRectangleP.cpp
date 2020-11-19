#include <iostream>
#include <omp.h>
#include <vapor/VAssert.h>
#include <vapor/QuadTreeRectangleP.h>

using namespace VAPoR;
using namespace std;

template<typename T, typename S> QuadTreeRectangleP<T, S>::QuadTreeRectangleP(T left, T top, T right, T bottom, size_t max_depth, size_t reserve_size) : _left(left), _right(right)
{
    VAssert(left <= right);
    VAssert(top <= bottom);

    int nthreads = 1;
    omp_set_num_threads(1);
#pragma omp parallel
    {
        nthreads = omp_get_num_threads();
    }
    cout << "NTHREADS = " << nthreads << endl;

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
    bool status = true;
    for (int i = 0; i < _qtrs.size(); i++) {
        T bin_width = (_right - _left) / ((T)_qtrs.size());
        T l = _left + (i * bin_width);
        T r = l + bin_width;

        if (left <= r && right > l) { status &= _qtrs[i]->Insert(left, top, right, bottom, payload); }
    }
    return (status);
}

template<typename T, typename S> bool QuadTreeRectangleP<T, S>::Insert(std::vector<class QuadTreeRectangle<T, S>::rectangle_t> rectangles, std::vector<S> payloads)
{
    VAssert(rectangles.size() == payloads.size());

    cout << "INSERT 1\n";

    bool status = true;

    vector<vector<class QuadTreeRectangle<T, S>::rectangle_t>> parRectangles(_qtrs.size());
    vector<vector<S>>                                          parPayloads(_qtrs.size());
    for (int i = 0; i < _qtrs.size(); i++) {
        parRectangles[i].reserve(rectangles.size() / _qtrs.size());
        parPayloads[i].reserve(payloads.size() / _qtrs.size());
    }

    cout << "INSERT 2\n";
    T bin_width = (_right - _left) / ((T)_qtrs.size());
    for (size_t j = 0; j < rectangles.size(); j++) {
        for (int i = 0; i < _qtrs.size(); i++) {
            T l = _left + (i * bin_width);
            T r = l + bin_width;

            if ((rectangles[j]._left <= r) && (rectangles[j]._right > l)) {
                parRectangles[i].push_back(rectangles[j]);
                parPayloads[i].push_back(payloads[j]);
            }
        }
    }

    cout << "INSERT 3\n";
#pragma omp parallel
#pragma omp for
    for (int i = 0; i < _qtrs.size(); i++) {
        cout << "thread num " << omp_get_thread_num() << endl;
        for (size_t j = 0; j < parRectangles[i].size(); j++) { status &= _qtrs[i]->Insert(parRectangles[i][j], parPayloads[i][j]); }
    }
    cout << "INSERT 4\n";

    return (status);
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
