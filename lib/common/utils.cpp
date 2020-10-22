#include "vapor/VAssert.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include <vapor/utils.h>

#define MAXCOORDS 4

using namespace std;
using namespace Wasp;

void *SmartBuf::Alloc(size_t size)
{
    if (size <= _buf_sz) return (_buf);

    if (_buf) delete[] _buf;

    _buf = new unsigned char[size];
    _buf_sz = size;
    return (_buf);
}

// convert tuple of multi-dimensional coordinates, 'coord', for a space
// with min and max bounds to a linear offset from 'min'
//
size_t Wasp::LinearizeCoords(const size_t *coords, const size_t *min, const size_t *max, int n)
{
    size_t offset = 0;

    for (int i = 0; i < n; i++) {
        VAssert(coords[i] >= min[i]);
        VAssert(coords[i] <= max[i]);
    }

    size_t factor = 1;
    for (int i = 0; i < n; i++) {
        offset += factor * (coords[i] - min[i]);
        factor *= max[i] - min[i] + 1;
    }
    return (offset);
}

size_t Wasp::LinearizeCoords(const std::vector<size_t> &coords, const std::vector<size_t> &dims)
{
    VAssert(coords.size() == dims.size());
    return (LinearizeCoords(coords.data(), dims.data(), coords.size()));
}

size_t Wasp::LinearizeCoords(const size_t *coords, const size_t *dims, int n)
{
    VAssert(n <= MAXCOORDS);
    size_t min[MAXCOORDS];
    size_t max[MAXCOORDS];

    for (int i = 0; i < n; i++) {
        min[i] = 0;
        max[i] = dims[i] - 1;
    }

    size_t returnVal = Wasp::LinearizeCoords(coords, min, max, n);

    return (returnVal);
}

size_t Wasp::LinearizeCoords(const std::vector<size_t> &coords, const std::vector<size_t> &min, const std::vector<size_t> &max)
{
    VAssert(coords.size() == min.size());
    VAssert(coords.size() == max.size());
    return (LinearizeCoords(coords.data(), min.data(), max.data(), coords.size()));
}

void Wasp::VectorizeCoords(size_t offset, const size_t *min, const size_t *max, size_t *coords, int n)
{
    size_t factor = 1;
    for (int i = 0; i < n; i++) {
        VAssert(min[i] <= max[i]);
        coords[i] = (offset % (factor * (max[i] - min[i] + 1))) / factor;
        offset = offset - coords[i] * factor;
        factor *= (max[i] - min[i] + 1);
    }
    VAssert(offset == 0);
}

std::vector<size_t> Wasp::VectorizeCoords(size_t offset, const std::vector<size_t> &min, const std::vector<size_t> &max)
{
    VAssert(min.size() == max.size());

    size_t coords[MAXCOORDS];
    VectorizeCoords(offset, min.data(), max.data(), coords, min.size());
    std::vector<size_t> coordsvec(min.size(), 0);
    for (int i = 0; i < min.size(); i++) coordsvec[i] = coords[i];
    return (coordsvec);
}

void Wasp::VectorizeCoords(size_t offset, const size_t *dims, size_t *coords, int n)
{
    VAssert(n <= MAXCOORDS);
    size_t min[MAXCOORDS];
    size_t max[MAXCOORDS];
    for (int i = 0; i < n; i++) {
        min[i] = 0;
        max[i] = dims[i] - 1;
    }

    Wasp::VectorizeCoords(offset, min, max, coords, n);
}

std::vector<size_t> Wasp::VectorizeCoords(size_t offset, const std::vector<size_t> &dims)
{
    size_t coords[MAXCOORDS];
    VectorizeCoords(offset, dims.data(), coords, dims.size());
    std::vector<size_t> coordsvec(dims.size(), 0);
    for (int i = 0; i < dims.size(); i++) coordsvec[i] = coords[i];

    return (coordsvec);
}

vector<size_t> Wasp::IncrementCoords(const vector<size_t> &min, const vector<size_t> &max, vector<size_t> counter, int dim)
{
    VAssert(min.size() == max.size());
    VAssert(min.size() == counter.size());

    for (int i = dim; i < counter.size(); i++) {
        if (counter[i] < (max[i])) {
            counter[i] += 1;
            break;
        }
        counter[i] = min[i];
    }
    return (counter);
}

vector<size_t> Wasp::Dims(const vector<size_t> &min, const vector<size_t> &max)
{
    VAssert(min.size() == max.size());
    vector<size_t> dims;

    for (int i = 0; i < min.size(); i++) {
        VAssert(min[i] <= max[i]);
        dims.push_back(max[i] - min[i] + 1);
    }
    return (dims);
}

size_t Wasp::VProduct(const vector<size_t> &a)
{
    size_t ntotal = 1;

    for (int i = 0; i < a.size(); i++) ntotal *= a[i];

    return (ntotal);
}

#define BLOCKSIZE 256

void Wasp::Transpose(const float *a, float *b, size_t p1, size_t m1, size_t s1, size_t p2, size_t m2, size_t s2)
{
    size_t       I1, I2;
    size_t       i1, i2;
    size_t       q, r;
    const size_t block = BLOCKSIZE;
    for (I2 = p2; I2 < p2 + m2; I2 += block)
        for (I1 = p1; I1 < p1 + m1; I1 += block)
            for (i2 = I2; i2 < min(I2 + block, p2 + m2); i2++)
                for (i1 = I1; i1 < min(I1 + block, p1 + m1); i1++) {
                    q = i2 * s1 + i1;
                    r = i1 * s2 + i2;
                    b[r] = a[q];
                }
}

void Wasp::Transpose(const float *a, float *b, size_t s1, size_t s2) { Wasp::Transpose(a, b, 0, s1, s1, 0, s2, s2); }

bool Wasp::BinarySearchRange(const vector<double> &sorted, double x, size_t &i)
{
    i = 0;

    if (sorted.size() == 1) return (sorted[0] == x);

    // if sorted in ascending order
    //
    if (sorted[0] <= sorted[sorted.size() - 1]) {
        if (x < sorted[0]) return (false);

        if (x == sorted[sorted.size() - 1]) {
            i = sorted.size() - 2;
            return (true);
        }

        vector<double>::const_iterator itr;
        itr = std::upper_bound(sorted.begin(), sorted.end(), x);
        if (itr == sorted.end()) { return (false); }
        if (itr != sorted.begin()) { --itr; }
        i = itr - sorted.begin();

    } else {
        if (x < sorted[sorted.size() - 1]) return (false);

        if (x == sorted[sorted.size() - 1]) {
            i = sorted.size() - 2;
            return (true);
        }

        vector<double>::const_reverse_iterator itr;
        itr = std::lower_bound(sorted.rbegin(), sorted.rend(), x);
        if (itr == sorted.rend()) { return (false); }
        if (itr != sorted.rbegin()) { ++itr; }
        i = sorted.rend() - itr;
    }

    return (true);
}

bool Wasp::NearlyEqual(float a, float b, float epsilon)
{
    float absA = std::fabs(a);
    float absB = std::fabs(b);
    float diff = std::fabs(a - b);

    // shortcut, handles infinities
    //
    if (a == b) return (true);

    if (a == 0.0 || b == 0.0 || (absA + absB < std::numeric_limits<float>::min())) {
        // a or b is zero or both are extremely close to it
        // relative error is less meaningful here
        //
        return (diff < (epsilon * std::numeric_limits<float>::min()));
    }

    // use relative error
    return (diff / min((absA + absB), std::numeric_limits<float>::max()) < epsilon);
}
