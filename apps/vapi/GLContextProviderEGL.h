#pragma once
#include "VPCommon.h"
#include "GLContext.h"

//! \class GLContextProviderEGL
//! \ingroup HeadlessGL
//! \brief Creates an OpenGL context using the EGL library.
//! \author Stas Jaroszynski

class GLContextProviderEGL {
    
    class GLContextEGL : public GLContext {
        void *_display = nullptr;
        void *_surface = nullptr;
        void *_context = nullptr;
    public:
        GLContextEGL(void *display, void *surface, void *context);
        virtual ~GLContextEGL() override;
        virtual void MakeCurrent() override;
    };
    
public:
    static GLContext *CreateContext();
    
protected:
    static GLContext *createContextForDisplay(void *display);
    static const char *stringifyEGLError(int e);
};
