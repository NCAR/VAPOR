#include <vapor/Framebuffer.h>
#include <vapor/MyBase.h>
#include <vapor/glutil.h>
#include "vapor/VAssert.h"

using namespace VAPoR;

Framebuffer::~Framebuffer()
{
    if (_id) glDeleteFramebuffers(1, &_id);
    _id = 0;
}

int Framebuffer::Generate()
{
    VAssert(!Initialized());

    glGenFramebuffers(1, &_id);
    glBindFramebuffer(GL_FRAMEBUFFER, _id);

    _colorBuffer.Generate();
    _colorBuffer.TexImage(GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorBuffer._id, 0);

    if (_hasDepthBuffer) {
        _depthBuffer.Generate();
        _depthBuffer.TexImage(GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthBuffer._id, 0);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}

bool Framebuffer::Initialized() const { return _id; }

bool Framebuffer::IsComplete() const { return Initialized() && GetStatus() == GL_FRAMEBUFFER_COMPLETE; }

int Framebuffer::GetStatus() const { return glCheckFramebufferStatus(GL_FRAMEBUFFER); }

const char *Framebuffer::GetStatusString() const
{
    switch (GetStatus()) {
    case GL_FRAMEBUFFER_COMPLETE: return "GL_FRAMEBUFFER_COMPLETE";
    case GL_FRAMEBUFFER_UNDEFINED: return "GL_FRAMEBUFFER_UNDEFINED";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
    case GL_FRAMEBUFFER_UNSUPPORTED: return "GL_FRAMEBUFFER_UNSUPPORTED";
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
    case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
    case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
    default: return "Unknown GL enum";
    }
}

void Framebuffer::Bind()
{
    VAssert(Initialized());
    glBindFramebuffer(GL_FRAMEBUFFER, _id);
}

void Framebuffer::UnBind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

void Framebuffer::SetSize(int width, int height)
{
    VAssert(Initialized());

    width = width <= 0 ? 1 : width;
    height = height <= 0 ? 1 : height;

    if (width != _width || height != _height) {
        _width = width;
        _height = height;

        _colorBuffer.TexImage(GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        if (_hasDepthBuffer) _depthBuffer.TexImage(GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
}

void Framebuffer::GetSize(int *width, int *height) const
{
    *width = _width;
    *height = _height;
}

int Framebuffer::MakeRenderTarget()
{
    Bind();
    if (!IsComplete()) {
        Wasp::MyBase::SetErrMsg("Incomplete framebuffer: %s\n", GetStatusString());
        UnBind();
        VAssert(false);
        return -1;
    }

    glViewport(0, 0, _width, _height);
    return 0;
}

void Framebuffer::EnableDepthBuffer()
{
    VAssert(!Initialized());
    _hasDepthBuffer = true;
}

const Texture2D *Framebuffer::GetColorTexture() const
{
    VAssert(Initialized());
    return &_colorBuffer;
}

const Texture2D *Framebuffer::GetDepthTexture() const
{
    VAssert(Initialized());
    if (!_hasDepthBuffer) {
        VAssert(!"Depth buffer requested from Framebuffer that does not have one");
        return nullptr;
    }
    return &_depthBuffer;
}
