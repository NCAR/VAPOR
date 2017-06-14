
#ifndef _GeoImage_h_
#define _GeoImage_h_

#include <xtiffio.h>

#include <vapor/MyBase.h>

namespace VAPoR {

//! \class GeoImage
//! \brief An abstract class for managing geo-referenced images
//! \author John Clyne
//!
//
class RENDER_API GeoImage : public Wasp::MyBase {
public:

 
 GeoImage();

 //! \param[in] pixelsize Size, in bits, of returned pixel channel
 //! \param[i] nbands Number of channel in a returned image
 //
 GeoImage(int pixelsize, int nbands);

 virtual ~GeoImage();

 //! Initialize the class
 //!
 //! \param[in] path Path to file or directory containing image 
 //! database
 //!
 //! \param[in] times Times coordinate variable. If image database contains
 //! time-varying images the time stamps of the images will be compared
 //! against \p times and the best match will be returned
 //!
 //! \retval status return -1 on failure
 //
 virtual int Initialize(string path, std::vector <double> times) = 0;

 //! Fetch a non-georeferenced image
 //!
 //! Return an image without any geo-referencing information. 
 //!
 //! \param[in] ts time step of image
 //! \param[out] width Width in pixels of returned image
 //! \param[out] height Height in pixels of returned image
 //!
 //! \retval image Upon success at 2D texture with n channels 
 //! (See GeoImage::GeoImage()) is returned.  Memory for the returned image
 //! is managed by the class, and should not be freed.
 //!
 virtual unsigned char *GetImage(size_t ts, size_t &width, size_t &height) = 0;


 //! Fetch a georeferenced image
 //!
 //! \param[in] ts time step 
 //! \param[in] pcsExtentsReq A four-element array containing the 
 //! extents (llx, lly, urx, ury) of the 
 //! requested region in Projected Coordinates.
 //!
 //! \param[in] proj4StringReq The Proj4 string that maps lat-long coordinates
 //! into the PCS coordinates used in \p pcsExtentsReq.
 //!
 //! \param[in] maxWidthReq The requested maximum width in pixels of the 
 //! returned image
 //! \param[in] maxHeightReq The requested maximum height in pixels of the 
 //! returned image
 //! 
 //! \param[out] pcsExtentsImg A four-element array containing the 
 //! extents (llx, lly, urx, ury) of the 
 //! returned image map in Projected Coordinates of the *image*.
 //!
 //! \param[out] geoCornersImg An eight-element array containing the 
 //! corner points (llx, lly, ulx, uly, urx, ury, lrx, lry) of the 
 //! returned image map in Geographic coordinates
 //!
 //! \param[out] width The actual width in pixels of the 
 //! returned image
 //! \param[out] height The actual  height in pixels of the 
 //! returned image
 //!
 //! \retval image Upon success at 2D texture with n channels 
 //! (See GeoImage::GeoImage()) is returned.  Memory for the returned image
 //! is managed by the class, and should not be freed.
 //
 virtual unsigned char *GetImage(
	size_t ts, const double pcsExtentsReq[4], string proj4StringReq,
	size_t maxWidthReq, size_t maxHeightReq,  
	double pcsExtentsImg[4], double geoCornersImg[8], string &proj4StringImg,
	size_t &width, size_t &height
 ) = 0;

protected:

 int _pixelsize;
 int _nbands;

 // Support routines for reading tiff images
 //
 int TiffOpen(string path);
 void TiffClose();

 int TiffGetImageDimensions(
    int dirnum, size_t &width, size_t &height
 ) const;

 int TiffReadImage(int dirnum, unsigned char* texture) const;

 TIFF* TiffGetHandle() const {return (_tif);}

 int CornerExtents(
	const double srccoords[4], double dstcoords[4],
    string proj4src
 ) const;

private:
 TIFF* _tif;
 string _path;

};
};

#endif

