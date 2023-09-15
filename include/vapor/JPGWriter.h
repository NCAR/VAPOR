#pragma once

#include "vapor/ImageWriter.h"
#include <stdio.h>

namespace VAPoR {
class RENDER_API JPGWriter : public ImageWriter {
    FILE *fp;

public:
    static int DefaultQuality;
    int        Quality;

    JPGWriter(const std::string &path);
    ~JPGWriter();

    static std::vector<std::string> GetFileExtensions();
    int                             Write(const unsigned char *buffer, const unsigned int width, const unsigned int height);
};
}    // namespace VAPoR
