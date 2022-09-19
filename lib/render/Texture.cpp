#include <vapor/Texture.h>
#include <vapor/glutil.h>
#include <vapor/MyBase.h>
#include <vapor/VAssert.h>

using namespace VAPoR;

Texture::Texture(unsigned int type) : _type(type), _nDims(GetDimsCount(type)) {}

Texture::~Texture() { Delete(); }

int Texture::Generate() { return Generate(GL_LINEAR); }

int Texture::Generate(int filter)
{
    VAssert(!Initialized());
    glGenTextures(1, &_id);
    glBindTexture(_type, _id);

    glTexParameteri(_type, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(_type, GL_TEXTURE_MAG_FILTER, filter);

    if (_nDims >= 1) glTexParameteri(_type, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    if (_nDims >= 2) glTexParameteri(_type, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (_nDims >= 3) glTexParameteri(_type, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(_type, 0);
    return 0;
}

void Texture::Delete()
{
    if (_id) glDeleteTextures(1, &_id);
    _id = 0;
    _width = 0;
    _height = 0;
    _depth = 0;
}

bool Texture::Initialized() const { return _id; }

void Texture::Bind() const
{
    VAssert(Initialized());
    glBindTexture(_type, _id);
}

void Texture::UnBind() const { glBindTexture(_type, 0); }

// glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, dims[0], dims[1], dims[2], 0, GL_RED, GL_FLOAT, data);
int Texture::TexImage(int internalFormat, int width, int height, int depth, unsigned int format, unsigned int type, const void *data, int level)
{
    VAssert(Initialized());
    VAssert(_nDims >= 2 || height == 0);
    VAssert(_nDims >= 3 || depth == 0);

    if (_nDims == 3) {
        int max3DTexDim;
        glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max3DTexDim);
        if (width > max3DTexDim || height > max3DTexDim || depth > max3DTexDim) {
            Wasp::MyBase::SetErrMsg("Texture size (%lix%lix%li) not supported by GPU (max supported size per dim is %i)\n", width, height, depth, max3DTexDim);
            return -1;
        }
    } else {
        int max2DTexDim;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max2DTexDim);
        if (width > max2DTexDim || height > max2DTexDim) {
            Wasp::MyBase::SetErrMsg("Texture size (%lix%li) not supported by GPU (max supported size per dim is %i)\n", width, height, max2DTexDim);
            return -1;
        }
    }

    _width = width;
    _height = height;
    _depth = depth;

    Bind();

    if (_nDims == 1)
        glTexImage1D(_type, level, internalFormat, width, 0, format, type, data);
    else if (_nDims == 2)
        glTexImage2D(_type, level, internalFormat, width, height, 0, format, type, data);
    else if (_nDims == 3) {
        glTexImage3D(_type, level, internalFormat, 0, 0, 0, 0, format, type, NULL);    // Fix driver bug with re-uploading large textures
        glTexImage3D(_type, level, internalFormat, width, height, depth, 0, format, type, data);
    }

    UnBind();
    return 0;
}

unsigned int Texture::GetDimsCount(unsigned int glTextureEnum)
{
    switch (glTextureEnum) {
    case GL_TEXTURE_1D: return 1;
    case GL_TEXTURE_2D: return 2;
    case GL_TEXTURE_3D: return 3;
    case GL_TEXTURE_2D_ARRAY: return 3;

    default: VAssert(0); return -1;
    }
}

Texture1D::Texture1D() : Texture(GL_TEXTURE_1D) {}
Texture2D::Texture2D() : Texture(GL_TEXTURE_2D) {}
Texture3D::Texture3D() : Texture(GL_TEXTURE_3D) {}
Texture2DArray::Texture2DArray() : Texture(GL_TEXTURE_2D_ARRAY) {}

void Texture2D::CopyDepthBuffer()
{
    GLint viewport[4] = {0};
    glGetIntegerv(GL_VIEWPORT, viewport);

    Bind();
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, viewport[0], viewport[1], viewport[2], viewport[3], 0);

    _width = viewport[2] - viewport[0];
    _height = viewport[3] - viewport[1];

    UnBind();
}
