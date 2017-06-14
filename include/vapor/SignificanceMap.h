//
// $Id: SignificanceMap.h,v 1.5 2012/02/28 18:15:58 ypolius Exp $
//

#ifndef _SignificanceMap_h_
#define _SignificanceMap_h_

#include <vector>
#include <cassert>
#include <vapor/MyBase.h>
#include <algorithm>

// using namespace std;

//#include "MyBase.h"

#ifndef BITSPERBYTE
    #define BITSPERBYTE 8
#endif

namespace VAPoR {

//
//! \class SignificanceMap
//! \brief Implements a significance map
//! \author John Clyne
//! \version $Revision: 1.5 $
//! \date    $Date: 2012/02/28 18:15:58 $
//!
//! This class implements a quick and dirty significance map - a mapping
//! indicating which entries in an array are valid and which are not.
//!

class WASP_API SignificanceMap : public Wasp::MyBase {
public:
    SignificanceMap();

    //!
    //! Significance map constructors for 1D, 2D, 3D, and 4D maps
    //!
    //! \param[in] nx Dimension of X axis (fastest varying)
    //! \param[in] ny Dimension of Y axis (second fastest varying)
    //! \param[in] nz Dimension of Z axis (third fastest varying)
    //! \param[in] nt Dimension of Z axis (fourth fastest varying)
    //! \param[in] dims A vector specifying the number of dimensions and their
    //! lengths.
    //!
    SignificanceMap(size_t nx, size_t ny = 1, size_t nz = 1, size_t nt = 1);
    SignificanceMap(std::vector<size_t> dims);

    //
    //! Significance map constructors for 1D, 2D, 3D, and 4D maps, using
    //! a previously created map. I.e. a map returned from GetMap()
    //!
    //! \param[in] map An encoded significance map returned by GetMap()
    //! \param[in] nx Dimension of X axis (fastest varying)
    //! \param[in] ny Dimension of Y axis (second fastest varying)
    //! \param[in] nz Dimension of Z axis (third fastest varying)
    //! \param[in] nt Dimension of Z axis (fourth fastest varying)
    //! \param[in] dims A vector specifying the number of dimensions and their
    //! lengths.
    //
    SignificanceMap(const unsigned char *map, size_t nx, size_t ny = 1, size_t nz = 1, size_t nt = 1);
    SignificanceMap(const unsigned char *map, std::vector<size_t> dims);

    //! Significance map copy constructor
    //!
    SignificanceMap(const SignificanceMap &);

    ~SignificanceMap();

    //! Set the shape (dimension) of a significance map
    //!
    //! This method changes the shape (dimension) of a significance map
    //!
    //! \param[in] nx Dimension of X axis (fastest varying)
    //! \param[in] ny Dimension of Y axis (second fastest varying)
    //! \param[in] nz Dimension of Z axis (third fastest varying)
    //! \param[in] nt Dimension of Z axis (fourth fastest varying)
    //! \param[in] dims A vector specifying the number of dimensions and their
    //! lengths.
    int Reshape(std::vector<size_t> dims);
    int Reshape(size_t nx, size_t ny = 1, size_t nz = 1, size_t nt = 1);

    //! Return the shape of a significance map
    //!
    void GetShape(std::vector<size_t> &dims) const { dims = _dimsVec; };

    //! Mark a map entry as significant.
    //!
    //! This method marks a particular coordinate as significant.
    //! The coordinate parameters, \p x, \p y, \p z, and \p t, must lie within the
    //! range [0..n-1], where n-1 is the value passed as the coordinates
    //! dimension to the constructor. No attempt is to prevent duplicate
    //! entries. If a coordinate is not specified its default value is zero.
    //!
    //! \param[in] x X coordinate
    //! \param[in] y Y coordinate
    //! \param[in] z Z coordinate
    //! \param[in] t T coordinate
    //! \param[in] idx coordinate specified as an offset from a linear array
    //!
    //! \retval status a negative value is returned on failure
    //!
    int Set(size_t idx);
    int SetXYZT(size_t x, size_t y = 0, size_t z = 0, size_t t = 0);

    //! Return true if the indicated entry is significant. Test() returns
    //! false if the entry is out of range or in range, but not set
    //!
    //! \param[in] x X coordinate
    //! \param[in] y Y coordinate
    //! \param[in] z Z coordinate
    //! \param[in] t T coordinate
    //! \param[in] idx coordinate specified as an offset from a linear array
    //!
    bool inline Test(size_t idx) const;
    bool inline TestXYZT(size_t x, size_t y = 0, size_t z = 0, size_t t = 0) const;

    //! Mark the indicated entry as insignificant
    //!
    //! This method clears the indicated entry (marks it as insignificant)
    //!
    //! \retval status a negative value is returned on failure
    //!
    int Clear(size_t idx);
    int ClearXYZT(size_t x, size_t y = 0, size_t z = 0, size_t t = 0);

    //! Clear entire map
    //!
    void Clear();

    //! Return the number of significant entries in the map
    //!
    size_t GetNumSignificant() const { return (_sigMapVec.size()); };

    //! Return coordinates for an ordered entry in the map
    //!
    //! Return the coordinates of the ith significant entry in the
    //! significant map. Note, the mapping between an entry number and
    //! it's coordinates returned by this function are no longer valid after
    //! the map is modified (set, cleared, or sorted). If the coordinate pointers,
    //! x,y,z,t are null, no value is returned for that coordinate.
    //!
    //! Valid values for 'i' are in the range [0..GetNumSignificant()-1]
    //! \retval status a negative value is returned on failure
    //!
    //
    int GetCoordinatesXYZT(size_t i, size_t *x, size_t *y = NULL, size_t *z = NULL, size_t *t = NULL) const;
    int GetCoordinates(size_t i, size_t *idx) const;

    //! Restart counters for GetNextEntry()
    //!
    //! This method is used to restore internal counters used by the
    //! GetNextEntry() method
    //!
    //! \sa GetMextEntry()
    //
    void GetNextEntryRestart();

    //
    //! Return the coordinates for the next signficant entry in the
    //! significance map. This method may be called iteratively until
    //! all map entries are turned. The GetNextEntryRestart() method may be used
    //! to reset the current entry to the first one in the map. A zero
    //! value is returned if the entry list is exhausted. The coordinates
    //! are returned in the order that they were set with Set() or SetXYZT().
    //!
    //! \note For sequential processing of map entries it is preferable to
    //! use this method over the indexed GetEntry() method.
    //!
    //! \retval status a negative value is returned on failure
    //!
    //
    int GetNextEntry(size_t *idx);
    int GetNextEntryXYZT(size_t *x, size_t *y, size_t *z, size_t *t);

    //! Return size in bytes of an encoded signficance map of given size
    //!
    //! This static member method returns the size in bytes of an encoded
    //! signficance map that would be returned by GetMap() for a
    //! SignificanceMap of given dimension, \p dims, and number of
    //! entries, \p num_entries.
    //
    static size_t GetMapSize(vector<size_t> dims, size_t num_entries);

    //! Return size in bytes of an encoded signficance map of given size
    //!
    //! This method returns the size in bytes of an encoded
    //! signficance map that would be returned by GetMap() after
    //! the given number of entries, \p num_entries, were stored.
    //! This method can be used to determine the amount of space
    //! required to store an encoded signficance map after
    //! \p num_entries entries have been stored in it. In general,
    //! this is not the same value for the current significance map.
    //!
    //! \param num_entries[in] Number of entries in the signficance map
    //!
    //! \sa GetMap()
    //
    size_t GetMapSize(size_t num_entries) const;

    //! Return size in bytes of current encoded signficance map
    //!
    //! This method returns the size in bytes of the encoded,
    //! current signficance map that would be returned by GetMap().
    //!
    //! \sa GetMap()
    //
    size_t GetMapSize() const { return (GetMapSize(GetNumSignificant())); };

    //! Return the compressed representation of the significance map. The data
    //! returned are only suitable passing as an argument to the constructor.
    //!
    //! \param map[out] Encoded significance map data
    //! \param maplen[out] Length of \p map in bytes
    //
    void GetMap(const unsigned char **map, size_t *maplen);

    //! Return the compressed representation of the significance map. The data
    //! returned are only suitable passing as an argument to the constructor.
    //!
    //! \param map[out] Encoded significance map data. Caller is
    //! responsible for allocating memory. The array \p map must be of
    //! size GetMapSize().
    //
    void GetMap(unsigned char *map);

    //
    //! Reinitialize the significance map with the map, \p map , returned from a
    //! previous call to GetMap.
    //!
    //! \param[in] map An encoded significance map returned by GetMap()
    //!
    //! \sa GetMap()
    //!
    //! \retval status a negative value is returned on failure
    //!
    //
    int SetMap(const unsigned char *map);

    //! Append a SignificanceMap
    //!
    //! This method appends the coordinates in the map, \p smap, to
    //! this map, creating a new signficance map containing the entries
    //! from both this signficance map and \p smap. The ordering of the new map
    //! is as if the GetNextEntry() method of \p smap was called
    //! iteratively, storing the the returned coordinate with Set().
    //!
    //! The dimensions of \p smap must match those of this map or error
    //! condition is returned.
    //!
    //! \param[in] smap An encoded significance map returned by GetMap()
    //!
    //! \retval status a negative value is returned on failure
    //!
    //
    int Append(const SignificanceMap &smap);

    //! Invert the signficance map
    //!
    //! This method inverts the signficance map, making insignficant
    //! coordinates (those not found in the significance map) into significant
    //! coordinates. The ordering of the newly created map will be from
    //! smallest to largest coordinate.
    //!
    //! \sa GetNextEntry()
    //
    void Invert();

    //! Sort map entries
    //!
    //! This method sorts the entries of the significance maps so that
    //! they are stored from smallest to largest. Once sorted, map entries
    //! returned by GetNextEntry() are guaranteed to be returned from
    //! smallest to largest
    //!
    //! \sa GetNextEntry()
    //
    void Sort();

    SignificanceMap &operator=(const SignificanceMap &map);

    friend std::ostream &operator<<(std::ostream &o, const SignificanceMap &sigmap);

private:
    static const int HEADER_SIZE = 64;
    static const int VDF_VERSION = 2;
    size_t           _nx;
    size_t           _ny;
    size_t           _nz;
    size_t           _nt;    // dimensions of map (when # dims < 5)

    bool _sorted;    // true if the map is sorted in ascending order

    std::vector<size_t> _dimsVec;       // Map dimensions
    size_t              _sigMapSize;    // product of dimensions (max coordinate index)
    std::vector<size_t> _sigMapVec;     // The signficance map

    int            _bits_per_idx;        // # bits needed to encode a coordinate
    unsigned char *_sigMapEncode;        // compactly encoded version of _sigMapVec
    size_t         _sigMapEncodeSize;    // size of _sigMapEncode in bytes

    size_t _idxentry;    // Counter for sequential access to sig. map

    int _SignificanceMap(std::vector<size_t> dims);
    int _SignificanceMap(const unsigned char *map, std::vector<size_t> dims);

    static size_t _GetBitsPerIdx(vector<size_t> dims);
};

bool inline SignificanceMap::Test(size_t idx) const
{
    if (idx > _sigMapSize) return (0);    // Invalid coordinate

    if (_sorted) { return (binary_search(_sigMapVec.begin(), _sigMapVec.end(), idx)); }

    for (size_t i = 0; i < _sigMapVec.size(); i++) {
        if (_sigMapVec[i] == idx) return (1);
    }
    return (0);
}

bool inline SignificanceMap::TestXYZT(size_t x, size_t y, size_t z, size_t t) const
{
    size_t idx = (t * _nz * _ny * _nx) + (z * _ny * _nx) + (y * _nx) + x;

    return (this->Test(idx));
}

}    // namespace VAPoR

#endif
