#include <vapor/Framebuffer.h>
#include <vapor/glutil.h>
#include <cassert>

using namespace VAPoR;

Framebuffer::~Framebuffer()
{
    if (_id) glDeleteFramebuffers(1, &_id);
    _id = 0;
}

int Framebuffer::Generate()
{
    assert(!Initialized());
    
    glGenFramebuffers(1, &_id);
    glBindFramebuffer(GL_FRAMEBUFFER, _id);
    
    _colorBuffer.Generate();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorBuffer._id, 0);
    
    if (_hasDepthBuffer) {
        _depthBuffer.Generate();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, _depthBuffer._id, 0);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return 0;
}

bool Framebuffer::Initialized() const
{
    return _id;
}

bool Framebuffer::IsComplete() const
{
    return Initialized() && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

void Framebuffer::Bind()
{
    assert(Initialized());
    glBindFramebuffer(GL_FRAMEBUFFER, _id);
}

void Framebuffer::UnBind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::SetSize(int width, int height)
{
    assert(Initialized());
    
    if (width != _width || height != _height) {
        _width = width;
        _height = height;
        
        _colorBuffer.TexImage(GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        if (_hasDepthBuffer)
            _depthBuffer.TexImage(GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }
}

int Framebuffer::MakeRenderTarget()
{
    Bind();
    assert(IsComplete());
    if (!IsComplete()) {
        UnBind();
        return -1;
    }
    
    glViewport(0, 0, _width, _height);
    return 0;
}

void Framebuffer::EnableDepthBuffer()
{
    assert(!Initialized());
    _hasDepthBuffer = true;
}

const Texture2D *Framebuffer::GetColorTexture() const
{
    assert(Initialized());
    return &_colorBuffer;
}

const Texture2D *Framebuffer::GetDepthTexture() const
{
    assert(Initialized());
    if (!_hasDepthBuffer) {
        assert(!"Depth buffer requested from Framebuffer that does not have one");
        return nullptr;
    }
    return &_depthBuffer;
}
