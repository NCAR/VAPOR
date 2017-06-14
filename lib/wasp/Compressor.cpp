//
// $Id: Compressor.cpp,v 1.6 2013/05/15 23:05:48 clynejp Exp $
//
#include <cstring>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <vapor/Compressor.h>

using namespace VAPoR;
using namespace std;

void Compressor::_Compressor(
    vector<size_t> dims) {

    if (dims.size() > 3)
        return;

    _dims.clear();
    _nlevels = 0;
    _indexvec.clear();
    _nx = 1;
    _ny = 1;
    _nz = 1;
    _C = NULL;
    _CLen = 0;
    _L = NULL;
    _LLen = 0;
    _keepapp = true;
    _clamp_min_flag = false;
    _clamp_max_flag = false;
    _epsilon_flag = false;
    _clamp_min = 0.0;
    _clamp_max = 1.0;
    _epsilon = 0.0;

    for (int i = 0; i < dims.size(); i++) {
        _dims.push_back(dims[i]);
    }

    _nx = _ny = _nz = 1;
    if (_dims.size() >= 1)
        _nx = _dims[0];
    if (_dims.size() >= 2)
        _ny = _dims[1];
    if (_dims.size() >= 3)
        _nz = _dims[2];

    //
    // Create an appropriate filter bank allocate memory for storing
    // wavelet coefficients
    //
    if (_dims.size() == 3) {

        _nlevels = min(
            min(wmaxlev(_nx), wmaxlev(_ny)), wmaxlev(_nz));

        size_t clen = coefflength3(_nx, _ny, _nz, _nlevels);
        _C = new double[clen];
        _CLen = clen;

        _L = new size_t[(21 * _nlevels) + 6];
        _LLen = (21 * _nlevels) + 6;

        // Compute the bookkeeping vector, L.
        //
        // N.B. L will be recomputed by wavedec(), but not
        // waverec();
        //
        computeL3(_nx, _ny, _nz, _nlevels, _L);
    } else if (_dims.size() == 2) {
        _nlevels = min(wmaxlev(_nx), wmaxlev(_ny));

        size_t clen = coefflength2(_nx, _ny, _nlevels);
        _C = new double[clen];
        _CLen = clen;

        _L = new size_t[(6 * _nlevels) + 4];
        _LLen = (6 * _nlevels) + 4;
        computeL2(_nx, _ny, _nlevels, _L);
    } else {
        _nlevels = wmaxlev(_nx);

        size_t clen = coefflength(_nx, _nlevels);
        _C = new double[clen];
        _CLen = clen;

        _L = new size_t[_nlevels + 2];
        _LLen = _nlevels + 2;
        computeL(_nx, _nlevels, _L);
    }
    _indexvec.reserve(_CLen);
}

Compressor::Compressor(
    vector<size_t> dims, const string &wavename,
    const string &mode) : MatWaveWavedec(wavename, mode) {

    _C = NULL;
    _L = NULL;
    _CLen = 0;
    _LLen = 0;

    _Compressor(dims);
}

Compressor::Compressor(
    vector<size_t> dims, const string &wavename) : MatWaveWavedec(wavename) {

    _C = NULL;
    _L = NULL;
    _CLen = 0;
    _LLen = 0;

    _Compressor(dims);
}

Compressor::~Compressor() {

    if (_C)
        delete[] _C;
    if (_L)
        delete[] _L;
}

//
// Comparision functions for the C++ Std Lib sort function
//
inline bool my_compare_f(const void *x1, const void *x2) {
    return (fabsf(*(float *)x1) > fabsf(*(float *)x2));
}

inline bool my_compare_d(const void *x1, const void *x2) {
    return (fabs(*(double *)x1) > fabs(*(double *)x2));
}

inline bool my_compare_i(const void *x1, const void *x2) {
    return (fabsf(*(int *)x1) > fabsf(*(int *)x2));
}

inline bool my_compare_l(const void *x1, const void *x2) {
    return (abs(*(long *)x1) > abs(*(long *)x2));
}

namespace {

template <class T>
int compress_template(
    Compressor *cmp,
    const T *src_arr,
    T *dst_arr,
    size_t dst_arr_len,
    T *C,
    size_t clen,
    size_t *L,
    SignificanceMap *sigmap,
    const vector<size_t> &dims,
    size_t nlevels,
    vector<void *> indexvec,
    bool my_compare(const void *, const void *)) {

    if (!C) {
        Compressor::SetErrMsg("Invalid state");
        return (-1);
    }

    if ((dims.size() < 1) || (dims.size() > 3)) {
        Compressor::SetErrMsg("Invalid array shape");
        return (-1);
    }

    if (dst_arr_len > clen) {
        Compressor::SetErrMsg(
            "Invalid array destination array length : %lu",
            dst_arr_len);
        return (-1);
    }

    //
    // Forward wavelet transform the array and compute the number of
    // approximation coefficients (numkeep).
    //
    size_t numkeep = 0;
    int rc = 0;
    if (dims.size() == 3) {
        if (cmp->KeepAppOnOff())
            numkeep = L[0] * L[1] * L[2];
        rc = cmp->wavedec3(src_arr, dims[0], dims[1], dims[2], nlevels, C, L);
    } else if (dims.size() == 2) {
        if (cmp->KeepAppOnOff())
            numkeep = L[0] * L[1];
        rc = cmp->wavedec2(src_arr, dims[0], dims[1], nlevels, C, L);
    } else if (dims.size() == 1) {
        if (cmp->KeepAppOnOff())
            numkeep = L[0];
        rc = cmp->wavedec(src_arr, dims[0], nlevels, C, L);
    }
    if (rc < 0)
        return (-1);

    assert(dst_arr_len >= numkeep);

    rc = sigmap->Reshape(clen);
    if (rc < 0)
        return (-1);

    sigmap->Clear();

    // Data has been transformed. Now we need to sort it and find
    // the threshold value. Note: we don't actually move the data. We
    // sort an index array that references the data array.

    for (size_t i = 0; i < dst_arr_len; i++)
        dst_arr[i] = 0.0;

    if (numkeep) {
        // If numkeep>0, copy approximation coeffs. verbatim
        //
        for (size_t idx = 0; idx < numkeep; idx++) {
            rc = sigmap->Set(idx);
            if (rc < 0)
                return (-1);
            dst_arr[idx] = C[idx];
        }
        if (numkeep == dst_arr_len)
            return (0);
        dst_arr += numkeep;
        dst_arr_len -= numkeep;
    }

    indexvec.clear();
    for (size_t i = numkeep; i < clen; i++)
        indexvec.push_back(&C[i]);
    sort(indexvec.begin(), indexvec.end(), my_compare);

    // Copy coefficients that are larger than the threshold to
    // the destination array. Record their location in the significance
    // map.
    //

    sort(indexvec.begin(), indexvec.begin() + dst_arr_len);
    for (size_t idx = numkeep, i = 0; idx < clen && i < dst_arr_len; idx++) {
        const T *cptr = (T *)indexvec[i];
        dst_arr[i++] = *cptr;
        sigmap->Set(cptr - C);
    }
    return (0);
}
}; // namespace

int Compressor::Compress(
    const float *src_arr,
    float *dst_arr,
    size_t dst_arr_len,
    SignificanceMap *sigmap) {

    return compress_template(
        this, src_arr, dst_arr, dst_arr_len, (float *)_C, _CLen,
        _L, sigmap, _dims, _nlevels, _indexvec, my_compare_f);
}

int Compressor::Compress(
    const double *src_arr,
    double *dst_arr,
    size_t dst_arr_len,
    SignificanceMap *sigmap) {

    return compress_template(
        this, src_arr, dst_arr, dst_arr_len, (double *)_C, _CLen,
        _L, sigmap, _dims, _nlevels, _indexvec, my_compare_d);
}

int Compressor::Compress(
    const int *src_arr,
    int *dst_arr,
    size_t dst_arr_len,
    SignificanceMap *sigmap) {

    return compress_template(
        this, src_arr, dst_arr, dst_arr_len, (int *)_C, _CLen,
        _L, sigmap, _dims, _nlevels, _indexvec, my_compare_i);
}

int Compressor::Compress(
    const long *src_arr,
    long *dst_arr,
    size_t dst_arr_len,
    SignificanceMap *sigmap) {

    return compress_template(
        this, src_arr, dst_arr, dst_arr_len, (long *)_C, _CLen,
        _L, sigmap, _dims, _nlevels, _indexvec, my_compare_l);
}

namespace {
template <class T>
int decompress_template(
    Compressor *cmp,
    const T *src_arr,
    T *dst_arr,
    T *C,
    size_t clen,
    const size_t *L,
    int nlevels,
    SignificanceMap *sigmap,
    const vector<size_t> &dims) {
    if (!C) {
        Compressor::SetErrMsg("Invalid state");
        return (-1);
    }

    if ((dims.size() < 1) || (dims.size() > 3)) {
        Compressor::SetErrMsg("Invalid array shape");
        return (-1);
    }

    for (size_t i = 0; i < clen; i++) {
        C[i] = 0.0;
    }
    //
    // Restore the non-zero wavelet coefficients
    //
    sigmap->GetNextEntryRestart();

    size_t nsig = sigmap->GetNumSignificant();
    for (size_t i = 0; i < nsig; i++) {
        size_t idx;

        if (!sigmap->GetNextEntry(&idx)) {
            Compressor::SetErrMsg("Invalid significance map");
            return (-1);
        }

        C[idx] = src_arr[i];
    }

    int rc = 0;
    size_t dst_dim[] = {1, 1, 1};
    if (dims.size() == 3) {
        //rc = cmp->waverec3(C, L, nlevels, dst_arr);
        rc = cmp->appcoef3(C, L, nlevels, nlevels, true, dst_arr);
        cmp->approxlength3(L, nlevels, nlevels, &dst_dim[0], &dst_dim[1], &dst_dim[2]);
    } else if (dims.size() == 2) {
        //rc = cmp->waverec2(C, L, nlevels, dst_arr);
        rc = cmp->appcoef2(C, L, nlevels, nlevels, true, dst_arr);
        cmp->approxlength2(L, nlevels, nlevels, &dst_dim[0], &dst_dim[1]);
    } else if (dims.size() == 1) {
        //cmp->waverec(C, L, nlevels, dst_arr);
        rc = cmp->appcoef(C, L, nlevels, nlevels, true, dst_arr);
        cmp->approxlength(L, nlevels, nlevels, &dst_dim[0]);
    }
    if (rc < 0)
        return (rc);

    if (cmp->ClampMinOnOff() || cmp->ClampMaxOnOff() || cmp->EpsilonOnOff()) {
        bool clamp_min_f = cmp->ClampMinOnOff();
        double clamp_min = cmp->ClampMin();
        bool clamp_max_f = cmp->ClampMaxOnOff();
        double clamp_max = cmp->ClampMax();
        bool epsilon_f = cmp->EpsilonOnOff();
        double epsilon = fabs(cmp->Epsilon());

        size_t sz = dst_dim[0] * dst_dim[1] * dst_dim[2];
        for (size_t i = 0; i < sz; i++) {
            if (clamp_min_f && dst_arr[i] < clamp_min)
                dst_arr[i] = clamp_min;
            if (clamp_max_f && dst_arr[i] > clamp_max)
                dst_arr[i] = clamp_max;
            if (epsilon_f && abs(dst_arr[i]) < epsilon)
                dst_arr[i] = 0.0;
        }
    }

    return (0);
}

}; // namespace

int Compressor::Decompress(
    const float *src_arr,
    float *dst_arr,
    SignificanceMap *sigmap) {

    return decompress_template(
        this, src_arr, dst_arr, (float *)_C, _CLen, _L,
        _nlevels, sigmap, _dims);
}

int Compressor::Decompress(
    const double *src_arr,
    double *dst_arr,
    SignificanceMap *sigmap) {

    return decompress_template(
        this, src_arr, dst_arr, (double *)_C, _CLen, _L,
        _nlevels, sigmap, _dims);
}

int Compressor::Decompress(
    const int *src_arr,
    int *dst_arr,
    SignificanceMap *sigmap) {

    return decompress_template(
        this, src_arr, dst_arr, (int *)_C, _CLen, _L,
        _nlevels, sigmap, _dims);
}

int Compressor::Decompress(
    const long *src_arr,
    long *dst_arr,
    SignificanceMap *sigmap) {

    return decompress_template(
        this, src_arr, dst_arr, (long *)_C, _CLen, _L,
        _nlevels, sigmap, _dims);
}

namespace {
template <class T>
int decompose_template(
    Compressor *cmp,
    const T *src_arr,
    T *dst_arr,
    const vector<size_t> &dst_arr_lens,
    T *C,
    size_t clen,
    size_t *L,
    vector<SignificanceMap> &sigmaps,
    const vector<size_t> &dims,
    size_t nlevels,
    vector<void *> indexvec,
    bool my_compare(const void *, const void *)) {
    if (!C) {
        Compressor::SetErrMsg("Invalid state");
        return (-1);
    }

    if (sigmaps.size() != dst_arr_lens.size()) {
        Compressor::SetErrMsg("Invalid parameter");
        return (-1);
    }

    vector<size_t> my_dst_arr_lens = dst_arr_lens;

    size_t tlen = 0; // total # of coefficients to retain
    for (int i = 0; i < my_dst_arr_lens.size(); i++) {
        tlen += my_dst_arr_lens[i];
    }
    if (tlen > clen) {
        Compressor::SetErrMsg("Invalid decomposition");
        return (-1);
    }

    if ((dims.size() < 1) || (dims.size() > 3)) {
        Compressor::SetErrMsg("Invalid array shape");
        return (-1);
    }

    //
    // Forward wavelet transform the array and compute the number of
    // approximation coefficients (numkeep).
    //
    size_t numkeep = 0;
    int rc = 0;
    if (dims.size() == 3) {
        if (cmp->KeepAppOnOff())
            numkeep = L[0] * L[1] * L[2];
        rc = cmp->wavedec3(src_arr, dims[0], dims[1], dims[2], nlevels, C, L);
    } else if (dims.size() == 2) {
        if (cmp->KeepAppOnOff())
            numkeep = L[0] * L[1];
        rc = cmp->wavedec2(src_arr, dims[0], dims[1], nlevels, C, L);
    } else if (dims.size() == 1) {
        if (cmp->KeepAppOnOff())
            numkeep = L[0];
        rc = cmp->wavedec(src_arr, dims[0], nlevels, C, L);
    }
    if (rc < 0)
        return (-1);

    if (my_dst_arr_lens[0] < numkeep) {
        Compressor::SetErrMsg("Invalid decomposition - not enougth coefficients");
        return (-1);
    }

    for (int i = 0; i < sigmaps.size(); i++) {
        rc = sigmaps[i].Reshape(clen);
        if (rc < 0)
            return (-1);
        sigmaps[i].Clear();
    }

    // Data has been transformed. Now we need to sort it and find
    // the threshold value. Note: we don't actually move the data. We
    // sort an index array that references the data array.

    for (size_t i = 0; i < tlen; i++)
        dst_arr[i] = 0.0;

    if (numkeep) {
        // If numkeep>0, copy approximation coeffs. verbatim
        //
        for (size_t idx = 0; idx < numkeep; idx++) {
            rc = sigmaps[0].Set(idx);
            if (rc < 0)
                return (-1);
            dst_arr[idx] = C[idx];
        }
        if (numkeep == tlen)
            return (0);
        dst_arr += numkeep;
        my_dst_arr_lens[0] -= numkeep;
    }

    //
    // sort the **indecies** of the coefficients based on the
    // coefficient's magnitude
    //
    indexvec.clear();
    for (size_t i = numkeep; i < clen; i++)
        indexvec.push_back(&C[i]);
    sort(indexvec.begin(), indexvec.end(), my_compare);

    vector<void *>::iterator itr = indexvec.begin();
    for (int j = 0, idx = 0; j < my_dst_arr_lens.size(); j++) {
        sort(itr, itr + my_dst_arr_lens[j]); // sort coefficient's indecies
        itr += my_dst_arr_lens[j];
        for (int i = 0; i < my_dst_arr_lens[j]; i++, idx++) {
            const T *cptr = (T *)indexvec[idx];
            dst_arr[i] = *cptr;
            sigmaps[j].Set(cptr - C);
        }
        dst_arr += my_dst_arr_lens[j];
    }

    return (0);
}

template <class T>
int reconstruct_template(
    Compressor *cmp,
    const T *src_arr,
    T *dst_arr,
    T *C,
    size_t clen,
    const size_t *L,
    int nlevels,
    int l,
    vector<SignificanceMap> &sigmaps,
    const vector<size_t> &dims) {
    if (!C) {
        Compressor::SetErrMsg("Invalid state");
        return (-1);
    }

    if ((dims.size() < 1) || (dims.size() > 3)) {
        Compressor::SetErrMsg("Invalid array shape");
        return (-1);
    }

    for (size_t count = 0; count < clen; count++) {
        C[count] = 0.0;
    }

    size_t count = 0;
    size_t idx;
    for (int j = 0; j < sigmaps.size(); j++) {
        sigmaps[j].GetNextEntryRestart();

        size_t nsig = sigmaps[j].GetNumSignificant();
        for (size_t i = 0; i < nsig; i++) {

            if (!sigmaps[j].GetNextEntry(&idx)) {
                Compressor::SetErrMsg("Invalid significance map");
                return (-1);
            }

            C[idx] = src_arr[count];
            count++;
        }
    }

    int rc = 0;
    size_t dst_dim[] = {1, 1, 1};
    if (dims.size() == 3) {
        //cmp->waverec3(C, L, nlevels, dst_arr);
        rc = cmp->appcoef3(C, L, nlevels, l, true, dst_arr);
        cmp->approxlength3(L, nlevels, l, &dst_dim[0], &dst_dim[1], &dst_dim[2]);
    } else if (dims.size() == 2) {
        //cmp->waverec2(C, L, nlevels, dst_arr);
        rc = cmp->appcoef2(C, L, nlevels, l, true, dst_arr);
        cmp->approxlength2(L, nlevels, l, &dst_dim[0], &dst_dim[1]);
    } else if (dims.size() == 1) {
        //cmp->waverec(C, L, nlevels, dst_arr);
        rc = cmp->appcoef(C, L, nlevels, l, true, dst_arr);
        cmp->approxlength(L, nlevels, l, &dst_dim[0]);
    }
    if (rc < 0)
        return (-1);

    if (cmp->ClampMinOnOff() || cmp->ClampMaxOnOff()) {
        bool clamp_min_f = cmp->ClampMinOnOff();
        double clamp_min = cmp->ClampMin();
        bool clamp_max_f = cmp->ClampMaxOnOff();
        double clamp_max = cmp->ClampMax();

        size_t sz = dst_dim[0] * dst_dim[1] * dst_dim[2];
        for (size_t i = 0; i < sz; i++) {
            if (clamp_min_f && dst_arr[i] < clamp_min)
                dst_arr[i] = clamp_min;
            if (clamp_max_f && dst_arr[i] > clamp_max)
                dst_arr[i] = clamp_max;
        }
    }

    return (0);
}

}; // namespace

int Compressor::Decompose(
    const float *src_arr, float *dst_arr, const vector<size_t> &dst_arr_lens,
    vector<SignificanceMap> &sigmaps) {
    return decompose_template(
        this, src_arr, dst_arr, dst_arr_lens, (float *)_C, _CLen,
        _L, sigmaps, _dims, _nlevels, _indexvec, my_compare_f);
}

int Compressor::Decompose(
    const double *src_arr, double *dst_arr, const vector<size_t> &dst_arr_lens,
    vector<SignificanceMap> &sigmaps) {
    return decompose_template(
        this, src_arr, dst_arr, dst_arr_lens, (double *)_C, _CLen,
        _L, sigmaps, _dims, _nlevels, _indexvec, my_compare_d);
}

int Compressor::Decompose(
    const int *src_arr, int *dst_arr, const vector<size_t> &dst_arr_lens,
    vector<SignificanceMap> &sigmaps) {
    return decompose_template(
        this, src_arr, dst_arr, dst_arr_lens, (int *)_C, _CLen,
        _L, sigmaps, _dims, _nlevels, _indexvec, my_compare_i);
}

int Compressor::Decompose(
    const long *src_arr, long *dst_arr, const vector<size_t> &dst_arr_lens,
    vector<SignificanceMap> &sigmaps) {
    return decompose_template(
        this, src_arr, dst_arr, dst_arr_lens, (long *)_C, _CLen,
        _L, sigmaps, _dims, _nlevels, _indexvec, my_compare_l);
}

int Compressor::Reconstruct(
    const float *src_arr, float *dst_arr,
    vector<SignificanceMap> &sigmaps, int l) {
    if (l == -1)
        l = GetNumLevels();
    return reconstruct_template(
        this, src_arr, dst_arr, (float *)_C, _CLen, _L, _nlevels, l, sigmaps,
        _dims);
}

int Compressor::Reconstruct(
    const double *src_arr, double *dst_arr,
    vector<SignificanceMap> &sigmaps, int l) {
    if (l == -1)
        l = GetNumLevels();
    return reconstruct_template(
        this, src_arr, dst_arr, (double *)_C, _CLen, _L, _nlevels, l, sigmaps,
        _dims);
}

int Compressor::Reconstruct(
    const int *src_arr, int *dst_arr,
    vector<SignificanceMap> &sigmaps, int l) {
    if (l == -1)
        l = GetNumLevels();
    return reconstruct_template(
        this, src_arr, dst_arr, (int *)_C, _CLen, _L, _nlevels, l, sigmaps,
        _dims);
}

int Compressor::Reconstruct(
    const long *src_arr, long *dst_arr,
    vector<SignificanceMap> &sigmaps, int l) {
    if (l == -1)
        l = GetNumLevels();
    return reconstruct_template(
        this, src_arr, dst_arr, (long *)_C, _CLen, _L, _nlevels, l, sigmaps,
        _dims);
}

#ifdef DEAD
bool Compressor::IsCompressible(
    vector<size_t> dims, const string &wavename, const string &mode) {

    if (dims.size() < 1 || dims.size() > 3) {
        return (false);
    }

    size_t nx;
    size_t ny;
    size_t nz;

    nx = ny = nz = 1;
    if (dims.size() >= 1)
        nx = dims[0];
    if (dims.size() >= 2)
        ny = dims[1];
    if (dims.size() >= 3)
        nz = dims[2];

    int nlevels;
    if (dims.size() == 3) {

        nlevels = min(min(wmaxlev(nx), wmaxlev(ny)), wmaxlev(nz));
    } else if (dims.size() == 2) {
        nlevels = min(wmaxlev(nx), wmaxlev(ny));
    } else {
        nlevels = wmaxlev(nx);
    }

    return (nlevels > 0);
}
#endif

void Compressor::GetDimension(vector<size_t> &dims, int l) const {

    if (l < 0 || l > _nlevels)
        l = _nlevels;

    dims.clear();
    for (int i = 0; i < _dims.size(); i++) {
        size_t len = approxlength(_dims[i], _nlevels - l);
        dims.push_back(len);
    }
}

size_t Compressor::GetMinCompression() const {
    if (!_keepapp)
        return (1);

    if (_dims.size() == 3) {
        return (_L[0] * _L[1] * _L[2]);
    } else if (_dims.size() == 2) {
        return (_L[0] * _L[1]);
    } else if (_dims.size() == 1) {
        return (_L[0]);
    }
    return (0);
}

bool Compressor::CompressionInfo(
    vector<size_t> dims, const string wavename,
    bool keepapp, size_t &nlevels, size_t &maxcratio) {
    nlevels = 0;
    maxcratio = 0;

    if (dims.size() < 1 || dims.size() > 3)
        return (false);

    for (int i = 0; i < dims.size(); i++) {
        if (dims[i] < 1)
            return (false);
    }

    MatWaveWavedec mww(wavename);

    size_t ncoeff = 1;
    for (int i = 0; i < dims.size(); i++)
        ncoeff *= dims[i];

    size_t mincoeff = 1;
    if (keepapp) {
        size_t *L;
        if (dims.size() == 1) {
            nlevels = mww.wmaxlev(dims[0]);
            L = new size_t[nlevels + 2];
            mww.computeL(dims[0], nlevels, L);
            mincoeff = L[0];
        }
        if (dims.size() == 2) {
            nlevels = min(mww.wmaxlev(dims[0]), mww.wmaxlev(dims[1]));
            L = new size_t[(6 * nlevels) + 4];
            mww.computeL2(dims[0], dims[1], nlevels, L);
            mincoeff = L[0] * L[1];
        }
        if (dims.size() == 3) {
            nlevels = min(
                min(mww.wmaxlev(dims[0]), mww.wmaxlev(dims[1])),
                mww.wmaxlev(dims[2]));
            L = new size_t[(21 * nlevels) + 6];
            mww.computeL3(dims[0], dims[1], dims[2], nlevels, L);
            mincoeff = L[0] * L[1] * L[2];
        }
        delete[] L;
    }
    nlevels += 1;

    maxcratio = ncoeff / mincoeff;
    return (true);
}

namespace VAPoR {
ostream &operator<<(std::ostream &o, const Compressor &rhs) {
    o << "Compressor:" << endl;
    o << " Dimensions:" << endl;
    for (int i = 0; i < rhs._dims.size(); i++) {
        o << "  " << rhs._dims[i] << " ";
    }
    o << endl;
    o << " Coefficients length " << rhs._CLen << endl;
    o << " Book keeping length " << rhs._LLen << endl;
    o << " Keep approx flag " << rhs._keepapp << endl;
    o << " Clamp min flag " << rhs._clamp_min_flag << endl;
    o << " Clamp max flag " << rhs._clamp_max_flag << endl;
    o << " Clamp min " << rhs._clamp_min << endl;
    o << " Clamp max " << rhs._clamp_max << endl;
    o << " Epsilon flag " << rhs._epsilon_flag << endl;
    o << " Epsilon " << rhs._epsilon << endl;

    return (o);
}
}; // namespace VAPoR
