#include "vapor/GeoTileEquirectangular.h"
using namespace VAPoR;

void GeoTileEquirectangular::LatLongToPixelXY(
    double lon, double lat, int lod, size_t &pixelX, size_t &pixelY) const {
    while (lon < _MinLongitude)
        lon += 360.0;
    while (lon > _MaxLongitude)
        lon -= 360.0;

    double x = (lon + 180) / 360.0;
    double y = (lat + 90) / 180.0;

    size_t nx, ny;
    MapSize(lod, nx, ny);

    pixelX = (int)_Clip(x * nx + 0.5, 0, nx - 1);
    pixelY = (int)_Clip(y * ny + 0.5, 0, ny - 1);
}

void GeoTileEquirectangular::PixelXYToLatLon(
    size_t pixelX, size_t pixelY, int lod, double &lon, double &lat) const {

    size_t nx, ny;
    MapSize(lod, nx, ny);

    double x = (_Clip(pixelX, 0, nx - 1) / (double)nx) - 0.5;
    double y = 0.5 - (_Clip(pixelY, 0, ny - 1) / (double)ny);

    lat = 180 * y;
    lon = 360 * x;
}
