#include "vapor/JPGWriter.h"
#include "vapor/jpegapi.h"

using namespace VAPoR;

REGISTER_IMAGEWRITER(JPGWriter);

int JPGWriter::DefaultQuality = 95;

std::vector<std::string> JPGWriter::GetFileExtensions() { return {"jpg", "jpeg"}; }

JPGWriter::JPGWriter(const std::string &path) : ImageWriter(path), fp(nullptr), Quality(DefaultQuality)
{
    fp = fopen(path.c_str(), "wb");
    if (fp) opened = true;
}

JPGWriter::~JPGWriter()
{
    if (fp) fclose(fp);
    fp = nullptr;
}

int JPGWriter::Write(const unsigned char *buffer, const unsigned int width, const unsigned int height)
{
    if (!opened) {
        MyBase::SetErrMsg("Unable to open JPG file for writing: \"%s\"", path.c_str());
        return -1;
    }

    return write_JPEG_file(fp, width, height, const_cast<unsigned char *>(buffer), Quality);
}
