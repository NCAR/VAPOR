#ifndef GeoTileMercator_h_
#define GeoTileMercator_h_
#ifdef _WINDOWS
#pragma warning(disable : 4251)
#endif
#include "GeoTile.h"
#include <vapor/common.h>
//! \class GeoTileMercator
//!
//! This class derives the GetTile base class to provide a tiled world
//! mapping system using the Web "Pseudo-Mercator" projection.
//!
//! The following GDAL commands wered used to transform the NASA blue
//! marble image into a suitable Mercator
//!
//! gdal_translate -of GTiff -a_srs EPSG:4326 -gcp 0 0 -180 90
//! -gcp 4096 0 180 90 -gcp 4096 2048 180 -90 BBM.4096.png bluemarble1.tif
//!
//! gdalwarp -t_srs EPSG:4326 bluemarble1 bluemarble2.tif
//!
//! gdalwarp -s_srs EPSG:4326 -t_srs EPSG:3857 -r bilinear
//! -ts 4096 4096 -te -20037508.34 -20037508.34 20037508.34 20037508.34
//! bluemarble1.tif bluemarble3.tif
//!
//! \sa https://www.mapbox.com/tilemill/docs/guides/reprojecting-geotiff/
//!
//! \author John Clyne
//! \version $Revision$
//! \date    $Date$
//!

namespace VAPoR {
class RENDER_API GeoTileMercator : public GeoTile {
  public:
    //! GeoTileMercator class constructor
    //!
    //! \param[in] tile_height Height of an image tile in pixels. The tile
    //! width must the same as the tile height
    //! \param[in] pixelsize The size of a pixel in bytes
    //!
    //!
    GeoTileMercator(size_t tile_width, size_t tile_height, size_t pixelsize) : GeoTile(
                                                                                   tile_width, tile_height, pixelsize,
                                                                                   -180.0, -85.05112878, 180.0, 85.05112878) {}

    GeoTileMercator(
        size_t tile_width, size_t tile_height, size_t pixelsize,
        double min_lon, double min_lat, double max_lon, double max_lat) : GeoTile(tile_width, tile_height, pixelsize,
                                                                                  min_lon, min_lat, max_lon, max_lat) {}

    //! copydoc GeoTile::LatLongToPixelXY()
    //
    virtual void LatLongToPixelXY(
        double lon, double lat, int lod, size_t &pixelX, size_t &pixelY) const;

    //! copydoc GeoTile::PixelXYToLatLon()
    //
    virtual void PixelXYToLatLon(
        size_t pixelX, size_t pixelY, int lod, double &lon, double &lat) const;
};
}; // namespace VAPoR
#endif
