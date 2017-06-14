
#ifndef _GeoImageTMS_h_
#define _GeoImageTMS_h_

#include <xtiffio.h>
#include <geotiff.h>
#include <vapor/MyBase.h>
#include <vapor/UDUnitsClass.h>
#include "GeoTileMercator.h"
#include "GeoImage.h"

namespace VAPoR {

//! \class GeoImageTMS
//! \brief A class for managing OSGeo Tile Map Service Specification images
//! \author John Clyne
//!
//
class RENDER_API GeoImageTMS : public GeoImage {
  public:
    GeoImageTMS();
    virtual ~GeoImageTMS();

    int Initialize(string path, vector<double> times);

    unsigned char *GetImage(size_t ts, size_t &width, size_t &height);

    unsigned char *GetImage(
        size_t ts, const double pcsExtentsReq[4], string proj4StringReq,
        size_t maxWidthReq, size_t maxHeightReq,
        double pcsExtentsImg[4], double geoCornersImg[8], string &proj4StringImg,
        size_t &width, size_t &height);

  private:
    string _dir; // path to TMS directory
    int _maxLOD; // Maximum LOD available in TMS

    unsigned char *_texture; // storage for texture image
    size_t _textureSize;

    unsigned char *_tileBuf; // storage for a single tile
    size_t _tileBufSize;

    GeoTileMercator *_geotile;

    string _defaultProj4String; // proj4 string for global mercator

    string _tilePath(string dir, size_t tileX, size_t tileY, int lod) const;

    int _tileSize(
        string dir, size_t tileX, size_t tileY, int lod, size_t &w, size_t &h);

    int _tileRead(
        string dir, size_t tileX, size_t tileY, int lod, unsigned char *tile);

    int _getBestLOD(
        const double myGeoExtentsData[4], int maxWidthReq, int maxHeightReq) const;

    int _getMap(
        const size_t pixelSW[2], const size_t pixelNE[2],
        int lod, unsigned char *texture);
};

}; // namespace VAPoR
#endif
