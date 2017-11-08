#include <iostream>
#include <sstream>
#include <cstdarg>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <sys/stat.h>
#ifdef WIN32
    #include <geotiff/geotiff.h>
    #include <geotiff/geo_normalize.h>
#else
    #include <geotiff.h>
    #include <geo_normalize.h>
#endif

#include <vapor/Proj4API.h>
#include <vapor/GeoUtil.h>
#include <vapor/GeoTileEquirectangular.h>
#include <vapor/GeoTileMercator.h>
#include <vapor/GeoImageGeoTiff.h>

using namespace VAPoR;
using namespace Wasp;

GeoImageGeoTiff::GeoImageGeoTiff() : GeoImage(8, 4)
{
    _tiffTimes.clear();
    _times.clear();
    _texture = NULL;
    _textureSize = 0;
}

GeoImageGeoTiff::~GeoImageGeoTiff()
{
    if (_texture) delete[] _texture;
    _textureSize = 0;
}

int GeoImageGeoTiff::Initialize(string path, vector<double> times)
{
    GeoImage::TiffClose();
    _tiffTimes.clear();
    _times.clear();

    int rc = GeoImage::TiffOpen(path);
    if (rc < 0) return (-1);

    // Set up time vector
    //
    _initTimeVector(GeoImage::TiffGetHandle(), times);

    return (0);
}

unsigned char *GeoImageGeoTiff::GetImage(size_t ts, size_t &width, size_t &height)
{
    // Get tiff directory number for this time step
    //
    int dirnum = _getBestDirNum(ts);

    // Get dimensions image at given time step
    //
    int rc = GeoImage::TiffGetImageDimensions(dirnum, width, height);
    if (rc < 0) return (NULL);

    //
    // Read the image texture from a file
    //
    size_t size = width * height * 4;
    if (_textureSize < size) {
        delete[] _texture;
        _texture = new unsigned char[size];
        _textureSize = size;
    }

    rc = GeoImage::TiffReadImage(dirnum, _texture);
    if (!_texture) return (NULL);

    return (_texture);
}

unsigned char *GeoImageGeoTiff::GetImage(size_t ts, const double pcsExtentsReq[4], string proj4StringReq, size_t maxWidthReq, size_t maxHeightReq,    // not used
                                         double pcsExtentsImg[4], double geoCornersImg[8], string &proj4StringImg, size_t &width, size_t &height)
{
    for (int i = 0; i < 4; i++) {
        pcsExtentsImg[i] = pcsExtentsReq[i];
        geoCornersImg[i] = 0.0;
        geoCornersImg[i + 4] = 0.0;    // Do we need this parameter?
    }
    proj4StringImg.clear();
    width = height = 0;

    _texture = GeoImageGeoTiff::GetImage(ts, width, height);
    if (!_texture) return (NULL);

    //
    // Get extents of image in both Projected Coordinates (PCS)
    // and lat-long.
    // N.B. if the proj4String defined a lat-long
    // projection (proj=latlong), it's modified to be cylindrical
    // equidistant (proj=eqc)
    //
    int rc = _getGTIFInfo(GeoImage::TiffGetHandle(), width, height, pcsExtentsImg, geoCornersImg, proj4StringImg);
    if (rc < 0) return (NULL);

    //
    // If image is not geo referenced we're done. pcsExtentsImg are
    // already initialized to pcsExtentsReq
    //
    if (!proj4StringImg.size()) return (_texture);

    //
    // Attempt to crop the texture to the smallest rectangle
    // that covers the data space. Only possible if image is
    // a global, cylindrical-equidistant or pseudo mercator projection.
    // _extractSubtexture()
    // modifies with, height, and pcsExtents if successful, otherwise
    // they are unchanged.
    //
    (void)_extractSubtexture(_texture, width, height, pcsExtentsReq, proj4StringReq, pcsExtentsImg, geoCornersImg, proj4StringImg, proj4StringImg, width, height, pcsExtentsImg, geoCornersImg);

    return (unsigned char *)_texture;
}

// Get the Proj4 projection string from a GTIF handle
//
string GeoImageGeoTiff::_getProjectionFromGTIF(GTIF *gtifHandle) const
{
    GTIFDefn gtifDef;
    GTIFGetDefn(gtifHandle, &gtifDef);

    string proj4String = GTIFGetProj4Defn(&gtifDef);
    if (proj4String.empty()) return (proj4String);

    // If there's no "ellps=" in the string, force it to be spherical,
    // This avoids a bug in the geotiff routines
    //
    if (std::string::npos == proj4String.find("ellps=")) { proj4String += " +ellps=sphere"; }
    return (proj4String);
}

// Construct a time vector, _tiffTimes, containing the time coordinates
// for all images in the TIFF data base
//
void GeoImageGeoTiff::_initTimeVector(TIFF *tif, const vector<double> &times)
{
    _times = times;
    _tiffTimes.clear();

    // Check if the first time step (tiff "directory" has a time stamp.
    // If it does all directories must have a time stamp. If no time
    // stamp is present then the time stamp is taken from the data itself.
    //
    TIFFSetDirectory(tif, 0);

    char *timePtr = NULL;
    bool  hasTime = TIFFGetField(tif, TIFFTAG_DATETIME, &timePtr);

    // build a list of the times in the tiff
    //
    UDUnits udunits;
    udunits.Initialize();

    do {
        int dircount = 0;

        if (hasTime) {
            double tifftime = 0.0;
            bool   ok = _getTiffTime(tif, &udunits, tifftime);
            if (!ok) {
                SetDiagMsg("Failed to read time stamp from TIFF image");
                break;
            }
            _tiffTimes.push_back(tifftime);
        } else {
            if (dircount < _times.size()) _tiffTimes.push_back(_times[dircount]);
        }

        dircount++;
    } while (TIFFReadDirectory(tif));

    return;
}

// Get the time coordinate of the current tiff directory, convert it
// from a date to seconds since EPOCH using udunits
//
bool GeoImageGeoTiff::_getTiffTime(TIFF *tif, UDUnits *udunits, double &tifftime) const
{
    tifftime = 0.0;

    char *timePtr = NULL;
    bool  hasTime = (bool)TIFFGetField(tif, TIFFTAG_DATETIME, &timePtr);

    if (!hasTime) return (false);

    // determine seconds from the time stamp in the tiff
    // convert tifftags to use WRF style date/time strings
    //
    int         year, mon, mday, hour, min, sec;
    const char *format = "%4d:%2d:%2d %2d:%2d:%2d";
    int         rc = sscanf(timePtr, format, &year, &mon, &mday, &hour, &min, &sec);

    // For backwords compatibility check WRF-style format
    //
    if (rc != 6) {
        format = "%4d-%2d-%2d_%2d:%2d:%2d";
        rc = sscanf(timePtr, format, &year, &mon, &mday, &hour, &min, &sec);

        if (rc != 6) return (false);
    }

    tifftime = udunits->EncodeTime(year, mon, mday, hour, min, sec);
    return (true);
}

// Find the tiff time coordinate that best matches the time step
// indicated by ts. Return an index into _tiffTimes for the best
// match.
//
int GeoImageGeoTiff::_getBestDirNum(size_t ts) const
{
    assert(ts < _times.size());
    assert(_tiffTimes.size());

    double t = _times[ts];

    int bestdir = 0;
    for (int i = 0; i < _tiffTimes.size(); i++) {
        if (fabs(_tiffTimes[i] - t) < fabs(_tiffTimes[bestdir] - t)) { bestdir = i; }
    }

    return (bestdir);
}

int GeoImageGeoTiff::_getGTIFInfo(TIFF *tif, size_t width, size_t height, double pcsExtents[4], double geoCorners[8], string &proj4String) const
{
    for (int i = 0; i < 4; i++) {
        pcsExtents[i] = 0.0;
        geoCorners[i] = geoCorners[i + 4] = 0.0;
    }
    proj4String.clear();

    GTIF *gtifHandle = GTIFNew(tif);
    assert(gtifHandle != NULL);

    // Read proj4 string from geotiff file
    //
    proj4String = _getProjectionFromGTIF(gtifHandle);
    if (proj4String.empty()) return (0);

    //
    // Get image extents by converting from pixel to
    // Projection Coordinate Space (PCS). Note, if the
    // proj4 projection is "latlong" the conversion will be from
    // pixels to geographic coords (i.e. lat and long) :-(
    //
    // N.B. GTIF library expects Y origin at top of image, we
    // us bottom for Y origin
    //
    //	double extents[4] = {0.0, height-1, width-1, 0};
    double extents[4] = {0.0, (double)height, (double)width, 0.0};

    bool ok = GTIFImageToPCS(gtifHandle, &extents[0], &extents[1]);
    if (!ok) {
        SetErrMsg("GTIFImageToPCS()");
        GTIFFree(gtifHandle);
        return (-1);
    }
    ok = GTIFImageToPCS(gtifHandle, &extents[2], &extents[3]);
    if (!ok) {
        SetErrMsg("GTIFImageToPCS()");
        GTIFFree(gtifHandle);
        return (-1);
    }
    GTIFFree(gtifHandle);

    //
    // Set up proj4 to convert from PCS, image space to lat-long
    //
    Proj4API proj4API;
    int      rc = proj4API.Initialize(proj4String, "");
    if (rc < 0) return (-1);

    // Make sure projection is not lat-long. If so, convert to eqc
    //
    if (proj4API.IsLatLonSrc()) {
        // Oops. Projection string was lat-long. This means we have
        // lat long coordinates in 'extents', not PCS. We need to generate
        // a new proj4 string to find geographic (lat-lon) coordinates
        //
        double        lon_0 = (extents[0] + extents[2]) / 2.0;
        double        lat_0 = (extents[1] + extents[3]) / 2.0;
        ostringstream oss;
        oss.precision(12);
        oss << " +lon_0=" << lon_0 << " +lat_0=" << lat_0;
        proj4String = "+proj=eqc +ellps=WGS84" + oss.str();
        int rc = proj4API.Initialize("", proj4String);
        if (rc < 0) return (-1);

        // Map geographic to PCS coordinates
        //
        proj4API.Transform(extents, extents + 1, 2, 2);

        // Reinitialize proj4API to map from geographic to PCS coordinates
        //
        rc = proj4API.Initialize(proj4String, "");
        if (rc < 0) return (-1);
    }

    // Clamp image bounds to range permitted by projection.
    //
    proj4API.Clamp(extents, extents + 1, 2, 2);

    for (int i = 0; i < 4; i++) { pcsExtents[i] = extents[i]; }

    //
    // clockwise order starting with lower-left
    //
    geoCorners[0] = extents[0];
    geoCorners[1] = extents[1];
    geoCorners[2] = extents[0];
    geoCorners[3] = extents[3];
    geoCorners[4] = extents[2];
    geoCorners[5] = extents[3];
    geoCorners[6] = extents[2];
    geoCorners[7] = extents[1];

    //
    // Transform from PCS to lat-long
    //
    proj4API.Transform(geoCorners, geoCorners + 1, 4, 2);

    // Don't allow latlong image to go all the way to the poles
    // Not sure why they can't to to 90. This code may no longer
    // be necessary.
    //
    if (geoCorners[1] < -89.9999) geoCorners[1] = -89.9999;
    if (geoCorners[7] < -89.9999) geoCorners[7] = -89.9999;
    if (geoCorners[3] > 89.9999) geoCorners[3] = 89.9999;
    if (geoCorners[5] > 89.9999) geoCorners[5] = 89.9999;

    return (0);
}

// Extract a ROI from a GeoTiff texture. Only possible if image is global
// and in either cylindrical equidistant or mercator projection. Otherwise
// the original texture is returned unchanged.
//
// texture : Pointer to a 2D texture containing the geoimage
// width, height : dimensions of texture in pixels
// pcsExtentsReq : Extents (llx, lly, urx, ury) of desired ROI expressed
// in PCS coordinates of the *image*
//
// proj4StringReq : Proj4 string associated with texture
// pcsExtentsImg : Extents (llx, lly, urx, ury) of texture expressed
// in PCS coordinates of the *image*
//
// geoCornersImg : Corner points (lat, lon) of image corners
// in clockwise order starting from S.W. corner
//
// proj4StringImg : Proj4 string associated with texture (how different
// from proj4StringReq)/
//
// subProj4StringImg : Proj4 string for returned image. The string
// may be modified from the orginal if the desired ROI crosses
// the boundary of the original image
//
// subWidth, subHeight : width and height of returned texture
// subPCSExtentsImg : Extents (llx, lly, urx, ury) of returned texture
// expressed
// in PCS coordinates of the *image*
//
bool GeoImageGeoTiff::_extractSubtexture(unsigned char *texture, size_t width, size_t height, const double pcsExtentsReq[4], string proj4StringReq, const double pcsExtentsImg[4],
                                         const double geoCornersImg[8], string proj4StringImg, string &subProj4StringImg, size_t &subWidth, size_t &subHeight, double subPCSExtentsImg[4],
                                         double subGeoCornersImg[8]) const
{
    //
    // Initialize output params to inputs in case of failure, in which
    // case the entire image is returned unchanged (uncropped)
    //
    subProj4StringImg = proj4StringImg;
    subWidth = width;
    subHeight = height;
    for (int i = 0; i < 4; i++) { subPCSExtentsImg[i] = pcsExtentsImg[i]; }
    for (int i = 0; i < 8; i++) { subGeoCornersImg[i] = geoCornersImg[i]; }

    //
    // Can only extract sub-textures from georeferenced images
    // with cylindrical equi-distant or mercator projections that cover
    // entire globe. This is really a hack and we need a better test
    // for global coverage.
    //
    bool mercator = std::string::npos != subProj4StringImg.find("proj=merc");
    bool eqc = std::string::npos != subProj4StringImg.find("proj=eqc");

    if (!mercator && !eqc) return (false);

    // There must be a better way to determine if image is global
    //
    if (geoCornersImg[0] > -179.5 || geoCornersImg[4] < 179.5) { return (false); }
    if (eqc) {
        if (geoCornersImg[1] > -89.5 || geoCornersImg[5] < 89.5) return (false);
        if (width != 2 * height) return (false);
    }
    if (mercator) {
        if (geoCornersImg[1] > -85.0 || geoCornersImg[5] < 85.0) return (false);
        if (width != height) return (false);
    }

    // Convert requested data extents from PCS coordinates to geographic
    // and crop data latitude extents to image latitude extents. With Mercator
    // projections latitudes can't extend beyond ~85 degrees
    //
    double myGeoExtentsData[4];
    for (int i = 0; i < 4; i++) myGeoExtentsData[i] = pcsExtentsReq[i];

    (void)CornerExtents(myGeoExtentsData, myGeoExtentsData, proj4StringReq);

    // Clamp latitude to extents of image. Can't do same for longitude
    // becuase they may be shifted
    //
    if (myGeoExtentsData[1] < geoCornersImg[1]) myGeoExtentsData[1] = geoCornersImg[1];

    if (myGeoExtentsData[3] > geoCornersImg[5]) myGeoExtentsData[3] = geoCornersImg[5];

    //
    // Construct a GeoTile. It supports subregion extraction and handles
    // wraparound
    //
    GeoTile *geotile;
    if (eqc) {
        geotile = new GeoTileEquirectangular(width, height, 4);
    } else {
        geotile = new GeoTileMercator(width, height, 4);
    }
    geotile->Insert("", (unsigned char *)texture);
    size_t pixelSW[2];
    size_t pixelNE[2];
    size_t nx, ny;

    //
    // Get GeoTile's pixel coordinates of subregion.
    //
    geotile->LatLongToPixelXY(myGeoExtentsData[0], myGeoExtentsData[1], 0, pixelSW[0], pixelSW[1]);
    geotile->LatLongToPixelXY(myGeoExtentsData[2], myGeoExtentsData[3], 0, pixelNE[0], pixelNE[1]);

    int rc = geotile->MapSize(pixelSW[0], pixelSW[1], pixelNE[0], pixelNE[1], 0, nx, ny);
    if (rc != 0) {
        delete geotile;
        return false;
    };

    //
    // Extract the image
    //
    assert(nx <= width);
    assert(ny <= height);
    rc = geotile->GetMap(pixelSW[0], pixelSW[1], pixelNE[0], pixelNE[1], 0, texture);
    if (rc != 0) {
        delete geotile;
        return false;
    };

    delete geotile;

    //
    // If data crosses -180 or 180 we need to generate a new
    // proj4 string with the correct centering
    //
    if (myGeoExtentsData[0] < -180 || myGeoExtentsData[2] > 180.0) {
        double        lon_0 = (myGeoExtentsData[0] + myGeoExtentsData[2]) / 2.0;
        ostringstream oss;
        oss.precision(12);
        oss << " +lon_0=" << lon_0;
        string::size_type first = subProj4StringImg.find("+lon_0");
        if (first == string::npos) {
            subProj4StringImg += oss.str();
        } else {
            string::size_type last = subProj4StringImg.find(" ", first);
            assert(last != string::npos);
            subProj4StringImg.replace(first, last - first, oss.str());
        }
    }

    //
    // Finally, update the extents of the new image in PCS coordinates
    // by mapping geographic coordinates of corners to PCS.
    // Since the projection is eqc we only need south-west and north-east
    // points.
    //
    Proj4API proj4API;
    proj4API.Initialize("", subProj4StringImg);

    subPCSExtentsImg[0] = myGeoExtentsData[0];
    subPCSExtentsImg[1] = myGeoExtentsData[1];
    subPCSExtentsImg[2] = myGeoExtentsData[2];
    subPCSExtentsImg[3] = myGeoExtentsData[3];

    proj4API.Transform(subPCSExtentsImg, subPCSExtentsImg + 1, 2, 2);
    subWidth = (int)nx;
    subHeight = (int)ny;

    // Image is rectangular so corners come directry from extents
    //
    subGeoCornersImg[0] = myGeoExtentsData[0];
    subGeoCornersImg[1] = myGeoExtentsData[1];
    subGeoCornersImg[2] = myGeoExtentsData[0];
    subGeoCornersImg[3] = myGeoExtentsData[3];
    subGeoCornersImg[4] = myGeoExtentsData[2];
    subGeoCornersImg[5] = myGeoExtentsData[3];
    subGeoCornersImg[6] = myGeoExtentsData[2];
    subGeoCornersImg[7] = myGeoExtentsData[1];

    return true;
}
