
#include <cstring>
#include <vector>
#include <vapor/common.h>

#pragma once

namespace Wasp {

class COMMON_API SmartBuf {
public:
    SmartBuf()
    {
        _buf = NULL;
        _buf_sz = 0;
    };

    SmartBuf(size_t size)
    {
        _buf = new unsigned char[size];
        _buf_sz = size;
    };

    SmartBuf(const SmartBuf &rhs)
    {
        _buf = new unsigned char[rhs._buf_sz];
        _buf_sz = rhs._buf_sz;
        memcpy(_buf, rhs._buf, rhs._buf_sz);
    }

    SmartBuf &operator=(const SmartBuf &rhs)
    {
        _buf = new unsigned char[rhs._buf_sz];
        _buf_sz = rhs._buf_sz;
        memcpy(_buf, rhs._buf, rhs._buf_sz);
        return *this;
    }

    ~SmartBuf()
    {
        if (_buf) delete[] _buf;
    };
    void * Alloc(size_t size);
    void * GetBuf() const { return (_buf); }
    size_t GetBufSize() const { return (_buf_sz); }

private:
    unsigned char *_buf;
    size_t         _buf_sz;
};

//! Linearize multi-dimensional coordinates
//!
//! Convert multi-dimensional coordinates, \p coords, for a space
//! with dimensions, \p dims, to a linear offset from the origin
//! of dims
//!
//! \param[in] coords A vector of integer coordinates into a an
//! array with dimensions given by \p dims. The minimum coordinate value is
//! zero. The maximum coordinate value is \p dims[i] - 1.
//! \param[in] dims A vector defining the dimensions of an array. The size
//! of \p dims must equal size of \p coords.
//!
//! \retval offset The offset from the first element of the array to the
//! address specified by \p coords
//
COMMON_API size_t LinearizeCoords(const size_t *coords, const size_t *dims, int n);
COMMON_API size_t LinearizeCoords(const std::vector<size_t> &coords, const std::vector<size_t> &dims);

//! Linearize multi-dimensional coordinates
//!
//! Convert multi-dimensional coordinates, \p coords, for a space
//! with minimum and maximum coordinates given by \p min, and \p max,
//! respectively.
//!
//! \param[in] coords A vector of integer coordinates into a an
//! array with boundaries defined by \p min and \p max. The minimum
//! coordinate value is \p min[i] and the maximum is \p max[i].
//! \param[in] min Minimum valid coordinate value
//! \param[in] min Maximum valid coordinate value
//!
//! \retval offset The offset from the first element of the array to the
//! address specified by \p coords
COMMON_API size_t LinearizeCoords(const size_t *coords, const size_t *min, const size_t *max, int n);
COMMON_API size_t LinearizeCoords(const std::vector<size_t> &coords, const std::vector<size_t> &min, const std::vector<size_t> &max);

//! Increment a coordinate vector by one
//!
//! Increments \p counter along the dimension \p dim by one within the
//! range of \p min
//! to \p max. Overflow is possible, resulting in wraparound and setting
//! \p counter back to \p min
//
COMMON_API std::vector<size_t> IncrementCoords(const std::vector<size_t> &min, const std::vector<size_t> &max, std::vector<size_t> counter, int dim = 0);

//! Return the dimesions of a subregion
//!
//! return the dimensions of a subregion enclosed by \p min and \p max
//
COMMON_API std::vector<size_t> Dims(const std::vector<size_t> &min, const std::vector<size_t> &max);

//! Return the scalar product of the elements of a vector
//!
COMMON_API size_t VProduct(const std::vector<size_t> &a);

//! Vectorize a coordinate offset. Inverse of VectorizeLinearize
//!
COMMON_API void VectorizeCoords(size_t offset, const size_t *min, const size_t *max, size_t *coords, int n);
COMMON_API std::vector<size_t> VectorizeCoords(size_t offset, const std::vector<size_t> &min, const std::vector<size_t> &max);

//! Vectorize a coordinate offset. Inverse of VectorizeLinearize
//!
COMMON_API void VectorizeCoords(size_t offset, const size_t *dims, size_t *coords, int n);
COMMON_API std::vector<size_t> VectorizeCoords(size_t offset, const std::vector<size_t> &dims);

//
// blocked submatrix Transpose suitable for multithreading
//   *a : pointer to input matrix
//   *b : pointer to output matrix
//    p1,p2: starting index of submatrix (row,col)
//    m1,m2: size of submatrix (row,col)
//    s1,s2: size of entire matrix (row,col)
//
COMMON_API void Transpose(const float *a, float *b, int p1, int m1, int s1, int p2, int m2, int s2);

//
// blocked matrix Transpose single threaded
//   *a : pointer to input matrix
//   *b : pointer to output matrix
//    s1,s2: size of entire matrix (row,col)
//
COMMON_API void Transpose(const float *a, float *b, int s1, int s2);

// Perform a binary search in a sorted 1D vector of values for the
// entry that it closest to 'x'. Return the offset 'i' of 'x' in
// 'sorted'
//
COMMON_API int BinarySearchRange(const std::vector<double> &sorted, double x, size_t &i);

};    // namespace Wasp
