#pragma once

#include <vapor/NonCopyableMixin.h>
#include <vapor/Texture.h>

namespace VAPoR {
class Framebuffer : private NonCopyableMixin {
    unsigned int _id = 0;
    int _width = 0;
    int _height = 0;
    bool _hasDepthBuffer = false;
    Texture2D _colorBuffer;
    Texture2D _depthBuffer;

  public:
    ~Framebuffer();

    int Generate();
    bool Initialized() const;
    bool IsComplete() const;
    void Bind();
    void UnBind();
    void SetSize(int width, int height);
    int MakeRenderTarget();
    void EnableDepthBuffer();

    const Texture2D *GetColorTexture() const;
    const Texture2D *GetDepthTexture() const;
};
}; // namespace VAPoR
