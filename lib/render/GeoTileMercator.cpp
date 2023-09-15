#ifdef _WINDOWS
    #define _USE_MATH_DEFINES
    #pragma warning(disable : 4251 4100)
#endif
#include <cmath>
#include "vapor/GeoTileMercator.h"
using namespace VAPoR;

void GeoTileMercator::LatLongToPixelXY(double lon, double lat, int lod, size_t &pixelX, size_t &pixelY) const
{
    while (lon < _MinLongitude) lon += 360.0;
    while (lon > _MaxLongitude) lon -= 360.0;

    lat = _Clip(lat, _MinLatitude, _MaxLatitude);
    lon = _Clip(lon, _MinLongitude, _MaxLongitude);

    double x = (lon + 180) / 360.0;

    double sinLat = sin(lat * M_PI / 180.0);
    double y = 0.5 - log((1 + sinLat) / (1 - sinLat)) / (4 * M_PI);

    // Flip Y coordinate. South most point is pixel 0
    //
    y = 1.0 - y;

    size_t nx, ny;
    MapSize(lod, nx, ny);

    pixelX = (int)_Clip(x * nx + 0.5, 0, nx - 1);
    pixelY = (int)_Clip(y * ny + 0.5, 0, ny - 1);
}

void GeoTileMercator::PixelXYToLatLon(size_t pixelX, size_t pixelY, int lod, double &lon, double &lat) const
{
    size_t nx, ny;
    MapSize(lod, nx, ny);

    double x = (_Clip(pixelX, 0, nx - 1) / (double)nx) - 0.5;
    double y = 0.5 - (_Clip(pixelY, 0, ny - 1) / ny);

    lon = 360 * x;
    lat = -1 * (90.0 - 360.0 * atan(exp(-y * 2 * M_PI)) / M_PI);
}
