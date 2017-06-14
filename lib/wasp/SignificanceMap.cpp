//
// $Id: SignificanceMap.cpp,v 1.12 2012/08/29 19:36:37 alannorton Exp $
//
#include <iostream>
#include <cstring>
#include <vapor/SignificanceMap.h>

using namespace VAPoR;

// gets the right-adjusted N bits of quantity TARG
// starting from bit position POSS
//
#define GETBITS(TARG, POSS, N) (((TARG) >> ((POSS) + 1 - (N))) & ~(~0ULL << (N)))

// set N bits of quantity TARG starting from position
// POSS to the right-most N bits in integer SRC
//
#define PUTBITS(TARG, POSS, N, SRC)                         \
    (TARG) &= ~(~((~0ULL) << (N)) << (((POSS) + 1) - (N))); \
    (TARG) |= (((SRC) & ~((~0ULL) << (N))) << (((POSS) + 1) - (N)))

using namespace std;

template<class T> void swapbytes(T *ptr, size_t nelem)
{
    for (size_t i = 0; i < nelem; i++) {
        unsigned char *uptr = (unsigned char *)&ptr[i];

        unsigned char *p1 = uptr;
        unsigned char *p2 = uptr + sizeof(T) - 1;
        unsigned char  t;
        for (int j = 0; j < (sizeof(T) >> 1); j++) {
            t = *p1;
            *p1 = *p2;
            *p2 = t;
            p1++;
            p2--;
        }
    }
}

size_t SignificanceMap::_GetBitsPerIdx(vector<size_t> dims)
{
    size_t size = 1;

    for (int i = 0; i < dims.size(); i++) { size *= dims[i]; }

    // Compute # bits needed per entry for encoded form of sigMapVec
    //
    size_t bits_per_idx = 1;
    size--;
    while ((size = (size >> 1))) bits_per_idx++;

    return (bits_per_idx);
}

int SignificanceMap::_SignificanceMap(vector<size_t> dims)
{
    _dimsVec.clear();
    _sigMapVec.clear();
    _sigMapSize = 1;
    _sorted = true;
    for (int i = 0; i < dims.size(); i++) {
        if (dims[i] < 1) {
            SetErrMsg("Zero length dimensions not permitted");
            return (-1);
        }
        _sigMapSize *= dims[i];
        _dimsVec.push_back(dims[i]);
    }
    _idxentry = 0;

    _nx = _ny = _nz = _nt = 1;
    if (dims.size() >= 1) _nx = dims[0];
    if (dims.size() >= 2) _ny = dims[1];
    if (dims.size() >= 3) _nz = dims[2];
    if (dims.size() >= 4) _nt = dims[3];

    // Compute # bits needed per entry for encoded form of sigMapVec
    //
    //_bits_per_idx = 1;
    // size_t tmpsize = _sigMapSize - 1;
    // while (tmpsize = (tmpsize >> 1)) _bits_per_idx++;

    _bits_per_idx = _GetBitsPerIdx(_dimsVec);

    return (0);
}

int SignificanceMap::_SignificanceMap(const unsigned char *map, std::vector<size_t> dims)
{
    if (_SignificanceMap(dims) < 0) return (-1);

    return (SetMap(map));
}

SignificanceMap::SignificanceMap()
{
    vector<size_t> dims;
    dims.push_back(1);

    _sigMapEncode = NULL;
    _sigMapEncodeSize = 0;
    if (_SignificanceMap(dims) < 0) return;
}

SignificanceMap::SignificanceMap(size_t nx, size_t ny, size_t nz, size_t nt)
{
    vector<size_t> dims;

    dims.push_back(nx);
    dims.push_back(ny);
    dims.push_back(nz);
    dims.push_back(nt);

    _sigMapEncode = NULL;
    _sigMapEncodeSize = 0;
    if (_SignificanceMap(dims) < 0) return;
}

SignificanceMap::SignificanceMap(vector<size_t> dims)
{
    _sigMapEncode = NULL;
    _sigMapEncodeSize = 0;
    if (_SignificanceMap(dims) < 0) return;
}

//
// Significance map constructors for 1D, 2D, 3D, and 4D maps, using
// a previously created map. I.e. a map returned from GetMap()
//
SignificanceMap::SignificanceMap(const unsigned char *map, size_t nx, size_t ny, size_t nz, size_t nt)
{
    vector<size_t> dims;

    dims.push_back(nx);
    dims.push_back(ny);
    dims.push_back(nz);
    dims.push_back(nt);

    _sigMapEncode = NULL;
    _sigMapEncodeSize = 0;
    if (_SignificanceMap(map, dims) < 0) return;
}

SignificanceMap::SignificanceMap(const unsigned char *map, vector<size_t> dims)
{
    _sigMapEncode = NULL;
    _sigMapEncodeSize = 0;
    if (_SignificanceMap(map, dims) < 0) return;
}

SignificanceMap::SignificanceMap(const SignificanceMap &rhs)
{
    if (this == &rhs) return;

    this->_nx = rhs._nx;
    this->_ny = rhs._ny;
    this->_nz = rhs._nz;
    this->_nt = rhs._nt;

    this->_sorted = rhs._sorted;

    this->_dimsVec = rhs._dimsVec;
    this->_sigMapSize = rhs._sigMapSize;
    this->_sigMapVec = rhs._sigMapVec;

    this->_bits_per_idx = rhs._bits_per_idx;
    this->_sigMapEncodeSize = rhs._sigMapEncodeSize;
    this->_sigMapEncode = NULL;

    this->_idxentry = rhs._idxentry;

    // handle raw pointers
    //
    if (rhs._sigMapEncode) {
        this->_sigMapEncode = new unsigned char[this->_sigMapEncodeSize];
        memcpy(this->_sigMapEncode, rhs._sigMapEncode, this->_sigMapEncodeSize);
    }
}

SignificanceMap &SignificanceMap::operator=(const SignificanceMap &rhs)
{
    if (this == &rhs) return (*this);

    this->_nx = rhs._nx;
    this->_ny = rhs._ny;
    this->_nz = rhs._nz;
    this->_nt = rhs._nt;

    this->_sorted = rhs._sorted;

    this->_dimsVec = rhs._dimsVec;
    this->_sigMapSize = rhs._sigMapSize;
    this->_sigMapVec = rhs._sigMapVec;

    this->_bits_per_idx = rhs._bits_per_idx;
    this->_sigMapEncodeSize = rhs._sigMapEncodeSize;
    this->_sigMapEncode = NULL;

    this->_idxentry = rhs._idxentry;

    // handle raw pointers
    //
    if (rhs._sigMapEncode) {
        this->_sigMapEncode = new unsigned char[this->_sigMapEncodeSize];
        memcpy(this->_sigMapEncode, rhs._sigMapEncode, this->_sigMapEncodeSize);
    }

    return *this;
}

SignificanceMap::~SignificanceMap()
{
    if (_sigMapEncode) { delete[] _sigMapEncode; }

    _sigMapEncode = NULL;
}

int SignificanceMap::Reshape(vector<size_t> dims)
{
    _dimsVec.clear();

    if (dims.size() != _dimsVec.size()) {
        return (_SignificanceMap(dims));
    } else {
        for (int i = 0; i < dims.size(); i++) {
            if (dims[i] != _dimsVec[i]) { return (_SignificanceMap(dims)); }
        }
    }
    return (0);    // nothing to do
}

int SignificanceMap::Reshape(size_t nx, size_t ny, size_t nz, size_t nt)
{
    vector<size_t> dims;

    dims.push_back(nx);
    dims.push_back(ny);
    dims.push_back(nz);
    dims.push_back(nt);

    return Reshape(dims);
}

int SignificanceMap::Set(size_t idx)
{
    if (idx >= _sigMapSize) {
        SetErrMsg("Coordinates out of range");
        return (-1);
    }

    // if (Test(idx)) return(0);

    _sigMapVec.push_back(idx);

    //
    // See if sig map is still sorted
    //
    if (_sorted && (_sigMapVec.size() > 1)) {
        if (_sigMapVec[_sigMapVec.size() - 1] < _sigMapVec[_sigMapVec.size() - 2]) { _sorted = false; }
    }
    return (0);
}

int SignificanceMap::SetXYZT(size_t x, size_t y, size_t z, size_t t)
{
    if (_dimsVec.size() > 4) {
        SetErrMsg("Method not implemented for dimensions greater than 4");
        return (-1);
    }
    if (x >= _nx || y >= _ny || z >= _nz || t >= _nt) {
        SetErrMsg("Coordinates out of range");
        return (-1);
    }

    size_t idx = (t * _nz * _ny * _nx) + (z * _ny * _nx) + (y * _nx) + x;
    return (Set(idx));
}

int SignificanceMap::Clear(size_t idx)
{
    if (idx >= _sigMapSize) {
        SetErrMsg("Coordinates out of range");
        return (-1);
    }

    if (!Test(idx)) return (0);

    vector<size_t>::iterator itr;
    for (itr = _sigMapVec.begin(); itr != _sigMapVec.end();) {
        if (*itr == idx) {
            _sigMapVec.erase(itr);

            // We're being overly cautious here - entries should never
            // be duplicated
            //
            itr = _sigMapVec.begin();
        }
    }
    return (0);
}

int SignificanceMap::ClearXYZT(size_t x, size_t y, size_t z, size_t t)
{
    if (_dimsVec.size() > 4) {
        SetErrMsg("Method not implemented for dimensions greater than 4");
        return (-1);
    }

    if (x >= _nx || y >= _ny || z >= _nz || t >= _nt) {
        SetErrMsg("Coordinates out of range");
        return (-1);
    }

    size_t idx = (t * _nz * _ny * _nx) + (z * _ny * _nx) + (y * _nx) + x;
    return (Clear(idx));
}

void SignificanceMap::Clear() { _sigMapVec.clear(); }

int SignificanceMap::GetCoordinates(size_t offset, size_t *idx) const
{
    if (offset >= _sigMapVec.size()) {
        SetErrMsg("Index out of range");
        return (-1);
    }
    *idx = _sigMapVec[offset];
    return (0);
}

int SignificanceMap::GetCoordinatesXYZT(size_t offset, size_t *x, size_t *y, size_t *z, size_t *t) const
{
    if (_dimsVec.size() > 4) {
        SetErrMsg("Method not implemented for dimensions greater than 4");
        return (-1);
    }

    size_t idx;

    if (GetCoordinates(offset, &idx) < 0) return (-1);

    *t = idx / (_nz * _ny * _nx);
    idx -= (*t * _nz * _ny * _nx);

    *z = idx / (_ny * _nx);
    idx -= (*z * _ny * _nx);

    *y = idx / _nx;
    idx -= (*y * _nx);

    *x = idx;

    return (0);
}

void SignificanceMap::GetNextEntryRestart()
{
    if (!_sorted) SignificanceMap::Sort();
    _idxentry = 0;
}

int SignificanceMap::GetNextEntry(size_t *idx)
{
    if (_idxentry >= _sigMapVec.size()) return (0);

    *idx = _sigMapVec[_idxentry];
    _idxentry++;

    return (1);
}

int SignificanceMap::GetNextEntryXYZT(size_t *xptr, size_t *yptr, size_t *zptr, size_t *tptr)
{
    if (_dimsVec.size() > 4) {
        SetErrMsg("Method not implemented for dimensions greater than 4");
        return (-1);
    }

    size_t idx;
    if (GetNextEntry(&idx)) {
        size_t t = idx / (_nz * _ny * _nx);
        idx -= (t * _nz * _ny * _nx);

        size_t z = idx / (_ny * _nx);
        idx -= (z * _ny * _nx);

        size_t y = idx / _nx;
        idx -= (y * _nx);

        size_t x = idx;

        if (xptr) *xptr = x;
        if (yptr) *yptr = y;
        if (zptr) *zptr = z;
        if (tptr) *tptr = t;
        return (1);
    }

    return (0);
}
size_t SignificanceMap::GetMapSize(vector<size_t> dims, size_t num_entries)
{
    // Calculate size of encoded map
    //
    size_t mapsize;
    size_t tbits = num_entries * _GetBitsPerIdx(dims);
    if (tbits)
        mapsize = (tbits - 1) / BITSPERBYTE + 1 + HEADER_SIZE;
    else
        mapsize = HEADER_SIZE;

    return (mapsize);
}

size_t SignificanceMap::GetMapSize(size_t num_entries) const { return (GetMapSize(_dimsVec, num_entries)); }

void SignificanceMap::GetMap(unsigned char *encodedMap)
{
    unsigned long LSBTest = 1;
    bool          do_swapbytes = false;
    if (!(*(char *)&LSBTest)) {
        // swap to MSBFirst
        do_swapbytes = true;
    }

    memset(_sigMapEncode, 0, _sigMapEncodeSize);

    //
    //  Encode header
    //		bytes[0-2] : magic
    //		bytes[3] : version number
    //		bytes[4-11] : _sigMapVec.size()
    //		bytes[12-19] : _dimsVec.size()
    //		bytes[20-] : _dimsVec[i]
    //
    encodedMap[0] = encodedMap[1] = encodedMap[2] = 'c';
    encodedMap[3] = VDF_VERSION;

    vector<size_t> header_data;
    header_data.push_back(_sigMapVec.size());
    header_data.push_back(_dimsVec.size());
    for (int i = 0; i < _dimsVec.size(); i++) header_data.push_back(_dimsVec[i]);

    // encode num entries and sigmap dimens
    //
    unsigned char *ucptr = &encodedMap[4];

    for (int i = 0; i < header_data.size(); i++) {
        size_t entry = header_data[i];

        assert(((ucptr + 8) - encodedMap) <= HEADER_SIZE);

        if (do_swapbytes) { swapbytes(&entry, 1); }

        unsigned char *cptr = (unsigned char *)&entry;
        for (int j = 0; j < sizeof(entry); j++) { ucptr[j] = cptr[j]; }
        for (int j = sizeof(entry); j < 8; j++) { ucptr[j] = 0; }
        ucptr += 8;
    }

    unsigned char *ptr = encodedMap + HEADER_SIZE;
    int            bib = BITSPERBYTE;    // bits available in current byte
    int            p = BITSPERBYTE - 1;

    if (!_sorted) SignificanceMap::Sort();

    for (size_t i = 0; i < _sigMapVec.size(); i++) {
        size_t idx = _sigMapVec[i];
        int    tbits = _bits_per_idx;
        while (tbits) {
            int n = min(tbits, bib);
            PUTBITS(*ptr, p, n, idx >> (tbits - n));
            p -= n;
            tbits -= n;
            bib -= n;
            if (bib == 0) {
                ptr++;
                bib = BITSPERBYTE;
                p = BITSPERBYTE - 1;
            }
        }
    }
}

void SignificanceMap::GetMap(const unsigned char **map, size_t *maplen)
{
    *map = NULL;
    *maplen = 0;

    size_t mapsize = GetMapSize();

    if (_sigMapEncodeSize < mapsize) {
        size_t l = mapsize;    // hack to allow word-size reads
        while (l % 4) l++;

        if (_sigMapEncode) { delete[] _sigMapEncode; }
        _sigMapEncode = new unsigned char[l];
        _sigMapEncodeSize = l;
    }
    memset(_sigMapEncode, 0, _sigMapEncodeSize);
    *map = _sigMapEncode;
    *maplen = mapsize;

    return (GetMap(_sigMapEncode));
}

int SignificanceMap::SetMap(const unsigned char *map)
{
    if (map[0] != 'c' || map[1] != 'c' || map[2] != 'c') {
        SetErrMsg("Invalid significance map - bogus header");
        return (-1);
    }
    if (map[3] > VDF_VERSION) {
        SetErrMsg("Invalid significance map - bogus header");
        return (-1);
    }

    unsigned long LSBTest = 1;
    bool          do_swapbytes = false;
    if (!(*(char *)&LSBTest)) {
        // swap to MSBFirst
        do_swapbytes = true;
    }

    unsigned char version = map[3];

    const unsigned char *ucptr = &map[4];
    size_t               numentries = 0;

    unsigned char *cptr = (unsigned char *)&numentries;
    for (int i = 0; i < sizeof(numentries); i++) { cptr[i] = ucptr[i]; }
    ucptr += 8;
    if (do_swapbytes) swapbytes(&numentries, 1);

    size_t header_size = HEADER_SIZE;

    // Check for old style (version 1) encoding
    //
    if (version == 01) {
        if (numentries > _sigMapSize) {
            SetErrMsg("SignificanceMap shape does not match encoded map");
            return (-1);
        }
        header_size = 16;
    } else {
        size_t ndims = 0;
        cptr = (unsigned char *)&ndims;
        for (int i = 0; i < sizeof(ndims); i++) { cptr[i] = ucptr[i]; }
        ucptr += 8;
        if (do_swapbytes) swapbytes(&ndims, 1);

        vector<size_t> dims;
        for (int j = 0; j < ndims; j++) {
            size_t dim = 0;

            cptr = (unsigned char *)&dim;
            for (int i = 0; i < sizeof(dim); i++) { cptr[i] = ucptr[i]; }
            ucptr += 8;
            if (do_swapbytes) swapbytes(&dim, 1);

            dims.push_back(dim);
        }
        if (_SignificanceMap(dims) < 0) return (-1);
    }

    _sigMapVec.clear();
    _sigMapVec.reserve(numentries);

    const unsigned char *ptr = map + header_size;
    int                  bib = BITSPERBYTE;    // bits remaining in current byte

    _sorted = true;
    size_t idxprev = 0;
    for (size_t i = 0; i < numentries; i++) {
        size_t idx = 0;
        int    tbits = _bits_per_idx;
        int    p = _bits_per_idx - 1;
        while (tbits) {
            int n = min(tbits, bib);
            PUTBITS(idx, p, n, *ptr >> (bib - n));
            p -= n;
            tbits -= n;
            bib -= n;
            if (bib == 0) {
                ptr++;
                bib = BITSPERBYTE;
            }
        }
        //
        // Should probably call SignificanceMap::Set() here so
        // that we check for duplicate values. But this is quicker.
        //
        _sigMapVec.push_back(idx);
        if (idx < idxprev) { _sorted = false; }
        idxprev = idx;
    }
    return (0);
}

int SignificanceMap::Append(const SignificanceMap &smap)
{
    if (_sigMapVec.size() == 0) {
        if (_SignificanceMap(smap._dimsVec) < 0) return (-1);
    }

    if (_dimsVec.size() != smap._dimsVec.size()) {
        SetErrMsg("Dimension mismatch");
        return (-1);
    }

    for (int i = 0; i < _dimsVec.size(); i++) {
        if (_dimsVec[i] != smap._dimsVec[i]) {
            SetErrMsg("Dimension mismatch");
            return (-1);
        }
    }

    for (size_t i = 0; i < smap._sigMapVec.size(); i++) {
        int rc = this->Set(smap._sigMapVec[i]);
        if (rc < 0) return (-1);
    }
    return (0);
}

void SignificanceMap::Invert()
{
    vector<size_t> tmpvec = _sigMapVec;
    if (!_sorted) SignificanceMap::Sort();

    _sigMapVec.clear();

    size_t idx = 0;
    for (size_t i = 0; i < tmpvec.size(); i++) {
        while (idx != tmpvec[i]) {
            _sigMapVec.push_back(idx);
            idx++;
        }
        idx++;
    }
    while (idx < _sigMapSize) {
        _sigMapVec.push_back(idx);
        idx++;
    }
}

void SignificanceMap::Sort()
{
    sort(_sigMapVec.begin(), _sigMapVec.end());
    _sorted = true;
}

namespace VAPoR {
std::ostream &operator<<(std::ostream &o, const SignificanceMap &sigmap)
{
    o << "SignificanceMap" << endl;
    o << " _nx : " << sigmap._nx << endl;
    o << " _ny : " << sigmap._ny << endl;
    o << " _nz : " << sigmap._nz << endl;
    o << " _nt : " << sigmap._nt << endl;
    o << " _sorted : " << sigmap._sorted << endl;
    o << " _dimsVec : ";
    for (int i = 0; i < sigmap._dimsVec.size(); i++) { o << sigmap._dimsVec[i] << " "; }
    o << endl;
    o << " _sigMapSize : " << sigmap._sigMapSize << endl;
    o << " _bits_per_idx : " << sigmap._bits_per_idx << endl;
    o << " _sigMapEncodeSize : " << sigmap._sigMapEncodeSize << endl;
    o << " _idxentry : " << sigmap._idxentry << endl;
    o << " _sigMapVec : " << endl;
    for (int i = 0; i < sigmap._sigMapVec.size(); i++) { o << "  " << i << " " << sigmap._sigMapVec[i] << endl; }

    return (o);
}
};    // namespace VAPoR
