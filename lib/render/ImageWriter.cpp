#include "vapor/ImageWriter.h"
#include "vapor/FileUtils.h"
#include "vapor/PNGWriter.h"
#include "vapor/TIFWriter.h"
#include "vapor/JPGWriter.h"

using namespace VAPoR;
using namespace Wasp;

std::vector<ImageWriterFactory *> ImageWriter::factories;

ImageWriter::ImageWriter(const std::string &path) : format(Format::RGB), path(path), opened(false) {}

ImageWriter *ImageWriter::CreateImageWriterForFile(const std::string &path)
{
    std::string pathExtension = FileUtils::Extension(path);
    for (auto factory = factories.begin(); factory != factories.end(); ++factory)
        for (auto writerExtension = (*factory)->Extensions.begin(); writerExtension != (*factory)->Extensions.end(); ++writerExtension)
            if (*writerExtension == pathExtension) return (*factory)->Create(path);

    SetErrMsg("Unsupported image file type \"%s\"", FileUtils::Extension(path).c_str());
    return nullptr;
}

void ImageWriter::RegisterFactory(ImageWriterFactory *factory) { factories.push_back(factory); }
