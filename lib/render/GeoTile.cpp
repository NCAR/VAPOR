#include <iostream>
#include <cstring>
#include <algorithm>
#include <sstream>
#include "vapor/GeoTile.h"

using namespace std;
using namespace VAPoR;

GeoTile::GeoTile(size_t tile_width, size_t tile_height, size_t pixel_size, double min_lon, double min_lat, double max_lon, double max_lat)
{
    _tile_width = tile_width;
    _tile_height = tile_height;
    _pixel_size = pixel_size;
    _tiles.clear();

    _MinLongitude = min_lon;
    _MinLatitude = min_lat;
    _MaxLongitude = max_lon;
    _MaxLatitude = max_lat;
}

GeoTile::~GeoTile()
{
    std::map<string, unsigned char *>::iterator p;

    for (p = _tiles.begin(); p != _tiles.end(); ++p) {
        if (p->second) delete[] p->second;
    }
}

void GeoTile::PixelXYToTileXY(size_t pixelX, size_t pixelY, size_t &tileX, size_t &tileY, size_t &tilePixelX, size_t &tilePixelY) const
{
    tileX = pixelX / _tile_width;
    tileY = pixelY / _tile_height;
    tilePixelX = pixelX % _tile_width;
    tilePixelY = pixelY % _tile_height;
}

void GeoTile::TileXYToPixelXY(size_t tileX, size_t tileY, size_t &pixelX, size_t &pixelY) const
{
    pixelX = tileX * _tile_width;
    pixelY = tileY * _tile_height;
}

int GeoTile::Insert(std::string quadkey, const unsigned char *image)
{
    size_t tileX, tileY;
    int    lod;
    int    rc = QuadKeyToTileXY(quadkey, tileX, tileY, lod);
    if (rc < 0) return (rc);    // invalid quadkey

    std::map<string, unsigned char *>::iterator p = _tiles.find(quadkey);
    unsigned char *                             imgptr;
    if (p != _tiles.end()) {
        imgptr = p->second;    // tile already exists;
    } else {
        imgptr = new unsigned char[_tile_width * _tile_height * _pixel_size];
        _tiles[quadkey] = imgptr;
    }
    memcpy(imgptr, image, _tile_width * _tile_height * _pixel_size);
    return (0);
}

const unsigned char *GeoTile::GetTile(string quadkey) const
{
    map<string, unsigned char *>::const_iterator p = _tiles.find(quadkey);
    if (p != _tiles.end()) {
        return (p->second);    // tile already exists;
    } else {
        return (NULL);
    }
}

int GeoTile::GetMap(size_t pixelX0, size_t pixelY0, size_t pixelX1, size_t pixelY1, int lod, unsigned char *map_image) const
{
    //
    // resolution of global map at requested lod
    //
    size_t nx_global, ny_global;
    MapSize(lod, nx_global, ny_global);

    if (pixelX0 == pixelX1) { pixelX1 = pixelX0 == 0 ? nx_global - 1 : pixelX1 - 1; }
    if (pixelY0 == pixelY1) {    // Needed?
        pixelY1 = pixelY0 == 0 ? ny_global - 1 : pixelY1 - 1;
    }

    //
    // resolution of requested sub region (map). Need to handle periodicity.
    //
    size_t map_width, map_height;
    int    rc = MapSize(pixelX0, pixelY0, pixelX1, pixelY1, lod, map_width, map_height);
    if (rc < 0) return (rc);

    // coordinates of tile containing first (north-west) , and second
    // (south-east) pixel
    //
    size_t tileX0, tileY0;
    size_t tileX1, tileY1;

    // pixel coordinates in  tile coordinate system
    //
    size_t tilePixelX0, tilePixelY0;
    size_t tilePixelX1, tilePixelY1;

    PixelXYToTileXY(pixelX0, pixelY0, tileX0, tileY0, tilePixelX0, tilePixelY0);
    PixelXYToTileXY(pixelX1, pixelY1, tileX1, tileY1, tilePixelX1, tilePixelY1);

    // Deal with wraparound
    //
    size_t ntiles = 1 << lod;
    size_t nxtiles, nytiles;
    if (pixelX0 < pixelX1) {
        nxtiles = tileX1 - tileX0 + 1;
    } else {
        nxtiles = (tileX1 == tileX0) ? ntiles + 1 : ntiles - (tileX0 - tileX1 - 1);
    }

    if (pixelY0 < pixelY1) {
        nytiles = tileY1 - tileY0 + 1;
    } else {
        nytiles = (tileY1 == tileY0) ? ntiles + 1 : ntiles - (tileY0 - tileY1 - 1);
    }

    size_t tpx0 = 0, tpy0 = 0;    // pixel coord bounds in tile coordinate system
    size_t tpx1 = 0, tpy1 = 0;

    // Process tiles from north-west to south-east.
    //
    // N.B. because of periodicity tile{X,Y}0 may be >= tile{X,Y}1
    //
    size_t map_py = 0;
    size_t map_px = 0;

    //
    // p{x,y}0 and p{x,y}1 define bounds of tile within map in global coords
    //
    size_t px0, py0;
    size_t px1, py1;

    size_t ty = tileY0;
    for (int j = 0; j < nytiles; j++) {
        map_px = 0;
        size_t tx = tileX0;

        size_t dummy;
        TileXYToPixelXY(tx, ty, dummy, py0);
        py1 = py0 + _tile_height - 1;

        // Handle boundary tiles
        //
        tpy0 = 0;
        tpy1 = _tile_height - 1;
        if (j == 0 && py0 < pixelY0) {
            size_t offset = pixelY0 - py0;
            py0 += offset;
            tpy0 += offset;
        }
        if (j == nytiles - 1 && py1 > pixelY1) {
            size_t offset = py1 - pixelY1;
            py1 -= offset;
            tpy1 -= offset;
        }

        for (int i = 0; i < nxtiles; i++) {
            const unsigned char *tile = GetTile(tx, ty, lod);
            if (!tile) return (-1);    // tile doesn't exist at this lod

            TileXYToPixelXY(tx, ty, px0, dummy);
            px1 = px0 + _tile_width - 1;

            // Handle boundary tiles
            //
            tpx0 = 0;
            tpx1 = _tile_width - 1;
            if (i == 0 && px0 < pixelX0) {
                size_t offset = pixelX0 - px0;
                px0 += offset;
                tpx0 += offset;
            }
            if (i == nxtiles - 1 && px1 > pixelX1) {
                size_t offset = px1 - pixelX1;
                px1 -= offset;
                tpx1 -= offset;
            }

            _CopyTileToMap(tile, tpx0, tpy0, tpx1, tpy1, map_image, map_px, map_py, map_width, map_height);

            map_px += tpx1 - tpx0 + 1;
            tx = (tx + 1) % ntiles;
        }
        map_py += tpy1 - tpy0 + 1;
        ty = (ty + 1) % ntiles;
    }
    return (0);
}

string GeoTile::TileXYToQuadKey(size_t tileX, size_t tileY, int lod)
{
    string quadKey = "";    // lod == 0
    for (int i = lod; i > 0; i--) {
        ostringstream oss;
        char          digit = '0';
        size_t        mask = 1 << (i - 1);
        if ((tileX & mask) != 0) { digit++; }
        if ((tileY & mask) != 0) {
            digit++;
            digit++;
        }
        oss << digit;
        quadKey.append(oss.str());
    }
    return (quadKey);
}

int GeoTile::QuadKeyToTileXY(string quadKey, size_t &tileX, size_t &tileY, int &lod)
{
    tileX = tileY = 0;
    lod = quadKey.length();
    for (int i = lod; i > 0; i--) {
        int mask = 1 << (i - 1);
        switch (quadKey[lod - i]) {
        case '0': break;

        case '1': tileX |= mask; break;

        case '2': tileY |= mask; break;

        case '3':
            tileX |= mask;
            tileY |= mask;
            break;

        default: return (-1);    // Invalid QuadKey digit sequence
        }
    }
    return (0);
}

void GeoTile::MapSize(int lod, size_t &nx, size_t &ny) const
{
    nx = _tile_width << lod;
    ny = _tile_height << lod;
}

int GeoTile::MapSize(size_t pixelX0, size_t pixelY0, size_t pixelX1, size_t pixelY1, int lod, size_t &nx, size_t &ny) const
{
    size_t nx_global, ny_global;
    MapSize(lod, nx_global, ny_global);

    if (pixelX0 == pixelX1) { pixelX1 = pixelX0 == 0 ? nx_global - 1 : pixelX1 - 1; }

    if (pixelX0 > nx_global - 1) return (-1);
    if (pixelY0 > ny_global - 1) return (-1);
    if (pixelX1 > nx_global - 1) return (-1);
    if (pixelY1 > ny_global - 1) return (-1);

    if (pixelY0 > pixelY1) return (-1);

    //
    // resolution of requested sub region (map). Need to handle periodicity.
    //
    if (pixelX1 > pixelX0) {
        nx = pixelX1 - pixelX0 + 1;
    } else {
        nx = nx_global - (pixelX0 - pixelX1 - 1);    // map wraps around lon
    }
    ny = pixelY1 - pixelY0 + 1;
    return (0);
}

double GeoTile::_Clip(double n, double minValue, double maxValue) const { return std::min(std::max(n, minValue), maxValue); }

void GeoTile::_CopyTileToMap(const unsigned char *tile, size_t tilePixelX0, size_t tilePixelY0, size_t tilePixelX1, size_t tilePixelY1, unsigned char *map, size_t pixelX0, size_t pixelY0, size_t nx,
                             size_t ny) const
{
    size_t px = pixelX0;
    size_t py = pixelY0;

    unsigned char *      mapptr = map;
    const unsigned char *tileptr = tile;
    for (size_t tpy = tilePixelY0; tpy <= tilePixelY1; tpy++, py++) {
        mapptr = map + (py * nx + pixelX0) * _pixel_size;
        tileptr = tile + (tpy * _tile_width + tilePixelX0) * _pixel_size;

        for (size_t tpx = tilePixelX0; tpx <= tilePixelX1; tpx++, px++) {
            for (int i = 0; i < _pixel_size; i++) { mapptr[i] = tileptr[i]; }

            mapptr += _pixel_size;
            tileptr += _pixel_size;
        }
    }
}
