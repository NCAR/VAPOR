#ifndef	GeoTile_h_
#define GeoTile_h_

#include <string>
#include <map>
#ifdef _WINDOWS
#pragma warning(disable : 4251)
#endif
//
//! \class GeoTile
//!
//! \brief An abstract class for managing geo-referenced world maps, 
//! decomposed into tiles, and supporting level-of-detail refinement.
//!
//! This representation is based on Microsoft's Virtual Earth Tile System
//! (Bing Maps), described here 
//! \sa http://msdn.microsoft.com/en-us/library/bb259689
//!
//! The image tiles are also compatible with the tiling system used by
//! Google Maps and the open source Tile Map Service, differing only 
//! in how the tiles are indexed. Microsoft (and this class) use
//! "quadkeys", whereas, Google and TMS employ different indexing/naming
//! conventions for tiles. The encoding of quadkeys in this implementation
//! differs from Microsoft's in one respect: a quadkey == "" (empty string)
//! is a valid key, coresponding to level-of-detail zero, a single tile
//! covering the entire globe. I.e. the coarsest lod permitted by Microsoft is
//! level one, and coresponds to a four-tile decomposition of the global
//! map projection. A single tile, level-zero, map is permitted by this class.
//!
//! The above world map tiling systems all employ a Mercator projection.
//! No projection is specified by this base class. The projection type
//! is left to derived classes.
//!
//! \note Much of this code is based on the sample representation provided
//! by Microsoft.
//!
//! \author John Clyne
//! \version $Revision$
//! \date    $Date$
//!
//!
#include <vapor/common.h>
#ifdef WINDOWS
#pragma warning(disable : 4251)
#endif
namespace VAPoR {
class RENDER_API GeoTile {

public:

 //! Create a GeoTile object
 //!
 //! \param[in] tile_width The width of a tile in pixels
 //! \param[in] tile_height The height of a tile in pixels
 //! \param[in] pixelsize The size of a pixel in bytes
 //! \param[in] min_lon The minimum valid longitude for the map projection
 //! \param[in] min_lat The minimum valid lattitude for the map projection
 //! \param[in] max_lon The maximum valid longitude for the map projection
 //! \param[in] max_lat The maximum valid lattitude for the map projection
 //!
 GeoTile(
	size_t tile_width, size_t tile_height, size_t pixelsize,
	double min_lon, double min_lat, double max_lon, double max_lat
 );
 virtual ~GeoTile();

 //! Insert an image tile into the class object
 //!
 //! This method is used to add geo-referenced image tiles to the class.
 //!
 //! \param[in] quadkey A string encoding the location (index) and level
 //! of detail of \p image
 //! \param[in] image A pointer to the image tile to be copied into 
 //! the object class. The size of the image tile must be \p tile_width *
 //! \p tile_height * \p pixelsize
 //!
 //! \sa GeoTile()
 //
 int Insert(std::string quadkey, const unsigned char *image);

 //! Converts a point from latitude/longitude WGS-84 coordinates (in degrees)
 //! into pixel XY coordinates at a specified level of detail.
 //!
 //! \param[in] lon Longitude of the point, in degrees.
 //! \param[in] lat Latitude of the point, in degrees.
 //! \param[in] lod Level of detail, from 0 (lowest detail)
 //! to 23 (highest detail).
 //! \param[out] pixelX Output parameter receiving the X coordinate in pixels.
 //! \param[out] pixelY Output parameter receiving the Y coordinate in pixels.
 //!
 //! \sa PixelXYToLatLong()
 //
 virtual void LatLongToPixelXY(
	double lon, double lat, int lod, size_t &pixelX, size_t &pixelY
 ) const = 0;

 //! Converts a pixel from pixel XY coordinates at a specified level of detail
 //! into latitude/longitude WGS-84 coordinates (in degrees).
 //!
 //! \param [in] pixelX X coordinate of the point, in pixels.
 //! \param [in] pixelY Y coordinates of the point, in pixels.
 //! \param [in] lod Level of detail, from 0 (lowest detail)
 //! to 23 (highest detail).
 //! \param [out] lat Output parameter receiving the latitude in degrees.
 //! \param [out] lon Output parameter receiving the longitude in degrees
 //!
 //! \sa LatLongToPixelXY()
 //!
 virtual void PixelXYToLatLon(
	size_t pixelX, size_t pixelY, int lod, double &lon, double &lat
 ) const = 0;

 //! Converts pixel XY coordinates into tile XY coordinates of the tile 
 //! containing the specified pixel.
 //!
 //! Given a map coordinate in pixels, this method returns the index of
 //! of the tile containing the pixel, as well as the pixel's location
 //! within the tile coordinate system.
 //!
 //! \param [in] pixelX Pixel X coordinate.
 //! \param [in] pixelY Pixel Y coordinate.
 //! \param [in] tileX Output parameter receiving the tile's X coordinate.
 //! \param [in] tileY Output parameter receiving the tile's Y coordinate.
 //! \param [in] tilePixelX Output parameter receiving the pixel's X 
 //! in the tile's local coordinate system.
 //! \param [in] tilePixelY Output parameter receiving the pixel's Y 
 //! in the tile's local coordinate system.
 //!
 void PixelXYToTileXY(
	size_t pixelX, size_t pixelY,
	size_t &tileX, size_t &tileY, size_t &tilePixelX, size_t &tilePixelY
 ) const;


 //! Converts tile XY coordinates into pixel XY coordinates of the 
 //! upper-left pixel of the specified tile.
 //!
 //! \param [in] tileX Tile X coordinate.
 //! \param [in] tileY Tile Y coordinate.
 //! \param [out] pixelX Output parameter receiving the pixel X coordinate.
 //! \param [out] pixelY Output parameter receiving the pixel Y coordinate.
 //!
 void TileXYToPixelXY(
	size_t tileX, size_t tileY, size_t &pixelX, size_t &pixelY
 ) const;

 //! Return a pointer to the image tile associated with a quadkey
 //!
 //! This method returns a pointer to the image tile associated with
 //! the Quad Key \p quadkey. If no image tile has been added to the 
 //! class for \p quadkey, a null pointer is returned.
 //!
 //! \param[in] quadkey A Quad Key
 //! \retval tile If the tile associated with \p quadkey exists, a pointer
 //! to it is returned. If the tile does not exist (or if \p quadkey is
 //! not a valid key), the NULL pointer is returned
 //!
 //! \sa Insert()
 //
 const unsigned char *GetTile(std::string quadkey) const;

 //! Return a pointer to an image tile 
 //!
 //! This method returns a pointer to the image tile associated with
 //! a tile index and level of detail.
 //! If no image tile has been added to the 
 //! class for combination of tile index and lod, a null pointer is returned.
 //!
 //! \param[in] tileX X coordinate of tile
 //! \param[in] tileY X coordinate of tile
 //! \param[in] lod level of detail of tile
 //! \retval tile If the tile associated with \p tileX, \p tileY, and
 //! \p lod, a pointer
 //! to it is returned. If the tile does not exist (or if \p quadkey is
 //! not a valid key), the NULL pointer is returned
 //!
 //! \sa Insert()
 //
 const unsigned char *GetTile(size_t tileX, size_t tileY, int lod) const {
	std::string quadkey =  TileXYToQuadKey(tileX, tileY, lod);
	return(GetTile(quadkey));
 }

 //! This method contructs a continuous (non-tiled) map from the 
 //! tiles contained within the class.
 //! 
 //! Given a rectangular boundary described to two points (the north-west and
 //! south-east corner) in pixel coordinates, and a level of detail, this 
 //! method constructs and returns a continuous map.
 //!
 //! \param[in] pixelX0 X coordinate of north-west corner in pixel coordinates
 //! \param[in] pixelY0 Y coordinate of north-west corner in pixel coordinates
 //! \param[in] pixelX0 X coordinate of south-east corner in pixel coordinates
 //! \param[in] pixelY0 Y coordinate of south-east corner in pixel coordinates
 //! \param[in] lod Level of detail
 //! \param[out] map_image The constructed map is copied to the location
 //! pointed to by \p map_image. The memory referenced by \p map_image
 //! must of sufficient size to accommodate the map.
 //!
 //! \retval status A zero is returned on success. If the coordinates
 //! are invalid, or if all of the tiles needed are not present, a negative
 //! value is returned.
 //!
 //! \sa MapSize(), Insert(), LatLongToPixelXY().
 //
 int GetMap(
	size_t pixelX0, size_t pixelY0, size_t pixelX1, size_t pixelY1, 
	int lod, unsigned char *map_image) const;

 //! Converts tile XY coordinates into a QuadKey at a specified level of detail.
 //!
 //! \param[in] tileX Tile X coordinate.
 //! \param[in] tileY Tile Y coordinate.
 //! \param[in] lod Level of detail, from 0 (lowest detail)
 //! to 23 (highest detail).
 //!
 //! \retval quadkey A string containing the QuadKey.
 //
 static std::string TileXYToQuadKey(size_t tileX, size_t tileY, int lod);


 //! Converts a QuadKey into tile XY coordinates.
 //!
 //! \param[in] quadKey QuadKey of the tile.
 //! \param[out] tileX Output parameter receiving the tile X coordinate.
 //! \param[out] tileY Output parameter receiving the tile Y coordinate.
 //! \param[out] lod Output parameter receiving the level of detail.
 //!
 //! \retval status A zero is returned on success, otherwise a negative
 //! value is returned if the supplied inputs are invalid
 //!
 static int QuadKeyToTileXY(
	std::string quadKey, size_t &tileX, size_t &tileY, int &lod
 );

 //! Compute the size of the global map in pixels at a specified lod
 //!
 //! This method calculates the width and height, in pixels, of the global
 //! map at a specified level of detail
 //!
 //! \param[in] lod The level of detail
 //! \param[out] nx Width of the map in pixels
 //! \param[out] ny Height of the map in pixels
 //
 void MapSize(int lod, size_t &nx, size_t &ny) const;

 //! Compute the size of a map in pixels at a specified lod and coordinates
 //!
 //! Given a rectangular boundary described to two points (the north-west and
 //! south-east corner) in pixel coordinates, and a level of detail, this 
 //! method computes the size of the map (width and height) in pixels.
 //!
 //! \param[in] pixelX0 X coordinate of north-west corner in pixel coordinates
 //! \param[in] pixelY0 Y coordinate of north-west corner in pixel coordinates
 //! \param[in] pixelX0 X coordinate of south-east corner in pixel coordinates
 //! \param[in] pixelY0 Y coordinate of south-east corner in pixel coordinates
 //! \param[in] lod The level of detail
 //! \param[out] nx Width of the map in pixels
 //! \param[out] ny Height of the map in pixels
 //
 int MapSize(
	size_t pixelX0, size_t pixelY0, size_t pixelX1, size_t pixelY1,
	int lod, size_t &nx, size_t &ny
 ) const;

 void GetLatLonExtents(
	double &minlon, double &minlat, double &maxlon, double &maxlat
 ) const {
	minlon = _MinLongitude;
	minlat = _MinLatitude;
	maxlon = _MaxLongitude;
	maxlat = _MaxLatitude;
 }

 void GetTileSize(size_t &w, size_t &h) const {
	w = _tile_width;
	h = _tile_height;
 }

private:
 size_t _pixel_size;
 std::map <std::string, unsigned char *> _tiles;

 void _CopyTileToMap(
	const unsigned char *tile, size_t tilePixelX0, size_t tilePixelY0,
	size_t tilePixelX1, size_t tilePixelY1,
	unsigned char *map, size_t pixelX0, size_t pixelX1, size_t nx, size_t ny
 ) const;

protected:
 size_t _tile_width;
 size_t _tile_height;

 double _MinLatitude;
 double _MaxLatitude;
 double _MinLongitude;
 double _MaxLongitude;

 double _Clip(double n, double minValue, double maxValue) const;


};
};
#endif
