#include <cmath>
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
// Shift 1D array of longitudes, if needed, such adjacent values
// do not span 360/0 or -180/180. Shift is performed by adding or
// subtracting 360 as needed. Returned values are in range -360 to 360
//
template<class T> void _ShiftLonTemplate(const T *srclon, int nx, T *dstlon)
{
    dstlon[0] = srclon[0];
    double min = dstlon[0];
    double max = dstlon[0];
    for (int i = 1; i < nx; i++) {
        dstlon[i] = srclon[i];
        if (fabs(dstlon[(i - 1)] - dstlon[i]) > 180.0) {
            if (dstlon[(i - 1)] > dstlon[i])
                dstlon[i] += 360.0;
            else
                dstlon[i] -= 360.0;
        }
        if (dstlon[i] < min) min = dstlon[i];
        if (dstlon[i] > max) max = dstlon[i];
    }
    if (min < -360.0) {
        for (int i = 0; i < nx; i++) dstlon[i] += 360.0;
    }
    if (max > 360.0) {
        for (int i = 0; i < nx; i++) dstlon[i] -= 360.0;
    }
}

template<class T> void _ShiftLonTemplate(const T *srclon, int nx, int ny, T *dstlon)
{
    dstlon[0] = srclon[0];
    double min = dstlon[0];
    double max = dstlon[0];
    for (int j = 0; j < ny; j++) {
        for (int i = 1; i < nx; i++) {
            dstlon[j * nx + i] = srclon[j * nx + i];
            if (fabs(dstlon[j * nx + (i - 1)] - dstlon[j * nx + i]) > 180.0) {
                if (dstlon[j * nx + (i - 1)] > dstlon[j * nx + i])
                    dstlon[j * nx + i] += 360.0;
                else
                    dstlon[j * nx + i] -= 360.0;
            }
            if (dstlon[j * nx + i] < min) min = dstlon[j * nx + i];
            if (dstlon[j * nx + i] > max) max = dstlon[j * nx + i];
        }
    }

    if (min < -360.0) {
        for (int j = 0; j < ny; j++) {
            for (int i = 0; i < nx; i++) dstlon[j * nx + i] += 360.0;
        }
    }
    if (max > 360.0) {
        for (int j = 0; j < ny; j++) {
            for (int i = 0; i < nx; i++) dstlon[j * nx + i] -= 360.0;
        }
    }
}

//
// Template functions for class members to facilitate type overloading
//

template<class T> void _LonExtentsTemplate(const T *lon, int nx, int ny, T &lonwest, T &loneast)
{
    lonwest = loneast = 0.0;

    //
    // south, north, west and east grid longitudes
    //
    T *lonX0 = new T[nx];
    T *lonX1 = new T[nx];
    T *lonY0 = new T[ny];
    T *lonY1 = new T[ny];

    for (int i = 0; i < nx; i++) lonX0[i] = lon[i];
    for (int i = 0; i < nx; i++) lonX1[i] = lon[(nx * (ny - 1)) + i];
    for (int i = 0; i < ny; i++) lonY0[i] = lon[i * nx];
    for (int i = 0; i < ny; i++) lonY1[i] = lon[i * nx + nx - 1];

    //
    // Shift longitudes, if needed, such that adjacent points don't
    // straddle wraparound. (e.g 179, -179 => 179, 181)
    // We need to effectively walk the entire permiter, checking
    // all adjacent values along the perimeter
    //
    _ShiftLonTemplate(lonX0, nx, lonX0);    // south row
    lonY0[0] = lonX0[0];
    lonY1[0] = lonX0[nx - 1];

    _ShiftLonTemplate(lonY0, ny, lonY0);    // east column
    lonX1[0] = lonY0[ny - 1];

    _ShiftLonTemplate(lonY1, ny, lonY1);    // east column
    _ShiftLonTemplate(lonX1, nx, lonX1);    // north row

    //
    // Find the west-most (min) and east-most (max) longitudes
    //
    T min, max;
    lonwest = lonX0[0];
    loneast = lonX0[0];

    _minmax(lonX0, nx, 1, min, max);
    if (min < lonwest) lonwest = min;
    //	if (max > loneast) loneast = max;

    _minmax(lonX1, nx, 1, min, max);
    //	if (min < lonwest) lonwest = min;
    if (max > loneast) loneast = max;

    _minmax(lonY0, ny, 1, min, max);
    if (min < lonwest) lonwest = min;
    //	if (max > loneast) loneast = max;

    _minmax(lonY1, ny, 1, min, max);
    //	if (min < lonwest) lonwest = min;
    if (max > loneast) loneast = max;

    delete[] lonX0;
    delete[] lonX1;
    delete[] lonY0;
    delete[] lonY1;

    // grid wraps around over 360 degrees. Clamp to 360
    //
    //	if (fabs(loneast - lonwest) > 360.0) {
    //		loneast = lonwest + 360.0;
    //	}
}

template<class T> void _LonExtentsTemplate(const T *lon, int nx, T &lonwest, T &loneast)
{
    lonwest = loneast = 0.0;

    T *lonX0 = new T[nx];

    //
    // Shift longitudes, if needed, such that adjacent points don't
    // straddle wraparound. (e.g 179, -179 => 179, 181)
    //
    _ShiftLonTemplate(lon, nx, lonX0);    // south row

    //
    // Find the west-most (min) and east-most (max) longitudes
    //
    T min, max;
    lonwest = lonX0[0];
    loneast = lonX0[0];

    _minmax(lonX0, nx, 1, min, max);
    if (min < lonwest) lonwest = min;
    if (max > loneast) loneast = max;

    delete[] lonX0;

    // grid wraps around over 360 degrees. Clamp to 360
    //
    if (fabs(loneast - lonwest) > 360.0) { loneast = lonwest + 360.0; }
}

template<class T> void _LatExtentsTemplate(const T *lat, int nx, int ny, T &latsouth, T &latnorth)
{
    //
    // Find the south-most (min) and north-most (max) latitudes
    //
    latsouth = lat[0];
    latnorth = lat[0];

    T min, max;
    _minmax(lat, nx, 1, min, max);
    if (min < latsouth) latsouth = min;
    if (max > latnorth) latnorth = max;

    _minmax(lat + (nx * (ny - 1)), nx, 1, min, max);
    if (min < latsouth) latsouth = min;
    if (max > latnorth) latnorth = max;

    _minmax(lat, ny, nx, min, max);
    if (min < latsouth) latsouth = min;
    if (max > latnorth) latnorth = max;

    _minmax(lat + nx - 1, ny, nx, min, max);
    if (min < latsouth) latsouth = min;
    if (max > latnorth) latnorth = max;

    // Sanity check.
    //
    if (latsouth < -90.0) latsouth = -90.0;
    if (latnorth > 90.0) latnorth = 90.0;
}

template<class T> void _LatExtentsTemplate(const T *lat, int ny, T &latsouth, T &latnorth)
{
    //
    // Find the south-most (min) and north-most (max) latitudes
    //
    latsouth = lat[0];
    latnorth = lat[0];

    T min, max;
    _minmax(lat, ny, 1, min, max);
    if (min < latsouth) latsouth = min;
    if (max > latnorth) latnorth = max;

    // Sanity check.
    //
    if (latsouth < -90.0) latsouth = -90.0;
    if (latnorth > 90.0) latnorth = 90.0;
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

void GeoUtil::ShiftLon(const float *srclon, int nx, float *dstlon) { _ShiftLonTemplate(srclon, nx, dstlon); }

void GeoUtil::ShiftLon(const double *srclon, int nx, double *dstlon) { _ShiftLonTemplate(srclon, nx, dstlon); }

void GeoUtil::ShiftLon(const float *srclon, int nx, int ny, float *dstlon) { _ShiftLonTemplate(srclon, nx, ny, dstlon); }

void GeoUtil::ShiftLon(const double *srclon, int nx, int ny, double *dstlon) { _ShiftLonTemplate(srclon, nx, ny, dstlon); }

void GeoUtil::LonExtents(const float *lon, int nx, int ny, float &lonwest, float &loneast) { _LonExtentsTemplate(lon, nx, ny, lonwest, loneast); }

void GeoUtil::LonExtents(const double *lon, int nx, int ny, double &lonwest, double &loneast) { _LonExtentsTemplate(lon, nx, ny, lonwest, loneast); }

void GeoUtil::LonExtents(const float *lon, int nx, float &lonwest, float &loneast) { _LonExtentsTemplate(lon, nx, lonwest, loneast); }

void GeoUtil::LonExtents(const double *lon, int nx, double &lonwest, double &loneast) { _LonExtentsTemplate(lon, nx, lonwest, loneast); }

void GeoUtil::LatExtents(const float *lat, int nx, int ny, float &latsouth, float &latnorth) { _LatExtentsTemplate(lat, nx, ny, latsouth, latnorth); }

void GeoUtil::LatExtents(const double *lat, int nx, int ny, double &latsouth, double &latnorth) { _LatExtentsTemplate(lat, nx, ny, latsouth, latnorth); }

void GeoUtil::LatExtents(const float *lat, int ny, float &latsouth, float &latnorth) { _LatExtentsTemplate(lat, ny, latsouth, latnorth); }

void GeoUtil::LatExtents(const double *lat, int ny, double &latsouth, double &latnorth) { _LatExtentsTemplate(lat, ny, latsouth, latnorth); }

void GeoUtil::ExtractBoundary(const float *a, int nx, int ny, float *bdry) { _ExtractBoundaryTemplate(a, nx, ny, bdry); }

void GeoUtil::ExtractBoundary(const double *a, int nx, int ny, double *bdry) { _ExtractBoundaryTemplate(a, nx, ny, bdry); }
