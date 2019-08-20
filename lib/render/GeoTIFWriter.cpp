#include "vapor/GeoTIFWriter.h"
#include "vapor/Proj4StringParser.h"
#include "vapor/VAssert.h"

#ifdef WIN32
#include <geotiff/geo_normalize.h>
#include <geotiff/xtiffio.h>
#else
#include <geo_normalize.h>
#endif

using namespace VAPoR;

GeoTIFWriter::GeoTIFWriter(const std::string &path)
: TIFWriter(path), gtif(nullptr),
_hasTiePoint(false), _hasPixelScale(false),
_geoTiffWasConfigured(false)
{
    if (opened) {
        gtif = GTIFNew(tif);
        if (!gtif)
            opened = false;
    }
}

GeoTIFWriter::~GeoTIFWriter()
{
    if (gtif)
        GTIFFree(gtif);
}

int GeoTIFWriter::Write(const unsigned char *buffer, const unsigned int width, const unsigned int height)
{
    VAssert(_hasTiePoint);
    VAssert(_hasPixelScale);
    VAssert(_geoTiffWasConfigured);
    
    GTIFWriteKeys(gtif);
    return TIFWriter::Write(buffer, width, height);
}

int GeoTIFWriter::ConfigureFromProj4(const std::string proj4String)
{
    Proj4StringParser proj(proj4String);
    
    if (proj.GetString("ellps") == "sphere") {
        SetErrMsg("Arbitrary sphere projections not supported");
        return -1;
    }
    
    if (proj.GetString("proj") == "lcc" || proj.GetString("proj") == "utm") {
        if (GTIFSetFromProj4(gtif, proj4String.c_str()) == 0) {
            SetErrMsg("Unable to configure GeoTIFF using GTIFSetFromProj4(%s)", proj4String.c_str());
            return -1;
        }
    } else {
        if (proj.GetString("proj") == "merc") {
            GTIFKeySet(gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1, CT_Mercator);
            GTIFKeySet(gtif, ProjScaleAtNatOriginGeoKey, TYPE_DOUBLE, 1, proj.GetDouble("k", 1.0));
        }
        else if (proj.GetString("proj") == "eqc") {
            GTIFKeySet(gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1, CT_Equirectangular);
        }
        else if (proj.GetString("proj") == "stere") {
            GTIFKeySet(gtif, ProjCoordTransGeoKey, TYPE_SHORT, 1, CT_Stereographic);
            if (proj.HasKey("R")) {
                SetErrMsg("Arbitrary sphere projections not supporteds");
                return -1;
            }
        }
        else {
            SetErrMsg("Unsupported projection \"%s\"", proj.GetString("proj").c_str());
            return -1;
        }
        
        int ellipse = Proj4StringParser::Proj4EllipseStringToGeoTIFEnum(proj.GetString("ellps", "WGS84"));
        if (ellipse < 0) return -1;
        GTIFKeySet(gtif, GeogEllipsoidGeoKey, TYPE_SHORT, 1, ellipse);
        
        GTIFKeySet(gtif, GTModelTypeGeoKey, TYPE_SHORT, 1, ModelTypeProjected);
        GTIFKeySet(gtif, ProjectedCSTypeGeoKey, TYPE_SHORT, 1, KvUserDefined);
        GTIFKeySet(gtif, ProjectionGeoKey, TYPE_SHORT, 1, KvUserDefined);
        
        // Proj4 does not support lat_0 but some of our proj4 strings have this parameter in them
        // In the datasets I've tested, it appears to override lat_ts..?
        if (proj.HasKey("lat_0")) GTIFKeySet(gtif, ProjNatOriginLatGeoKey, TYPE_DOUBLE, 1, proj.GetDouble("lat_0",  0.0));
        else                      GTIFKeySet(gtif, ProjNatOriginLatGeoKey, TYPE_DOUBLE, 1, proj.GetDouble("lat_ts", 0.0));
        
        GTIFKeySet(gtif, ProjNatOriginLongGeoKey, TYPE_DOUBLE, 1, proj.GetDouble("lon_0", 0.0));
        GTIFKeySet(gtif, ProjFalseEastingGeoKey, TYPE_DOUBLE, 1, proj.GetDouble("x_0", 0.0));
        GTIFKeySet(gtif, ProjFalseNorthingGeoKey, TYPE_DOUBLE, 1, proj.GetDouble("y_0", 0.0));
    }
    
    _geoTiffWasConfigured = true;
    return 0;
}

void GeoTIFWriter::SetTiePoint(double worldX, double worldY, double rasterX, double rasterY)
{
    // http://geotiff.maptools.org/spec/geotiff2.6.html#2.6
    double tiePoints[6]={
        rasterX, rasterY, 0, // 3D I,J,K Raster coordinate space
        worldX,  worldY,  0  // 3D X,Y,Z World coordinate space
    };
    TIFFSetField(tif, TIFFTAG_GEOTIEPOINTS, 6, tiePoints);
    _hasTiePoint = true;
}

void GeoTIFWriter::SetPixelScale(double x, double y)
{
    double pixelScale[3] = {x,y,0};
    TIFFSetField(tif, TIFFTAG_GEOPIXELSCALE, 3, pixelScale);
    _hasPixelScale = true;
}
