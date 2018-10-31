#pragma once

#include "vapor/ImageWriter.h"
#include <xtiffio.h>

namespace VAPoR {
class RENDER_API TIFWriter : public ImageWriter {
protected:
    TIFF *tif;

    int ConfigureWithFormat(Format f);

public:
    static std::vector<std::string> GetFileExtensions();

    TIFWriter(const std::string &path);
    virtual ~TIFWriter();

    virtual int Write(const unsigned char *buffer, const unsigned int width, const unsigned int height);
};
}    // namespace VAPoR
