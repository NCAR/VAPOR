
#ifndef _GeoImageTMS_h_
#define _GeoImageTMS_h_

#ifdef WIN32
#include <geotiff/xtiffio.h>
#include <geotiff/geotiff.h>
#else
#include <xtiffio.h>
#include <geotiff.h>
#endif
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <vapor/MyBase.h>
#include <vapor/UDUnitsClass.h>
#include "GeoTileMercator.h"
#include "GeoImage.h"

namespace VAPoR {

static inline bool IsTMSFile( 
    std::string path 
) {
    if ( path.rfind(".tms", path.size()-4) != string::npos ) {
        ifstream in;
        in.open( path.c_str() );
        if ( ! in ) {
            GeoImage::SetErrMsg("fopen(%s) : %M", path.c_str());
            return false ;
        }
        in.close();
    }

    else {
        return false;
    }

    return true;
}

// Get the file path of a single tile from the TMS database
// with the give lod and tile coordinates
//
static inline std::string TilePath(
    string dir, size_t tileX, size_t tileY, int lod
) {

//  size_t ntiles = 1 << lod;
//  size_t tmsTileY = ntiles - 1 - tileY;
    size_t tmsTileY = tileY;

    ostringstream oss;
    oss << dir;
    oss << "/";
    oss << lod;
    oss << "/";
    oss << tileX;
    oss << "/";
    oss << tmsTileY;

    string base = oss.str();

    string path = base + ".tif";

    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) == 0)  return(path);

    path = base + ".tiff";

    if (stat(path.c_str(), &statbuf) == 0) return (path);

    // Tile does not exist
    //
    return("");
}

//! \class GeoImageTMS
//! \brief A class for managing OSGeo Tile Map Service Specification images
//! \author John Clyne
//!
//
class RENDER_API GeoImageTMS : public GeoImage {
public:

 GeoImageTMS();
 virtual ~GeoImageTMS();

 int Initialize(string path, vector <double> times);

 unsigned char *GetImage(size_t ts, size_t &width, size_t &height);

 unsigned char *GetImage(
	size_t ts, const double pcsExtentsReq[4], string proj4StringReq,
	size_t maxWidthReq, size_t maxHeightReq,
	double pcsExtentsImg[4], double geoCornersImg[8], string &proj4StringImg,
	size_t &width, size_t &height
 );


private:

 string _dir;	// path to TMS directory
 int _maxLOD;	// Maximum LOD available in TMS
 
 unsigned char *_texture;	// storage for texture image
 size_t _textureSize;

 unsigned char *_tileBuf;	// storage for a single tile
 size_t _tileBufSize;

 GeoTileMercator *_geotile;

 string _defaultProj4String;	// proj4 string for global mercator

 //string _tilePath(string dir, size_t tileX, size_t tileY, int lod) const;

 int _tileSize(
	string dir, size_t tileX, size_t tileY, int lod, size_t &w, size_t &h
 ); 

 int _tileRead(
    string dir, size_t tileX, size_t tileY, int lod, unsigned char *tile
 );

 int _getBestLOD(
	const double myGeoExtentsData[4], int maxWidthReq, int maxHeightReq
 ) const;

 int _getMap(
	const size_t pixelSW[2], const size_t pixelNE[2], 
	int lod, unsigned char *texture
 ) ;

};

};
#endif
