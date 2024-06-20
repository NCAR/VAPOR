#include <iostream>
//#include <vapor/MyBase.h>

//#include <QApplication>
//#include <QLabel>

//#include <openssl/opensslv.h>
//#include <openssl/crypto.h>
#include <png.h>
#include <jpeglib.h>
#include <tiffio.h>
#include <sqlite3.h>
#include <libssh/libssh.h>
#include <curl/curl.h>
#include <proj.h>
#include <geotiff.h>
#include <zlib.h>
#include <assimp/version.h>
#include <szlib.h>
#include <expat.h>
#include <udunits2.h>
#include <freetype2/ft2build.h>
#include FT_FREETYPE_H
//#include <ospray.h>

#include <hdf5.h>
#include <netcdf.h>

//#include <vapor/MyPython.h>
//#include <vapor/CFuncs.h>
#include <vapor/FileUtils.h> //common
//#include <vapor/ResourcePath.h>
#include <vapor/WASP.h> // wasp
#include <vapor/VDCNetCDF.h> // vdc
#include <vapor/DataStatus.h> // params
#include <vapor/Advection.h> //flow
#include <vapor/GLContextProvider.h> //osgl

#include <QApplication>
#include <QLabel>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QLabel label("testApp Label");
    label.show();

    //const char* version = OpenSSL_version(OPENSSL_VERSION);
    //std::cout << "OpenSSL library version: " << version << std::endl; 

    unsigned majnum, minnum, relnum;
    H5get_libversion(&majnum, &minnum, &relnum);
    std::cout << "THIRD PARTY LIBRARIES" << std::endl;
    std::cout << "    HDF5 library version: " << majnum << "." << minnum << "." << relnum << std::endl;
    std::cout << "    NetCDF library version: " << nc_inq_libvers() << std::endl;

    std::cout << png_get_header_version(nullptr) << std::endl;
    std::cout << "    libjpeg version: " << JPEG_LIB_VERSION << std::endl;
    std::cout << "    libtiff version: " << TIFFGetVersion() << std::endl;
    std::cout << "    sqlite version:  " << sqlite3_libversion() << std::endl;
    ssh_init(); std::cout << "    ssh version: " << ssh_version(0) << std::endl;
    curl_global_init(CURL_GLOBAL_ALL); std::cout << "    curl version " << curl_version() << std::endl; curl_global_cleanup();
    std::cout << "    proj version " << pj_release << std::endl;
    std::cout << "    geotiff version " << LIBGEOTIFF_VERSION << std:: endl;
    std::cout << "    zlib version " << zlibVersion() << std:: endl;
    std::cout << "    Assimp version: " << aiGetLegalString() << std::endl; 
    //std::cout << "    szip version: " << SZLIB_VERSION << std::endl; 
    std::cout << "    expat version: " << XML_ExpatVersion() << std::endl;
    std::cout << "    udunits test: " << UT_SUCCESS << std::endl;
    FT_Library  library; int error = FT_Init_FreeType( &library ); std::cout << "    freetype test " << error << std::endl;
    //std::cout << "    ospray test " << ospGetCurrentDevice() << std::endl;

    std::cout << std::endl << std::endl << "VAPOR LIBRARIES" << std::endl;
    //std::cout << "    python test " <<  Wasp::MyPython::Instance()->Initialize() << std::endl;

    std::cout << "    common test " <<  Wasp::FileUtils::HomeDir() << std::endl;
    VAPoR::WASP* w = new VAPoR::WASP(1);
    std::cout << "    wasp test " <<  w->Open("foo", 1) << std::endl;
    delete w;

    VAPoR::VDCNetCDF* v = new VAPoR::VDCNetCDF;
    std::vector<std::string> p;
    std::cout << "    vdc test " <<  v->Initialize(p) << std::endl;
    delete v;

    VAPoR::DataStatus* d = new VAPoR::DataStatus;
    std::cout << "    params test " <<  d->GetNumThreads() << std::endl;
    delete d;

    flow::Advection* a = new flow::Advection();
    std::cout << "    flow test " <<  a->GetNumberOfStreams() << std::endl;

    GLContext* c = GLContextProvider::CreateContext();
    std::cout << "    osgl test " << c->GetVersion() << std::endl;

    return app.exec();
}

/*
openssl - doesn't link?
- libpng
- jpeg
- tiff
- sqlite
- ssh
- curl
- proj
- geotiff
- zlib
pythonVapor
- assimp
- szip
- hdf5
- netcdf
- expat
- udunits
- freetype
if [ "$OS" == "Ubuntu" ] ; then
   xinerama
fi
- ospray
NA glm
NA gte
NA images
- qt
if [ "$OS" == "macOSx86" ] || [ "$OS" == "M1" ]; then
    add_rpath
fi
erenameAndCompress*/
