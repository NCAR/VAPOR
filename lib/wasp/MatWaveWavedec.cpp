
#include <iostream>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <vapor/MatWaveWavedec.h>

using namespace VAPoR;

MatWaveWavedec::MatWaveWavedec(const string &wname, const string &mode) : MatWaveDwt(wname, mode) {}

MatWaveWavedec::MatWaveWavedec(const string &wname) : MatWaveDwt(wname) {}

MatWaveWavedec::~MatWaveWavedec() {}

template<class T> int wavedec_template(MatWaveWavedec *mww, const T *sigIn, size_t sigInLength, int n, T *C, size_t *L)
{
    if (!mww->wavelet()) {
        MatWaveWavedec::SetErrMsg("Invalid state, no wavelet");
        return (-1);
    }

    if (n < 0 || n > mww->wmaxlev(sigInLength)) {
        MatWaveWavedec::SetErrMsg("Invalid number of transforms : %d", n);
        return (-1);
    }

    //
    // Compute bookkeeping vector, L, and length of output vector, C
    //
    mww->computeL(sigInLength, n, L);
    size_t CLength = mww->coefflength(L, n);

    if (n == 0) {
        for (size_t i = 0; i < sigInLength; i++) C[i] = sigIn[i];
        return (0);
    }

    int      rc;
    const T *sigInPtr = sigIn;

    size_t len = sigInLength;
    size_t cALen = mww->MatWaveBase::approxlength(len);
    T *    cptr;
    size_t tlen = 0;
    size_t L1d[3];
    for (int i = n; i > 0; i--) {
        tlen += L[i];
        cptr = C + CLength - tlen - cALen;
        rc = mww->dwt(sigInPtr, len, cptr, L1d);
        if (rc < 0) return (rc);

        len = cALen;
        cALen = mww->MatWaveBase::approxlength(cALen);
        sigInPtr = cptr;
    }

    return (0);
}

int MatWaveWavedec::wavedec(const double *sigIn, size_t sigInLength, int n, double *C, size_t *L) { return wavedec_template(this, sigIn, sigInLength, n, C, L); }

int MatWaveWavedec::wavedec(const float *sigIn, size_t sigInLength, int n, float *C, size_t *L) { return wavedec_template(this, sigIn, sigInLength, n, C, L); }

int MatWaveWavedec::wavedec(const long *sigIn, size_t sigInLength, int n, long *C, size_t *L) { return wavedec_template(this, sigIn, sigInLength, n, C, L); }

int MatWaveWavedec::wavedec(const int *sigIn, size_t sigInLength, int n, int *C, size_t *L) { return wavedec_template(this, sigIn, sigInLength, n, C, L); }

template<class T> int waverec_template(MatWaveWavedec *mww, const T *C, const size_t *L, int n, int l, bool normal, T *sigOut)
{
    if (!mww->wavelet()) {
        MatWaveWavedec::SetErrMsg("Invalid state, no wavelet");
        return (-1);
    }
    if (n < 0) {
        MatWaveWavedec::SetErrMsg("Invalid number of transforms : %d", n);
        return (-1);
    }

    if (l < 0 || l > n) l = n;
    int LLength = n + 2;

    if (l == 0) {
        double scale = 1.0;
        if (normal) {
            for (int i = l; i < n; i++) scale /= sqrt(2.0);
        }
        for (size_t i = 0; i < L[0]; i++) sigOut[i] = scale * C[i];
        return (0);
    }

    const T *cA = C;
    const T *cD = cA + L[0];
    size_t   L1d[3] = {L[0], L[1], 0};
    for (int i = 1; i <= l; i++) {
        L1d[2] = mww->approxlength(L[LLength - 1], n - i);

        int rc = mww->idwt(cA, cD, L1d, sigOut);
        if (rc < 0) return (rc);
        if (i == l) break;

        cA = sigOut;
        cD += L[i];
        L1d[0] = L1d[2];
        L1d[1] = L[i + 1];
    }

    if (l != n && normal) {
        double scale = 1.0;
        for (int i = l; i < n; i++) scale /= sqrt(2.0);
        for (size_t i = 0; i < L1d[2]; i++) sigOut[i] *= scale;
    }

    return (0);
}

int MatWaveWavedec::waverec(const double *C, const size_t *L, int n, double *sigOut) { return waverec_template(this, C, L, n, n, false, sigOut); }

int MatWaveWavedec::waverec(const float *C, const size_t *L, int n, float *sigOut) { return waverec_template(this, C, L, n, n, false, sigOut); }

int MatWaveWavedec::waverec(const long *C, const size_t *L, int n, long *sigOut) { return waverec_template(this, C, L, n, n, false, sigOut); }

int MatWaveWavedec::waverec(const int *C, const size_t *L, int n, int *sigOut) { return waverec_template(this, C, L, n, n, false, sigOut); }

int MatWaveWavedec::appcoef(const double *C, const size_t *L, int n, int l, bool normal, double *sigOut) { return waverec_template(this, C, L, n, l, normal, sigOut); }

int MatWaveWavedec::appcoef(const float *C, const size_t *L, int n, int l, bool normal, float *sigOut) { return waverec_template(this, C, L, n, l, normal, sigOut); }

int MatWaveWavedec::appcoef(const long *C, const size_t *L, int n, int l, bool normal, long *sigOut) { return waverec_template(this, C, L, n, l, normal, sigOut); }

int MatWaveWavedec::appcoef(const int *C, const size_t *L, int n, int l, bool normal, int *sigOut) { return waverec_template(this, C, L, n, l, normal, sigOut); }

template<class T> int wavedec2_template(MatWaveWavedec *mww, const T *sigIn, size_t sigInX, size_t sigInY, int n, T *C, size_t *L)
{
    if (!mww->wavelet()) {
        MatWaveWavedec::SetErrMsg("Invalid state, no wavelet");
        return (-1);
    }

    if (n < 0 || n > min(mww->wmaxlev(sigInX), mww->wmaxlev(sigInY))) {
        MatWaveWavedec::SetErrMsg("Invalid number of transforms : %d", n);
        return (-1);
    }

    mww->computeL2(sigInX, sigInY, n, L);
    size_t CLength = mww->coefflength2(L, n);

    if (n == 0) {
        for (size_t j = 0; j < sigInY; j++) {
            for (size_t i = 0; i < sigInX; i++) { *C++ = *sigIn++; }
        }
        return (0);
    }

    int      rc;
    const T *sigInPtr = sigIn;

    size_t lenx = sigInX;
    size_t leny = sigInY;
    size_t cALenX = mww->MatWaveBase::approxlength(sigInX);
    size_t cALenY = mww->MatWaveBase::approxlength(sigInY);
    T *    cptr;
    size_t tlen = 0;

    size_t L2d[10];
    for (int i = n; i > 0; i--) {
        tlen += (L[(6 * i) - 4] * L[(6 * i) - 3]) +    // cDh
                (L[(6 * i) - 2] * L[(6 * i) - 1]) +    // cDv
                (L[(6 * i) + 0] * L[(6 * i) + 1]);     // cDd

        cptr = C + CLength - tlen - (cALenX * cALenY);

        rc = mww->dwt2d(sigInPtr, lenx, leny, cptr, L2d);
        if (rc < 0) return (rc);

        lenx = cALenX;
        leny = cALenY;
        cALenX = mww->MatWaveBase::approxlength(cALenX);
        cALenY = mww->MatWaveBase::approxlength(cALenY);
        sigInPtr = cptr;
    }

    return (0);
}

int MatWaveWavedec::wavedec2(const double *sigIn, size_t sigInX, size_t sigInY, int n, double *C, size_t *L) { return wavedec2_template(this, sigIn, sigInX, sigInY, n, C, L); }

int MatWaveWavedec::wavedec2(const float *sigIn, size_t sigInX, size_t sigInY, int n, float *C, size_t *L) { return wavedec2_template(this, sigIn, sigInX, sigInY, n, C, L); }

int MatWaveWavedec::wavedec2(const long *sigIn, size_t sigInX, size_t sigInY, int n, long *C, size_t *L) { return wavedec2_template(this, sigIn, sigInX, sigInY, n, C, L); }

int MatWaveWavedec::wavedec2(const int *sigIn, size_t sigInX, size_t sigInY, int n, int *C, size_t *L) { return wavedec2_template(this, sigIn, sigInX, sigInY, n, C, L); }

template<class T> int waverec2_template(MatWaveWavedec *mww, const T *C, const size_t *L, int n, int l, bool normal, T *sigOut)
{
    if (!mww->wavelet()) {
        MatWaveWavedec::SetErrMsg("Invalid state, no wavelet");
        return (-1);
    }

    if (n < 0) {
        MatWaveWavedec::SetErrMsg("Invalid number of transforms : %d", n);
        return (-1);
    }

    if (l < 0 || l > n) l = n;
    int LLength = 6 * n + 4;

    if (l == 0) {
        double scale = 1.0;
        if (normal) {
            for (int i = l; i < n; i++) scale /= (sqrt(2.0) * sqrt(2.0));
        }
        for (size_t j = 0; j < L[3]; j++) {
            for (size_t i = 0; i < L[2]; i++) { *sigOut++ = scale * *C++; }
        }
        return (0);
    }

    const T *cA = C;
    const T *cDh = cA + L[0] * L[1];
    const T *cDv = cDh + L[2] * L[3];
    const T *cDd = cDv + L[4] * L[5];

    size_t L2d[10] = {L[0], L[1], L[2], L[3], L[4], L[5], L[6], L[7], 0, 0};
    for (int i = 1; i <= l; i++) {
        L2d[8] = mww->approxlength(L[LLength - 2], n - i);
        L2d[9] = mww->approxlength(L[LLength - 1], n - i);

        int rc = mww->idwt2d(cA, cDh, cDv, cDd, L2d, sigOut);
        if (rc < 0) return (rc);
        if (i == l) break;

        cA = sigOut;
        cDh += L[6 * (i - 1) + 2] * L[6 * (i - 1) + 3] + L[6 * (i - 1) + 4] * L[6 * (i - 1) + 5] + L[6 * (i - 1) + 6] * L[6 * (i - 1) + 7];

        cDv = cDh + L[6 * i + 2] * L[6 * i + 3];
        cDd = cDv + L[6 * i + 4] * L[6 * i + 5];

        L2d[0] = L2d[8];
        L2d[1] = L2d[9];
        L2d[2] = L[6 * i + 2];
        L2d[3] = L[6 * i + 3];
        L2d[4] = L[6 * i + 4];
        L2d[5] = L[6 * i + 5];
        L2d[6] = L[6 * i + 6];
        L2d[7] = L[6 * i + 7];
    }

    if (l != n && normal) {
        double scale = 1.0;
        for (int i = l; i < n; i++) scale /= (sqrt(2.0) * sqrt(2.0));
        for (size_t i = 0; i < L2d[8] * L2d[9]; i++) sigOut[i] *= scale;
    }

    return (0);
}

int MatWaveWavedec::waverec2(const double *C, const size_t *L, int n, double *sigOut) { return waverec2_template(this, C, L, n, n, false, sigOut); }

int MatWaveWavedec::waverec2(const float *C, const size_t *L, int n, float *sigOut) { return waverec2_template(this, C, L, n, n, false, sigOut); }

int MatWaveWavedec::waverec2(const long *C, const size_t *L, int n, long *sigOut) { return waverec2_template(this, C, L, n, n, false, sigOut); }

int MatWaveWavedec::waverec2(const int *C, const size_t *L, int n, int *sigOut) { return waverec2_template(this, C, L, n, n, false, sigOut); }

int MatWaveWavedec::appcoef2(const double *C, const size_t *L, int n, int l, bool normal, double *sigOut) { return waverec2_template(this, C, L, n, l, normal, sigOut); }

int MatWaveWavedec::appcoef2(const float *C, const size_t *L, int n, int l, bool normal, float *sigOut) { return waverec2_template(this, C, L, n, l, normal, sigOut); }

int MatWaveWavedec::appcoef2(const long *C, const size_t *L, int n, int l, bool normal, long *sigOut) { return waverec2_template(this, C, L, n, l, normal, sigOut); }

int MatWaveWavedec::appcoef2(const int *C, const size_t *L, int n, int l, bool normal, int *sigOut) { return waverec2_template(this, C, L, n, l, normal, sigOut); }

template<class T> int wavedec3_template(MatWaveWavedec *mww, const T *sigIn, size_t sigInX, size_t sigInY, size_t sigInZ, int n, T *C, size_t *L)
{
    if (!mww->wavelet()) {
        MatWaveWavedec::SetErrMsg("Invalid state, no wavelet");
        return (-1);
    }

    if (n < 0 || n > min(min(mww->wmaxlev(sigInX), mww->wmaxlev(sigInY)), mww->wmaxlev(sigInZ))) {
        MatWaveWavedec::SetErrMsg("Invalid number of transforms : %d", n);
        return (-1);
    }

    mww->computeL3(sigInX, sigInY, sigInZ, n, L);
    size_t CLength = mww->coefflength3(L, n);

    if (n == 0) {
        for (size_t k = 0; k < sigInZ; k++) {
            for (size_t j = 0; j < sigInY; j++) {
                for (size_t i = 0; i < sigInX; i++) { *C++ = *sigIn++; }
            }
        }
        return (0);
    }

    int      rc;
    const T *sigInPtr = sigIn;

    size_t lenx = sigInX;
    size_t leny = sigInY;
    size_t lenz = sigInZ;
    size_t cALenX = mww->MatWaveBase::approxlength(sigInX);
    size_t cALenY = mww->MatWaveBase::approxlength(sigInY);
    size_t cALenZ = mww->MatWaveBase::approxlength(sigInZ);
    T *    cptr;
    size_t tlen = 0;

    size_t L3d[27];

    for (int i = n; i > 0; i--) {
        tlen += (L[(21 * i) - 18] * L[(21 * i) - 17] * L[(21 * i) - 16]) +    // cLLH
                (L[(21 * i) - 15] * L[(21 * i) - 14] * L[(21 * i) - 13]) +    // cLHL
                (L[(21 * i) - 12] * L[(21 * i) - 11] * L[(21 * i) - 10]) +    // cLHH
                (L[(21 * i) - 9] * L[(21 * i) - 8] * L[(21 * i) - 7]) +       // cHLL
                (L[(21 * i) - 6] * L[(21 * i) - 5] * L[(21 * i) - 4]) +       // cHLH
                (L[(21 * i) - 3] * L[(21 * i) - 2] * L[(21 * i) - 1]) +       // HHL
                (L[(21 * i) + 0] * L[(21 * i) + 1] * L[(21 * i) + 2]);        // HHH

        cptr = C + CLength - tlen - (cALenX * cALenY * cALenZ);

        rc = mww->dwt3d(sigInPtr, lenx, leny, lenz, cptr, L3d);
        if (rc < 0) return (rc);

        lenx = cALenX;
        leny = cALenY;
        lenz = cALenZ;

        cALenX = mww->MatWaveBase::approxlength(cALenX);
        cALenY = mww->MatWaveBase::approxlength(cALenY);
        cALenZ = mww->MatWaveBase::approxlength(cALenZ);

        sigInPtr = cptr;
    }

    return (0);
}

int MatWaveWavedec::wavedec3(const double *sigIn, size_t sigInX, size_t sigInY, size_t sigInZ, int n, double *C, size_t *L) { return wavedec3_template(this, sigIn, sigInX, sigInY, sigInZ, n, C, L); }

int MatWaveWavedec::wavedec3(const float *sigIn, size_t sigInX, size_t sigInY, size_t sigInZ, int n, float *C, size_t *L) { return wavedec3_template(this, sigIn, sigInX, sigInY, sigInZ, n, C, L); }

int MatWaveWavedec::wavedec3(const long *sigIn, size_t sigInX, size_t sigInY, size_t sigInZ, int n, long *C, size_t *L) { return wavedec3_template(this, sigIn, sigInX, sigInY, sigInZ, n, C, L); }

int MatWaveWavedec::wavedec3(const int *sigIn, size_t sigInX, size_t sigInY, size_t sigInZ, int n, int *C, size_t *L) { return wavedec3_template(this, sigIn, sigInX, sigInY, sigInZ, n, C, L); }

template<class T> int waverec3_template(MatWaveWavedec *mww, const T *C, const size_t *L, int n, int l, bool normal, T *sigOut)
{
    if (!mww->wavelet()) {
        MatWaveWavedec::SetErrMsg("Invalid state, no wavelet");
        return (-1);
    }

    if (n < 0) {
        MatWaveWavedec::SetErrMsg("Invalid number of transforms : %d", n);
        return (-1);
    }

    if (l < 0 || l > n) l = n;

    int LLength = 21 * n + 6;

    if (l == 0) {
        double scale = 1.0;
        if (normal) {
            for (int i = l; i < n; i++) scale /= (sqrt(2.0) * sqrt(2.0) * sqrt(2.0));
        }
        for (size_t k = 0; k < L[5]; k++) {
            for (size_t j = 0; j < L[4]; j++) {
                for (size_t i = 0; i < L[3]; i++) { *sigOut++ = scale * *C++; }
            }
        }
        return (0);
    }

    const T *cLLL = C;
    const T *cLLH = cLLL + L[0] * L[1] * L[2];
    const T *cLHL = cLLH + L[3] * L[4] * L[5];
    const T *cLHH = cLHL + L[6] * L[7] * L[8];
    const T *cHLL = cLHH + L[9] * L[10] * L[11];
    const T *cHLH = cHLL + L[12] * L[13] * L[14];
    const T *cHHL = cHLH + L[15] * L[16] * L[17];
    const T *cHHH = cHHL + L[18] * L[19] * L[20];

    size_t L3d[27] = {L[0], L[1], L[2], L[3], L[4], L[5], L[6], L[7], L[8], L[9], L[10], L[11], L[12], L[13], L[14], L[15], L[16], L[17], L[18], L[19], L[20], L[21], L[22], L[23], 0, 0, 0};

    for (int i = 1; i <= l; i++) {
        L3d[24] = mww->approxlength(L[LLength - 3], n - i);
        L3d[25] = mww->approxlength(L[LLength - 2], n - i);
        L3d[26] = mww->approxlength(L[LLength - 1], n - i);

        int rc = mww->idwt3d(cLLL, cLLH, cLHL, cLHH, cHLL, cHLH, cHHL, cHHH, L3d, sigOut);

        if (rc < 0) return (rc);
        if (i == l) break;

        cLLL = sigOut;

        cLLH += (L[21 * (i - 1) + 3] * L[21 * (i - 1) + 4] * L[21 * (i - 1) + 5]) + (L[21 * (i - 1) + 6] * L[21 * (i - 1) + 7] * L[21 * (i - 1) + 8])
              + (L[21 * (i - 1) + 9] * L[21 * (i - 1) + 10] * L[21 * (i - 1) + 11]) + (L[21 * (i - 1) + 12] * L[21 * (i - 1) + 13] * L[21 * (i - 1) + 14])
              + (L[21 * (i - 1) + 15] * L[21 * (i - 1) + 16] * L[21 * (i - 1) + 17]) + (L[21 * (i - 1) + 18] * L[21 * (i - 1) + 19] * L[21 * (i - 1) + 20])
              + (L[21 * (i - 1) + 21] * L[21 * (i - 1) + 22] * L[21 * (i - 1) + 23]);

        cLHL = cLLH + (L[21 * i + 3] * L[21 * i + 4] * L[21 * i + 5]);
        cLHH = cLHL + (L[21 * i + 6] * L[21 * i + 7] * L[21 * i + 8]);
        cHLL = cLHH + (L[21 * i + 9] * L[21 * i + 10] * L[21 * i + 11]);
        cHLH = cHLL + (L[21 * i + 12] * L[21 * i + 13] * L[21 * i + 14]);
        cHHL = cHLH + (L[21 * i + 15] * L[21 * i + 16] * L[21 * i + 17]);
        cHHH = cHHL + (L[21 * i + 18] * L[21 * i + 19] * L[21 * i + 20]);

        L3d[0] = L3d[24];
        L3d[1] = L3d[25];
        L3d[2] = L3d[26];

        L3d[3] = L[21 * i + 3];
        L3d[4] = L[21 * i + 4];
        L3d[5] = L[21 * i + 5];
        L3d[6] = L[21 * i + 6];
        L3d[7] = L[21 * i + 7];
        L3d[8] = L[21 * i + 8];
        L3d[9] = L[21 * i + 9];
        L3d[10] = L[21 * i + 10];
        L3d[11] = L[21 * i + 11];
        L3d[12] = L[21 * i + 12];
        L3d[13] = L[21 * i + 13];
        L3d[14] = L[21 * i + 14];
        L3d[15] = L[21 * i + 15];
        L3d[16] = L[21 * i + 16];
        L3d[17] = L[21 * i + 17];
        L3d[18] = L[21 * i + 18];
        L3d[19] = L[21 * i + 19];
        L3d[20] = L[21 * i + 20];
        L3d[21] = L[21 * i + 21];
        L3d[22] = L[21 * i + 22];
        L3d[23] = L[21 * i + 23];
    }

    if (l != n && normal) {
        double scale = 1.0;
        for (int i = l; i < n; i++) scale /= (sqrt(2.0) * sqrt(2.0) * sqrt(2.0));
        for (size_t i = 0; i < L3d[24] * L3d[25] * L3d[26]; i++) sigOut[i] *= scale;
    }

    return (0);
}

int MatWaveWavedec::waverec3(const double *C, const size_t *L, int n, double *sigOut) { return waverec3_template(this, C, L, n, n, false, sigOut); }

int MatWaveWavedec::waverec3(const float *C, const size_t *L, int n, float *sigOut) { return waverec3_template(this, C, L, n, n, false, sigOut); }

int MatWaveWavedec::waverec3(const long *C, const size_t *L, int n, long *sigOut) { return waverec3_template(this, C, L, n, n, false, sigOut); }

int MatWaveWavedec::waverec3(const int *C, const size_t *L, int n, int *sigOut) { return waverec3_template(this, C, L, n, n, false, sigOut); }

int MatWaveWavedec::appcoef3(const double *C, const size_t *L, int n, int l, bool normal, double *sigOut) { return waverec3_template(this, C, L, n, l, normal, sigOut); }

int MatWaveWavedec::appcoef3(const float *C, const size_t *L, int n, int l, bool normal, float *sigOut) { return waverec3_template(this, C, L, n, l, normal, sigOut); }

int MatWaveWavedec::appcoef3(const long *C, const size_t *L, int n, int l, bool normal, long *sigOut) { return waverec3_template(this, C, L, n, l, normal, sigOut); }

int MatWaveWavedec::appcoef3(const int *C, const size_t *L, int n, int l, bool normal, int *sigOut) { return waverec3_template(this, C, L, n, l, normal, sigOut); }

void MatWaveWavedec::computeL(size_t sigInLen, int n, size_t *L) const
{
    L[n + 1] = sigInLen;
    L[n] = sigInLen;
    for (int i = n; i > 0; i--) {
        L[i - 1] = MatWaveBase::approxlength(L[i]);
        L[i] = MatWaveBase::detaillength(L[i]);
    }
}

void MatWaveWavedec::computeL2(size_t sigInX, size_t sigInY, int n, size_t *L) const
{
    L[(n * 6) + 4 - 4] = sigInX;
    L[(n * 6) + 4 - 3] = sigInY;
    L[(n * 6) + 4 - 2] = sigInX;
    L[(n * 6) + 4 - 1] = sigInY;

    for (int i = n; i > 0; i--) {
        // cA
        L[(i * 6) - 6] = MatWaveBase::approxlength(L[i * 6 + 0]);
        L[(i * 6) - 5] = MatWaveBase::approxlength(L[i * 6 + 1]);

        // cDh
        L[(i * 6) - 4] = MatWaveBase::approxlength(L[i * 6 + 0]);
        L[(i * 6) - 3] = MatWaveBase::detaillength(L[i * 6 + 1]);

        // cDv
        L[(i * 6) - 2] = MatWaveBase::detaillength(L[i * 6 + 0]);
        L[(i * 6) - 1] = MatWaveBase::approxlength(L[i * 6 + 1]);

        // cDd - overwrites previous value
        //
        L[(i * 6) - 0] = MatWaveBase::detaillength(L[i * 6 + 0]);
        L[(i * 6) + 1] = MatWaveBase::detaillength(L[i * 6 + 1]);
    }
}

void MatWaveWavedec::computeL3(size_t sigInX, size_t sigInY, size_t sigInZ, int n, size_t *L) const
{
    L[(n * 21) + 6 - 6] = sigInX;
    L[(n * 21) + 6 - 5] = sigInY;
    L[(n * 21) + 6 - 4] = sigInZ;
    L[(n * 21) + 6 - 3] = sigInX;
    L[(n * 21) + 6 - 2] = sigInY;
    L[(n * 21) + 6 - 1] = sigInZ;

    for (int i = n; i > 0; i--) {
        // cLLL
        L[(i * 21) - 21] = MatWaveBase::approxlength(L[i * 21 + 0]);
        L[(i * 21) - 20] = MatWaveBase::approxlength(L[i * 21 + 1]);
        L[(i * 21) - 19] = MatWaveBase::approxlength(L[i * 21 + 2]);

        // cLLH
        L[(i * 21) - 18] = MatWaveBase::approxlength(L[i * 21 + 0]);
        L[(i * 21) - 17] = MatWaveBase::approxlength(L[i * 21 + 1]);
        L[(i * 21) - 16] = MatWaveBase::detaillength(L[i * 21 + 2]);

        // cLHL
        L[(i * 21) - 15] = MatWaveBase::approxlength(L[i * 21 + 0]);
        L[(i * 21) - 14] = MatWaveBase::detaillength(L[i * 21 + 1]);
        L[(i * 21) - 13] = MatWaveBase::approxlength(L[i * 21 + 2]);

        // cLHH
        L[(i * 21) - 12] = MatWaveBase::approxlength(L[i * 21 + 0]);
        L[(i * 21) - 11] = MatWaveBase::detaillength(L[i * 21 + 1]);
        L[(i * 21) - 10] = MatWaveBase::detaillength(L[i * 21 + 2]);

        // cHLL
        L[(i * 21) - 9] = MatWaveBase::detaillength(L[i * 21 + 0]);
        L[(i * 21) - 8] = MatWaveBase::approxlength(L[i * 21 + 1]);
        L[(i * 21) - 7] = MatWaveBase::approxlength(L[i * 21 + 2]);

        // cHLH
        L[(i * 21) - 6] = MatWaveBase::detaillength(L[i * 21 + 0]);
        L[(i * 21) - 5] = MatWaveBase::approxlength(L[i * 21 + 1]);
        L[(i * 21) - 4] = MatWaveBase::detaillength(L[i * 21 + 2]);

        // cHHL
        L[(i * 21) - 3] = MatWaveBase::detaillength(L[i * 21 + 0]);
        L[(i * 21) - 2] = MatWaveBase::detaillength(L[i * 21 + 1]);
        L[(i * 21) - 1] = MatWaveBase::approxlength(L[i * 21 + 2]);

        // cHHH - overwrites previous value
        //
        L[(i * 21) + 0] = MatWaveBase::detaillength(L[i * 21 + 0]);
        L[(i * 21) + 1] = MatWaveBase::detaillength(L[i * 21 + 1]);
        L[(i * 21) + 2] = MatWaveBase::detaillength(L[i * 21 + 2]);
    }
}

size_t MatWaveWavedec::coefflength(const size_t *L, int n) const
{
    size_t tlength = L[0];    // cA coefficients

    for (int i = 1; i <= n; i++) tlength += L[i];

    return (tlength);
}

size_t MatWaveWavedec::coefflength(size_t sigInLen, int n) const
{
    size_t *L = new size_t[n + 2];
    computeL(sigInLen, n, L);

    size_t tlength = coefflength(L, n);

    delete[] L;
    return (tlength);
}

size_t MatWaveWavedec::approxlength(size_t sigInLen, int n) const
{
    size_t cALen = sigInLen;

    for (int i = 0; i < n; i++) {
        cALen = MatWaveBase::approxlength(cALen);
        if (cALen == 0) return (cALen);
    }
    return (cALen);
}

void MatWaveWavedec::approxlength(const size_t *L, int n, int l, size_t *len) const
{
    if (l > n) l = n;

    int LLength = n + 2;

    *len = approxlength(L[LLength - 1], n - l);
}

void MatWaveWavedec::approxlength2(const size_t *L, int n, int l, size_t *lenx, size_t *leny) const
{
    if (l > n) l = n;

    int LLength = n * 6 + 4;

    *lenx = approxlength(L[LLength - 2], n - l);
    *leny = approxlength(L[LLength - 1], n - l);
}

void MatWaveWavedec::approxlength3(const size_t *L, int n, int l, size_t *lenx, size_t *leny, size_t *lenz) const
{
    if (l > n) l = n;

    int LLength = n * 21 + 6;

    *lenx = approxlength(L[LLength - 3], n - l);
    *leny = approxlength(L[LLength - 2], n - l);
    *lenz = approxlength(L[LLength - 1], n - l);
}

size_t MatWaveWavedec::coefflength2(const size_t *L, int n) const
{
    size_t tlength = (L[0] * L[1]);    // cA coefficients;

    for (int i = 1; i <= n; i++) {
        tlength += L[(i * 6) - 4] * L[(i * 6) - 3];    // cDh
        tlength += L[(i * 6) - 2] * L[(i * 6) - 1];    // cDv
        tlength += L[(i * 6) - 0] * L[(i * 6) + 1];    // cDd
    }
    return (tlength);
}

size_t MatWaveWavedec::coefflength2(size_t sigInX, size_t sigInY, int n) const
{
    size_t *L = new size_t[(n * 6) + 4];
    computeL2(sigInX, sigInY, n, L);

    size_t tlength = coefflength2(L, n);

    delete[] L;
    return (tlength);
}

size_t MatWaveWavedec::coefflength3(const size_t *L, int n) const
{
    size_t tlength = (L[0] * L[1] * L[2]);    // cA coefficients;

    for (int i = 1; i <= n; i++) {
        tlength += L[(i * 21) - 18] * L[(i * 21) - 17] * L[(i * 21) - 16];    // cLLH
        tlength += L[(i * 21) - 15] * L[(i * 21) - 14] * L[(i * 21) - 13];    // cLHL
        tlength += L[(i * 21) - 12] * L[(i * 21) - 11] * L[(i * 21) - 10];    // cLHH
        tlength += L[(i * 21) - 9] * L[(i * 21) - 8] * L[(i * 21) - 7];       // cHLL
        tlength += L[(i * 21) - 6] * L[(i * 21) - 5] * L[(i * 21) - 4];       // cHLH
        tlength += L[(i * 21) - 3] * L[(i * 21) - 2] * L[(i * 21) - 1];       // cHHL
        tlength += L[(i * 21) - 0] * L[(i * 21) + 1] * L[(i * 21) + 2];       // cHHH
    }
    return (tlength);
}

size_t MatWaveWavedec::coefflength3(size_t sigInX, size_t sigInY, size_t sigInZ, int n) const
{
    size_t *L = new size_t[(n * 21) + 6];
    computeL3(sigInX, sigInY, sigInZ, n, L);

    size_t tlength = coefflength3(L, n);

    delete[] L;
    return (tlength);
}
