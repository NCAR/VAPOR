#pragma once

#include "vapor/TIFWriter.h"
#ifdef WIN32
    #include <geotiff/geotiffio.h>
#else
    #include <geotiffio.h>
#endif

namespace VAPoR {

//! \class GeoTIFWriter
//! \ingroup Public_Render
//! \brief Writes a TIF image with GeoTIF metadata
//! \author Stanislaw Jaroszynski
//!
class RENDER_API GeoTIFWriter : public TIFWriter {
    GTIF *gtif;
    bool  _hasTiePoint;
    bool  _hasPixelScale;
    bool  _geoTiffWasConfigured;

public:
    GeoTIFWriter(const std::string &path);
    ~GeoTIFWriter();

    int  Write(const unsigned char *buffer, const unsigned int width, const unsigned int height);
    int  ConfigureFromProj4(const std::string proj4String);
    void SetTiePoint(double worldX, double worldY, double rasterX = 0, double rasterY = 0);
    void SetPixelScale(double x, double y);
};
}    // namespace VAPoR
