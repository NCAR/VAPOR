#include <iostream>
#include <sstream>
#include <cstdarg>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <sys/stat.h>
#include <geotiff.h>
#include <geo_normalize.h>

#include <vapor/Proj4API.h>
#include <vapor/GeoUtil.h>
#include <vapor/GeoTileMercator.h>
#include <vapor/GeoImageTMS.h>

using namespace VAPoR;
using namespace Wasp;


GeoImageTMS::GeoImageTMS() 
	: GeoImage(8, 4) {

	_dir.clear();
	_maxLOD = 0;
	_texture = NULL;  
	_textureSize = 0;
	_tileBuf = NULL;
	_tileBufSize = 0;
	_geotile = NULL;

	// The default projection string for imagery centered at 0 degrees
	// longitude. This string is modified (+lon_0 is edited) if a 
	// reconstructed contiguous image is not centered at 0 degrees longitude
	//
	_defaultProj4String = "+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 "
		"+lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext "
		"+no_defs";
}

GeoImageTMS::~GeoImageTMS()
{
	if (_texture) delete [] _texture;
	_textureSize = 0;

	if (_tileBuf) delete [] _tileBuf;
		_tileBufSize = 0;

	if (_geotile) delete _geotile;
}


int GeoImageTMS::Initialize(string dir, vector <double> times) {

	SetDiagMsg("GeoImageTMS::Initialize(%s)", dir.c_str());

	if (_geotile) delete _geotile;
	_geotile = NULL;
	_dir = dir;
	_maxLOD = 0;

	// Find the maximum available LOD in the TMS database. 
	//
	int lod = 0;
	while (_tilePath(_dir, 0,0,lod) != "") {
		lod++;
	}
	lod--;

	if (lod<0) {
		SetErrMsg("Failed to initialize TMS directory %s", _dir.c_str());
		return(-1);
	}
	_maxLOD = lod;

	size_t w, h;
	int rc = _tileSize(_dir, 0, 0, 0, w, h);
	if (rc<0) return(-1);

	//
	// Construct a GeoTile. It supports subregion extraction and handles
	// wraparound. I.e. GeoTile can reconstruct contigous image for any
	// longitudinal span
	//
	_geotile = new GeoTileMercator(w, h, 4);

	//
	// Allocate space to buffer a tile
	//
	size_t size = w*h*4;
	if (_tileBufSize < size) {
		delete [] _tileBuf;
		_tileBuf = new unsigned char[size];
		_tileBufSize = size;
	}

	return(0);

}

unsigned char *GeoImageTMS::GetImage(
	size_t ts, size_t &width, size_t &height
) {

	_geotile->GetTileSize(width, height);

	size_t size = width*height*4;
	if (_textureSize < size) {
		delete [] _texture;
		_texture = new unsigned char[size];
		_textureSize = size;
	}

	//
	// Just return the base texture 
	//
	int rc = _tileRead(_dir, 0, 0, 0, _texture);
	if (rc<0) return(NULL);

	return(_texture);
}

unsigned char *GeoImageTMS::GetImage(
	size_t ts, const double pcsExtentsReq[4], string proj4StringReq,
	size_t maxWidthReq, size_t maxHeightReq,	
    double pcsExtentsImg[4], double geoCornersImg[8], string &proj4StringImg,
	size_t &width, size_t &height
) {
	SetDiagMsg("GeoImageTMS::GetImage()");

	proj4StringImg = _defaultProj4String;
	width = height = 0;


	// Convert requested data extents from PCS coordinates to geographic
	// and crop data latitude extents to image latitude extents. With Mercator
	// projections latitudes can't extend beyond ~85 degrees
	//
	double myGeoExtentsData[4];
	for (int i=0; i<4; i++) myGeoExtentsData[i] = pcsExtentsReq[i];

	(void)CornerExtents(myGeoExtentsData, myGeoExtentsData, proj4StringReq);


	// Clamp latitude to extents of image. Can't do same for longitude 
	// because they may be shifted
	//
	// Get the supported lat and lon extents for the GeoTile, which 
	// are guaranteed to be rectangular by the mercator projection.
	//
	double minlon, minlat, maxlon, maxlat;
	_geotile->GetLatLonExtents(minlon, minlat, maxlon, maxlat);

	if (myGeoExtentsData[1] < minlat) myGeoExtentsData[1] = minlat;
	if (myGeoExtentsData[3] > maxlat) myGeoExtentsData[3] = maxlat;


	// Pick a lod that won't allow the resulting image to 
	// exceed the max width and height
	//
	int lod = _getBestLOD(myGeoExtentsData, maxWidthReq, maxHeightReq);
	SetDiagMsg("GeoImageTMS::GetImage() LOD : %d", lod);

	//
	// Get GeoTile's pixel coordinates of subregion at the given lod 
	//
	size_t pixelSW[2];
	size_t pixelNE[2];
	size_t nx,ny;
	_geotile->LatLongToPixelXY(
		myGeoExtentsData[0], myGeoExtentsData[1], lod, 
		pixelSW[0], pixelSW[1]
	);
	_geotile->LatLongToPixelXY(
		myGeoExtentsData[2], myGeoExtentsData[3], lod, 
		pixelNE[0], pixelNE[1]
	);

	int rc = _geotile->MapSize(
		pixelSW[0],pixelSW[1],pixelNE[0],pixelNE[1],lod,nx, ny
	);

	//
	// Resize the buffer if needed.
	//
	size_t size = nx*ny*4;
	if (_textureSize < size) {
		delete [] _texture;
		_texture = new unsigned char[size];
		_textureSize = size;
	}

	//
	// Extract the image
	//
	rc = _getMap(pixelSW, pixelNE, lod, _texture);
	if (rc<0) return(NULL);


	//
	// If data crosses -180 or 180 we need to generate a new
	// proj4 string with the correct centering
	//
	if (myGeoExtentsData[0] < -180 || myGeoExtentsData[2] > 180.0) {
		double lon_0 = (myGeoExtentsData[0] + myGeoExtentsData[2]) / 2.0;
		ostringstream oss;
		oss.precision(12);
		oss << " +lon_0=" << lon_0;
		string::size_type first = proj4StringImg.find("+lon_0");
		if (first == string::npos) { 
			proj4StringImg += oss.str();
		}
		else {
			string::size_type last = proj4StringImg.find(" ", first);
			assert (last != string::npos);
			proj4StringImg.replace(first, last-first, oss.str());
		}
	}


	//
	// Finally, update the extents of the new image in PCS coordinates 
	// by mapping geographic coordinates of corners to PCS.
	// Since the projection is eqc we only need south-west and north-east 
	// points. 
	//
	Proj4API proj4API;
	proj4API.Initialize("",proj4StringImg);

	pcsExtentsImg[0] = myGeoExtentsData[0];
	pcsExtentsImg[1] = myGeoExtentsData[1];
	pcsExtentsImg[2] = myGeoExtentsData[2];
	pcsExtentsImg[3] = myGeoExtentsData[3];

	proj4API.Transform(pcsExtentsImg, pcsExtentsImg+1, 2, 2);
	width = nx;
	height = ny;

	// Image is rectangular so corners come directry from extents
	//
	geoCornersImg[0] = myGeoExtentsData[0];
	geoCornersImg[1] = myGeoExtentsData[1];
	geoCornersImg[2] = myGeoExtentsData[0];
	geoCornersImg[3] = myGeoExtentsData[3];
	geoCornersImg[4] = myGeoExtentsData[2];
	geoCornersImg[5] = myGeoExtentsData[3];
	geoCornersImg[6] = myGeoExtentsData[2];
	geoCornersImg[7] = myGeoExtentsData[1];

	return (_texture);
}


// Get the file path of a single tile from the TMS database
// with the give lod and tile coordinates
//
string GeoImageTMS::_tilePath(
	string dir, size_t tileX, size_t tileY, int lod
) const {

//	size_t ntiles = 1 << lod;
//	size_t tmsTileY = ntiles - 1 - tileY;
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

// Get the dimensions of a single tile from the TMS database
// N.B. This method allows the specification of the tile coordinates,
// but all tiles in a single TMS must be the same size
//
int GeoImageTMS::_tileSize(
	string dir, size_t tileX, size_t tileY, int lod, size_t &w, size_t &h
) {
	
	string path = _tilePath(dir, tileX, tileY, lod);
	if (path.empty()) {
		SetErrMsg("Tile %d %d %d does not exist", tileX, tileY, lod);
		return(-1);
	}

	int rc = GeoImage::TiffOpen(path);
	if (rc<0) return (-1);

	rc = GeoImage::TiffGetImageDimensions(0, w, h);
	if (rc<0) {
		GeoImage::TiffClose();
		return(-1);
	}

	GeoImage::TiffClose();

	return(0);
}

// Read a single tile from the TMS database and return as a raster
// image
//
int GeoImageTMS::_tileRead(
	string dir, size_t tileX, size_t tileY, int lod, unsigned char *tile
) {
	SetDiagMsg(
		"GeoImageTMS::_tileRead(%s,%d,%d,%d)", dir.c_str(), tileX, tileY, lod
	);

	string path = _tilePath(dir, tileX, tileY, lod);
	if (path.empty()) {
		SetErrMsg("Tile %d %d %d does not exist", tileX, tileY, lod);
		return(-1);
	}
	SetDiagMsg("GeoImageTMS::_tileRead() path : %s", path.c_str());

	int rc = GeoImage::TiffOpen(path);
	if (rc<0) return (-1);

	rc = GeoImage::TiffReadImage(0, tile);
	if (rc<0) {
		GeoImage::TiffClose();
		return(-1);
	}

	GeoImage::TiffClose();

	return(0);
}

// Determine the best lod for a region specified in lat-lon
// coordinates. I.e. the largest lod where the resulting image
// dimensions won't exceed maxWidthReq or maxHeightReq
//
int GeoImageTMS::_getBestLOD(
	const double myGeoExtentsData[4], int maxWidthReq, int maxHeightReq
) const {
	size_t pixelSW[2];
	size_t pixelNE[2];

	bool done = false;
	int lod = 0;
	for (; lod<_maxLOD && ! done; lod++) {
		size_t nx, ny;

		//
		// Get GeoTile's pixel coordinates of subregion. 
		//
		_geotile->LatLongToPixelXY(
			myGeoExtentsData[0], myGeoExtentsData[1], lod, 
			pixelSW[0], pixelSW[1]
		);
		_geotile->LatLongToPixelXY(
			myGeoExtentsData[2], myGeoExtentsData[3], lod, 
			pixelNE[0], pixelNE[1]
		);

		int rc = _geotile->MapSize(
			pixelSW[0],pixelSW[1],pixelNE[0],pixelNE[1],lod,nx, ny
		);
		assert(! (rc<0));

		if (nx > maxWidthReq || ny > maxHeightReq) {
			done = true;
			if (lod>0) lod--;
		}
	}

	return(lod);
}

// Construct a contiguous map (raster image) from the TMS for the 
// specified ROI
//

int GeoImageTMS::_getMap(
	const size_t pixelSW[2], const size_t pixelNE[2], 
	int lod, unsigned char *texture
) {


	size_t tileX0, tileX1;
	size_t tileY0, tileY1;
	size_t dummy;
	

	_geotile->PixelXYToTileXY(
		pixelSW[0], pixelSW[1], tileX0, tileY0, dummy, dummy
	);

	_geotile->PixelXYToTileXY(
		pixelNE[0], pixelNE[1], tileX1, tileY1, dummy, dummy
	);


	// Deal with wraparound
	//
	size_t ntiles = 1 << lod;
	size_t nxtiles, nytiles;
	if (pixelSW[0] < pixelNE[0]) {
		nxtiles = tileX1 - tileX0 + 1;
	}
	else {
		nxtiles = ntiles - ((tileX1==tileX0) ? 0 : (tileX0-tileX1-1));
	}

	if (pixelSW[1] < pixelNE[1]) {
		nytiles = tileY1 - tileY0 + 1;
	}
	else {
		nytiles = ntiles - ((tileY1==tileY0) ? 0 : (tileY0-tileY1-1));
	}


	// Make sure tiles need for this map are loaded. If not, read
	// and load them 
	//
	size_t tileY = tileY0;
	for (size_t y=0; y<nytiles; y++) {

		size_t tileX = tileX0;
		for (size_t x=0; x<nxtiles; x++) {

			string quadkey = _geotile->TileXYToQuadKey(tileX, tileY, lod);
			if (! _geotile->GetTile(quadkey)) {

				int rc = _tileRead( _dir, tileX, tileY, lod, _tileBuf);
				if (rc<0) return(-1);

				rc = _geotile->Insert(quadkey, _tileBuf);
				assert( !(rc<0));
			}
			tileX = (tileX + 1) % ntiles;

		}

		tileY = (tileY + 1) % ntiles;
	}

	int rc = _geotile->GetMap(
		pixelSW[0],pixelSW[1],pixelNE[0],pixelNE[1],lod,texture
	);
	return(rc);
}
