#pragma once

#include "vapor/ImageWriter.h"

namespace VAPoR {
class RENDER_API PNGWriter : public ImageWriter {
protected:
public:
    PNGWriter(const string &path);
    ~PNGWriter(){};

    static std::vector<std::string> GetFileExtensions();
    int                             Write(const unsigned char *buffer, const unsigned int width, const unsigned int height);
};
}    // namespace VAPoR
