#include <iostream>
#include <sstream>
#include <cstdarg>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <sys/stat.h>
#include <vapor/GeoUtil.h>
#include <vapor/Proj4API.h>

#include "vapor/GeoImage.h"
#ifdef WIN32
    #pragma warning(disable : 4996)
#endif
using namespace VAPoR;
using namespace Wasp;

namespace {

// Error handling for TIFF library
//
void myTiffErrHandler(const char *module, const char *fmt, va_list ap)
{
    char buf[1024];

#ifdef WIN32
    _vsnprintf(buf, sizeof(buf), fmt, ap);
#else
    vsnprintf(buf, sizeof(buf), fmt, ap);
#endif

    if (module) {
        MyBase::SetErrMsg("%s : %s", module, buf);
    } else {
        MyBase::SetErrMsg("%s", buf);
    }
}

};    // namespace

GeoImage::GeoImage(int pixelsize, int nbands) : _pixelsize(pixelsize), _nbands(nbands)
{
    assert(pixelsize = 8);
    assert(nbands = 4);
    _tif = NULL;
    _path.clear();
}

GeoImage::GeoImage() : _pixelsize(8), _nbands(4)
{
    _path.clear();
    _tif = NULL;
}

GeoImage::~GeoImage() { GeoImage::TiffClose(); }

int GeoImage::TiffOpen(string path)
{
    TIFFSetErrorHandler(myTiffErrHandler);

    GeoImage::TiffClose();

    // Check for a valid file name (this avoids Linux crash):
    //
    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) < 0) {
        SetErrMsg("Invalid tiff file: %s\n", path.c_str());
        return -1;
    }

    // Not using memory-mapped IO (m) is reputed to help plug
    // leaks (but doesn't do any good on windows for me)
    //
    _tif = XTIFFOpen(path.c_str(), "rm");
    if (!_tif) {
        SetErrMsg("Unable to open tiff file: %s\n", path.c_str());
        return -1;
    }

    char emsg[1000];
    int  ok = TIFFRGBAImageOK(_tif, emsg);
    if (!ok) {
        MyBase::SetErrMsg("Unable to process tiff file:\n %s\nError message: %s", path.c_str(), emsg);
        return (-1);
    }

    // Check compression.  Some compressions, e.g. jpeg, cause crash on Linux
    //
#ifdef DEAD
    short compr = 1;
    ok = TIFFGetField(_tif, TIFFTAG_COMPRESSION, &compr);
    if (ok) {
        if (compr != COMPRESSION_NONE && compr != COMPRESSION_LZW && compr != COMPRESSION_JPEG && compr != COMPRESSION_CCITTRLE) {
            MyBase::SetErrMsg("Unsupported Tiff compression");
            return (-1);
        }
    }
#endif

    return (0);
}

void GeoImage::TiffClose()
{
    if (_tif) XTIFFClose(_tif);
    _path.clear();
    _tif = NULL;
}

// Return dimensions of image at selected directory number
//
int GeoImage::TiffGetImageDimensions(int dirnum, size_t &width, size_t &height) const
{
    assert(_tif != NULL);
    width = 0;
    height = 0;

    bool ok = (bool)TIFFSetDirectory(_tif, dirnum);
    if (!ok) return (-1);

    uint32 w;
    ok = (bool)TIFFGetField(_tif, TIFFTAG_IMAGEWIDTH, &w);
    if (!ok) return (-1);

    uint32 h;
    ok = (bool)TIFFGetField(_tif, TIFFTAG_IMAGELENGTH, &h);
    if (!ok) return (-1);

    width = (size_t)w;
    height = (size_t)h;

    return (0);
}

// Read the indicated TIFF image and return it as a 2D texture.
//
int GeoImage::TiffReadImage(int dirnum, unsigned char *texture) const
{
    assert(_tif != NULL);

    uint32 *texuint32 = (uint32 *)texture;

    bool ok = (bool)TIFFSetDirectory(_tif, dirnum);
    if (!ok) return (-1);

    size_t w, h;
    int    rc = GeoImage::TiffGetImageDimensions(dirnum, w, h);
    if (rc < 0) return (-1);

    // Check if this is a 2-component 8-bit image.  These are read
    // by scanline since TIFFReadRGBAImage
    // apparently does not know how to get the alpha channel
    //
    short nsamples, nbitspersample;
    ok = TIFFGetField(_tif, TIFFTAG_SAMPLESPERPIXEL, &nsamples);
    if (!ok) return (-1);

    ok = TIFFGetField(_tif, TIFFTAG_BITSPERSAMPLE, &nbitspersample);
    if (!ok) return (-1);

    if (nsamples == 2 && nbitspersample == 8) {
        tdata_t buf;
        uint32  row;
        short   config;
        short   photometric;

        TIFFGetField(_tif, TIFFTAG_PLANARCONFIG, &config);
        if (!ok) return (-1);

        TIFFGetField(_tif, TIFFTAG_PHOTOMETRIC, &photometric);
        if (!ok) return (-1);

        buf = _TIFFmalloc(TIFFScanlineSize(_tif));
        assert(buf != NULL);

        unsigned char *charArray = (unsigned char *)buf;
        int            scanlength = TIFFScanlineSize(_tif) / 2;

        if (config == PLANARCONFIG_CONTIG) {
            for (row = 0; row < h; row++) {
                int revrow = h - row - 1;    // reverse, go bottom up
                int rc = TIFFReadScanline(_tif, buf, row);
                if (rc < 0) {
                    MyBase::SetErrMsg("Error reading tiff file:\n %s\n", _path.c_str());
                    _TIFFfree(buf);
                    return (-1);
                }
                for (int k = 0; k < scanlength; k++) {
                    unsigned char greyval = charArray[2 * k];
                    // If white is zero, reverse it:
                    if (!photometric) greyval = 255 - greyval;
                    unsigned char alphaval = charArray[2 * k + 1];
                    texuint32[revrow * w + k] = alphaval << 24 | greyval << 16 | greyval << 8 | greyval;
                }
            }
        } else if (config == PLANARCONFIG_SEPARATE) {
            uint16 s;

            // Note: following loop (adapted from tiff docs) has not
            // been tested.  Are there tiff
            // examples with separate alpha channel?
            //
            for (s = 0; s < nsamples; s++) {
                for (row = 0; row < h; row++) {
                    int rc = TIFFReadScanline(_tif, buf, row, s);
                    if (rc < 0) {
                        MyBase::SetErrMsg("Error reading tiff file:\n %s\n", _path.c_str());
                        _TIFFfree(buf);
                        return (-1);
                    }
                    int revrow = h - row - 1;    // reverse, go bottom up
                    if (!(s % 2)) {              // color
                        for (int k = 0; k < h; k++) {
                            unsigned char greyval = charArray[k];
                            // If white is zero, reverse it:
                            if (!photometric) greyval = 255 - greyval;
                            texuint32[revrow * w + k] = greyval << 16 | greyval << 8 | greyval;
                        }
                    } else {    // alpha
                        for (int k = 0; k < h; k++) {
                            unsigned char alphaval = charArray[k];
                            texuint32[revrow * w + k] = alphaval << 24 | (texuint32[revrow * w + k] & 0xffffff);
                        }
                    }
                }
            }
        }
        _TIFFfree(buf);
        return (0);

    } else {
        // Read pixels, whether or not we are georeferenced:

        ok = TIFFReadRGBAImage(_tif, w, h, texuint32, 0);
        if (!ok) {
            MyBase::SetErrMsg("Error reading tiff file:\n %s\n", _path.c_str());
            return -1;
        }

        return (0);
    }
}

// Project extents (ll, ur) given in PCS coordinates in srccoords using the
// specified projection in proj4src
// into 4 corners and return the extents (ll, ur) in the projected
// space. Because the projected space may be convex - the maximum and
// minimum values may not occur at the corner points - we need to walk
// around the boundaries to find the extents in the projected space.
//
int GeoImage::CornerExtents(const double srccoords[4], double dstcoords[4], string proj4src) const
{
    Proj4API proj4API;
    int      rc = proj4API.Initialize(proj4src, "");
    if (rc < 0) return (-1);

#ifdef DEAD
    double corners[8] = {
        srccoords[0],    // llx
        srccoords[1],    // lly
        srccoords[0],    // ulx
        srccoords[3],    // uly
        srccoords[2],    // urx
        srccoords[3],    // ury
        srccoords[2],    // lrx
        srccoords[1]     // lry
    };

    int     nsamples = 16;
    int     ntotal = 2 * nsamples + 2 * nsamples;
    double *xsamples = new double[2 * ntotal];
    double *ysamples = xsamples + ntotal;
    double *xptr = xsamples;
    double *yptr = ysamples;

    // Walk along each boundary
    //
    double deltax = (corners[2] - corners[0]) / (double)(nsamples - 1);
    double deltay = (corners[3] - corners[1]) / (double)(nsamples - 1);
    for (int i = 0; i < nsamples; i++) {
        *xptr++ = corners[0] + (i * deltax);
        *yptr++ = corners[1] + (i * deltay);
    }

    deltax = (corners[4] - corners[2]) / (double)(nsamples - 1);
    deltay = (corners[5] - corners[3]) / (double)(nsamples - 1);
    for (int i = 0; i < nsamples; i++) {
        *xptr++ = corners[2] + (i * deltax);
        *yptr++ = corners[3] + (i * deltay);
    }

    deltax = (corners[6] - corners[4]) / (double)(nsamples - 1);
    deltay = (corners[7] - corners[5]) / (double)(nsamples - 1);
    for (int i = 0; i < nsamples; i++) {
        *xptr++ = corners[4] + (i * deltax);
        *yptr++ = corners[5] + (i * deltay);
    }

    deltax = (corners[0] - corners[6]) / (double)(nsamples - 1);
    deltay = (corners[1] - corners[7]) / (double)(nsamples - 1);
    for (int i = 0; i < nsamples; i++) {
        *xptr++ = corners[6] + (i * deltax);
        *yptr++ = corners[7] + (i * deltay);
    }

#else

    int     nx = 256;
    int     ny = 256;
    int     ntotal = nx * ny;
    double *xsamples = new double[2 * ntotal];
    double *ysamples = xsamples + ntotal;

    double deltax = (srccoords[2] - srccoords[0]) / (double)(nx - 1);
    double deltay = (srccoords[3] - srccoords[1]) / (double)(ny - 1);

    for (int j = 0; j < ny; j++) {
        for (int i = 0; i < nx; i++) {
            xsamples[j * nx + i] = srccoords[0] + i * deltax;
            ysamples[j * nx + i] = srccoords[1] + j * deltay;
        }
    }

#endif

    proj4API.Transform(xsamples, ysamples, ntotal, 1);

    double minx = xsamples[0];
    double maxx = xsamples[0];
    double miny = ysamples[0];
    double maxy = ysamples[0];

    // Find the extents the extents of the
    // entire enclosed region
    //
    GeoUtil::LonExtents(xsamples, nx, ny, minx, maxx);
    GeoUtil::LatExtents(ysamples, nx, ny, miny, maxy);

    delete[] xsamples;

    // still needed?
    //
    if ((maxx - minx) >= 359.99) { maxx = minx + 359.99; }

    dstcoords[0] = minx;
    dstcoords[1] = miny;
    dstcoords[2] = maxx;
    dstcoords[3] = maxy;

    return (0);
}
