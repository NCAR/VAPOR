#ifndef GeoTileEquirectangular_h_
#define GeoTileEquirectangular_h_
#ifdef _WINDOWS
#pragma warning(disable : 4251)
#endif
#include "GeoTile.h"
#include <vapor/common.h>
namespace VAPoR {
//! \class GeoTileEquirectangular
//!
//! This class derives the GetTile base class to provide a tiled world
//! mapping system using the Equirectangular projection.
//!
//! \author John Clyne
//! \version $Revision$
//! \date    $Date$
//!

class RENDER_API GeoTileEquirectangular : public GeoTile {
  public:
    //! GeoTileEquirectangular class constructor
    //!
    //! \param[in] tile_height Height of an image tile in pixels. The tile
    //! width must be twice the tile height
    //! \param[in] pixelsize The size of a pixel in bytes
    //!
    //!
    GeoTileEquirectangular(size_t tile_width, size_t tile_height, size_t pixelsize) : GeoTile(
                                                                                          tile_width, tile_height, pixelsize, -180.0, -90.0, 180.0, 90.0) {}

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
