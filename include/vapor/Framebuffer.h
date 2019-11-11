#pragma once

#include <vapor/NonCopyableMixin.h>
#include <vapor/Texture.h>

namespace VAPoR {

//! \class Framebuffer
//! \ingroup Public_Render
//! \brief Wrapper class for an OpenGL Framebuffer
//! \author Stas Jaroszynski
//! \version 1.0
//! \date May 2019
//!
//! This class is intended to be used as a member object for a renderer.
//! Any use of this class (including the destructor and except the constructor)
//! must occur inside the correct OpenGL context.
//!
//! To use a depth buffer, EnableDepthBuffer must be called before calling
//! Generate()
//!

class Framebuffer : private NonCopyableMixin {
    unsigned int _id = 0;
    int          _width = 0;
    int          _height = 0;
    bool         _hasDepthBuffer = false;
    Texture2D    _colorBuffer;
    Texture2D    _depthBuffer;

public:
    ~Framebuffer();

    int         Generate();
    bool        Initialized() const;
    bool        IsComplete() const;
    int         GetStatus() const;
    const char *GetStatusString() const;
    void        Bind();
    void        UnBind();
    void        SetSize(int width, int height);
    void        GetSize(int *width, int *height) const;
    int         MakeRenderTarget();
    void        EnableDepthBuffer();

    const Texture2D *GetColorTexture() const;
    const Texture2D *GetDepthTexture() const;
};
};    // namespace VAPoR
