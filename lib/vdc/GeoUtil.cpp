#include <cmath>
#include <iostream>
#include <vapor/GeoUtil.h>

using namespace std;
using namespace VAPoR;

namespace {

template<class T> void _minmax(const T *a, int n, int stride, T &min, T &max)
{
    min = max = a[0];

    for (int i = 0; i < n; i++) {
        if (a[i * stride] < min) min = a[i * stride];
        if (a[i * stride] > max) max = a[i * stride];
    }
}
//
// Shift 1D array of longitudes, if needed, such that values
// are in the range [-360.0..360.0)
//
template<class ForwardIt> void shiftLonTemplate(ForwardIt first, ForwardIt last)
{
    auto min = *(std::min_element(first, last));
    auto max = *(std::max_element(first, last));

    if (max > 360.0) {
        double delta = 0.0;
        while (max > 360.0) {
            max -= 360.0;
            delta += 360.0;
        }
        for (auto itr = first; itr != last; ++itr) { *itr -= delta; }
    }
    if (min < -360.0) {
        double delta = 0.0;
        while (min < -360.0) {
            min += 360.0;
            delta -= 360.0;
        }
        for (auto itr = first; itr != last; ++itr) { *itr -= delta; }
    }
}

//
// Make longitude values monotonically increasing
//
template<class ForwardIt> void unwrapLongitudeTemplate(ForwardIt first, ForwardIt last)
{
    auto itr = first;
    if (itr == last) return;

    auto startValue = *itr;
    ++itr;

    for (; itr != last; ++itr) {
        while (*itr < startValue) { *itr += 360.0; }
    }
}



template<class T> void _ExtractBoundaryTemplate(const T *a, int nx, int ny, T *bdry)
{
    T *bdryptr = bdry;

    for (int i = 0; i < nx; i++) *bdryptr++ = a[i];
    for (int j = 1; j < ny; j++) *bdryptr++ = a[nx * j + nx - 1];
    for (int i = nx - 2; i >= 0; i--) *bdryptr++ = a[nx * (ny - 1) + i];
    for (int j = ny - 2; j >= 1; j--) *bdryptr++ = a[j * nx];
}

};    // namespace

void GeoUtil::ShiftLon(vector<float>::iterator first, vector<float>::iterator last) { shiftLonTemplate(first, last); }
void GeoUtil::ShiftLon(vector<double>::iterator first, vector<double>::iterator last) { shiftLonTemplate(first, last); }
void GeoUtil::ShiftLon(float *first, float *last) { shiftLonTemplate(first, last); }

void GeoUtil::UnwrapLongitude(vector<float>::iterator first, vector<float>::iterator last) { unwrapLongitudeTemplate(first, last); }
void GeoUtil::UnwrapLongitude(vector<double>::iterator first, vector<double>::iterator last) { unwrapLongitudeTemplate(first, last); }
void GeoUtil::UnwrapLongitude(float *first, float *last) { unwrapLongitudeTemplate(first, last); }


void GeoUtil::ExtractBoundary(const float *a, int nx, int ny, float *bdry) { _ExtractBoundaryTemplate(a, nx, ny, bdry); }

void GeoUtil::ExtractBoundary(const double *a, int nx, int ny, double *bdry) { _ExtractBoundaryTemplate(a, nx, ny, bdry); }
