#pragma once

#include "GLContext.h"

//! \class GLContextProviderMacOS
//! \ingroup HeadlessGL
//! \brief Creates an OpenGL context using macOS's libraries.
//! \author Stas Jaroszynski

class GLContextProviderMacOS {
    class GLContextMacOS : public GLContext {
        void *_ctx = nullptr;

    public:
        GLContextMacOS(void *ctx);
        virtual ~GLContextMacOS() override;
        virtual void MakeCurrent() override;

        friend class GLContextProviderMacOS;
    };

public:
    static GLContext *CreateContext();
};
