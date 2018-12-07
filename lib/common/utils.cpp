#include <cassert>
#include <iostream>
#include <algorithm>
#include <vapor/utils.h>

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
size_t Wasp::LinearizeCoords(const vector<size_t> &coords, const vector<size_t> &min, const vector<size_t> &max)
{
    size_t offset = 0;

    assert(min.size() == max.size());
    assert(min.size() == max.size());

    for (int i = 0; i < coords.size(); i++) {
        assert(coords[i] >= min[i]);
        assert(coords[i] <= max[i]);
    }

    size_t factor = 1;
    for (int i = 0; i < coords.size(); i++) {
        offset += factor * (coords[i] - min[i]);
        factor *= max[i] - min[i] + 1;
    }
    return (offset);
}

size_t Wasp::LinearizeCoords(const vector<size_t> &coords, const vector<size_t> &dims)
{
    assert(coords.size() == dims.size());

    vector<size_t> min, max;
    for (int i = 0; i < dims.size(); i++) {
        min.push_back(0);
        max.push_back(dims[i] - 1);
    }

    return (Wasp::LinearizeCoords(coords, min, max));
}

vector<size_t> Wasp::VectorizeCoords(size_t offset, const vector<size_t> &min, const vector<size_t> &max)
{
    assert(min.size() == max.size());

    vector<size_t> coords;

    coords.resize(min.size());

    size_t factor = 1;
    for (int i = 0; i < coords.size(); i++) {
        assert(min[i] <= max[i]);
        coords[i] = (offset % (factor * (max[i] - min[i] + 1))) / factor;
        offset = offset - coords[i] * factor;
        factor *= (max[i] - min[i] + 1);
    }
    assert(offset == 0);

    return (coords);
}

vector<size_t> Wasp::VectorizeCoords(size_t offset, const vector<size_t> &dims)
{
    vector<size_t> min, max;
    for (int i = 0; i < dims.size(); i++) {
        min.push_back(0);
        max.push_back(dims[i] - 1);
    }

    return (Wasp::VectorizeCoords(offset, min, max));
}

vector<size_t> Wasp::IncrementCoords(const vector<size_t> &min, const vector<size_t> &max, vector<size_t> counter, int dim)
{
    assert(min.size() == max.size());
    assert(min.size() == counter.size());

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
    assert(min.size() == max.size());
    vector<size_t> dims;

    for (int i = 0; i < min.size(); i++) {
        assert(min[i] <= max[i]);
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

void Wasp::Transpose(const float *a, float *b, int p1, int m1, int s1, int p2, int m2, int s2)
{
    int       I1, I2;
    int       i1, i2;
    int       q, r;
    const int block = BLOCKSIZE;
    for (I2 = p2; I2 < p2 + m2; I2 += block)
        for (I1 = p1; I1 < p1 + m1; I1 += block)
            for (i2 = I2; i2 < min(I2 + block, p2 + m2); i2++)
                for (i1 = I1; i1 < min(I1 + block, p1 + m1); i1++) {
                    q = i2 * s1 + i1;
                    r = i1 * s2 + i2;
                    b[r] = a[q];
                }
}

void Wasp::Transpose(const float *a, float *b, int s1, int s2) { Wasp::Transpose(a, b, 0, s1, s1, 0, s2, s2); }

int Wasp::BinarySearchRange(const vector<double> &sorted, double x, size_t &i)
{
    i = 0;

    // See if above or below the array
    //
    if (x < sorted[0]) return (-1);
    if (x > sorted[sorted.size() - 1]) return (1);

    // Binary search for starting index of cell containing x
    //
    size_t i0 = 0;
    size_t i1 = sorted.size() - 1;
    double x0 = sorted[i0];
    double x1 = sorted[i1];
    while (i1 - i0 > 1) {
        x1 = sorted[(i0 + i1) >> 1];
        if (x1 == x) {    // pathological case
            i0 = (i0 + i1) >> 1;
            break;
        }

        // if the signs of differences change then the coordinate
        // is between x0 and x1
        //
        if ((x - x0) * (x - x1) <= 0.0) {
            i1 = (i0 + i1) >> 1;
        } else {
            i0 = (i0 + i1) >> 1;
            x0 = x1;
        }
    }
    i = i0;
    return (0);
}
