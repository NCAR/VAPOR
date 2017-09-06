
#include <cstring>
#include <vector>
#include <vapor/common.h>
#ifndef _VAPOR_UTILS_H_
#define _VAPOR_UTILS_H_

namespace Wasp {

class COMMON_API SmartBuf {
  public:
    SmartBuf() {
        _buf = NULL;
        _buf_sz = 0;
    };

    SmartBuf(size_t size) {
        _buf = new unsigned char[size];
        _buf_sz = size;
    };

    SmartBuf(const SmartBuf &rhs) {
        _buf = new unsigned char[rhs._buf_sz];
        _buf_sz = rhs._buf_sz;
        memcpy(_buf, rhs._buf, rhs._buf_sz);
    }

    SmartBuf &operator=(const SmartBuf &rhs) {
        _buf = new unsigned char[rhs._buf_sz];
        _buf_sz = rhs._buf_sz;
        memcpy(_buf, rhs._buf, rhs._buf_sz);
        return *this;
    }

    ~SmartBuf() {
        if (_buf)
            delete[] _buf;
    };
    void *Alloc(size_t size);
    void *GetBuf() const { return (_buf); }
    size_t GetBufSize() const { return (_buf_sz); }

  private:
    unsigned char *_buf;
    size_t _buf_sz;
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
COMMON_API size_t LinearizeCoords(
    const std::vector<size_t> &coords, const std::vector<size_t> &dims);

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
COMMON_API size_t LinearizeCoords(
    const std::vector<size_t> &coords,
    const std::vector<size_t> &min,
    const std::vector<size_t> &max);

//! Vectorize a coordinate offset. Inverse of VectorizeLinearize
//!
COMMON_API std::vector<size_t> VectorizeCoords(
    size_t offset,
    const std::vector<size_t> &min, const std::vector<size_t> &max);

//! Vectorize a coordinate offset. Inverse of VectorizeLinearize
//!
COMMON_API std::vector<size_t> VectorizeCoords(
    size_t offset, const std::vector<size_t> &dims);

}; // namespace Wasp

#endif
