#include "vapor/TIFWriter.h"

using namespace VAPoR;

REGISTER_IMAGEWRITER(TIFWriter);

std::vector<std::string> TIFWriter::GetFileExtensions() { return {"tif", "tiff"}; }

int TIFWriter::ConfigureWithFormat(ImageWriter::Format f)
{
    TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);

    switch (f) {
    case Format::RGB:
        TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
        TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
        return 0;

    default: MyBase::SetErrMsg("Unsupported format"); return -1;
    }
}

TIFWriter::TIFWriter(const std::string &path) : ImageWriter(path), tif(nullptr)
{
    tif = XTIFFOpen(path.c_str(), "w");
    if (tif) opened = true;
}

TIFWriter::~TIFWriter()
{
    if (tif) XTIFFClose(tif);
    tif = nullptr;
}

int TIFWriter::Write(const unsigned char *buffer, const unsigned int width, const unsigned int height)
{
    if (!opened) {
        MyBase::SetErrMsg("Unable to open TIF file for writing: \"%s\"", path.c_str());
        return -1;
    }

    ConfigureWithFormat(format);

    TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
    TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
    TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_NONE);
    TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);

    int writeSuccess = TIFFWriteRawStrip(tif, 0, const_cast<unsigned char *>(buffer), width * height * 3);
    if (writeSuccess < 0) {
        SetErrMsg("TIFF write routine failed");
        return -1;
    }

    return 0;
}
