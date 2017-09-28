
#ifndef _GeoImageGeoTiff_h_
#define _GeoImageGeoTiff_h_

#ifdef WIN32
#include <geotiff/xtiffio.h>
#include <geotiff/geotiff.h>
#else
#include <xtiffio.h>
#include <geotiff.h>
#endif
#include <vapor/MyBase.h>
#include <vapor/UDUnitsClass.h>
#include "GeoImage.h"

namespace VAPoR {

//! \class GeoImageGeoTiff
//! \brief A class for managing GeoTiff images 
//! \author John Clyne
//!
//
class RENDER_API GeoImageGeoTiff : public GeoImage {
public:

 GeoImageGeoTiff();
 virtual ~GeoImageGeoTiff();

 int Initialize(string path, vector <double> times);

 unsigned char *GetImage(size_t ts, size_t &width, size_t &height);

 unsigned char *GetImage(
	size_t ts, const double pcsExtentsReq[4], string proj4StringReq,
	size_t maxWidthReq, size_t maxHeightReq,
	double pcsExtentsImg[4], double geoCornersImg[8], string &proj4StringImg,
	size_t &width, size_t &height
 );


private:
 string _proj4String;
 std::vector <double> _tiffTimes;
 std::vector <double> _times;
 unsigned char *_texture;	// storage for texture image
 size_t _textureSize;	// Texture size. No smart buffers :-(


 string _getProjectionFromGTIF(GTIF *gtifHandle) const;

 void _initTimeVector(TIFF* tif, const vector <double> &times);

 bool _getTiffTime( TIFF *tif, UDUnits *udunits, double &tifftime) const;

 int _getBestDirNum(size_t ts) const;

 int _getGTIFInfo(
	TIFF *tif, size_t width, size_t height,
	double pcsExtents[4], double geoCorners[8], string &proj4String
 ) const;

 bool _extractSubtexture(
	unsigned char *texture, size_t width, size_t height,
	const double pcsExtentsReq[4], string proj4StringReq,
	const double pcsExtentsImg[4], const double geoCornersImg[8],
	string proj4StringImg,
	string &subProj4StringImg, size_t &subWidth, size_t &subHeight,
	double subPCSExtentsImg[4], double subGeoCornersImg[8]
 ) const;


};

};
#endif
