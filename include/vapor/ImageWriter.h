#pragma once

#include <vapor/MyBase.h>
#include <string>

namespace VAPoR {

class ImageWriterFactory;

class RENDER_API ImageWriter : public Wasp::MyBase {
public:
    enum class Format { RGB };

    // virtual static std::vector<std::string> GetFileExtensions();
    virtual int Write(const unsigned char *buffer, const unsigned int width, const unsigned int height) = 0;
    virtual ~ImageWriter(){};

    static ImageWriter *CreateImageWriterForFile(const std::string &path);
    static void         RegisterFactory(ImageWriterFactory *factory);

protected:
    Format      format;
    std::string path;
    bool        opened;

    ImageWriter(const std::string &path);

private:
    static std::vector<ImageWriterFactory *> factories;
};

class ImageWriterFactory {
public:
    const std::vector<std::string> Extensions;
    virtual ImageWriter *          Create(const std::string &path) = 0;

protected:
    ImageWriterFactory(std::vector<std::string> extensions) : Extensions(extensions) {}
};

#define REGISTER_IMAGEWRITER(name)                                                                              \
    class name##Factory : public ImageWriterFactory {                                                           \
    public:                                                                                                     \
        name##Factory() : ImageWriterFactory(name::GetFileExtensions()) { ImageWriter::RegisterFactory(this); } \
        virtual ImageWriter *Create(const std::string &path) { return new name(path); }                         \
    };                                                                                                          \
    static name##Factory registration_##name##Factory;

}    // namespace VAPoR
