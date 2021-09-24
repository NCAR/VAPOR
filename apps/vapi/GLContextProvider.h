#pragma once

#include "GLContext.h"

//! \class GLContextProvider
//! \ingroup HeadlessGL
//! \brief Interface for creating an OpenGL context.
//! \author Stas Jaroszynski

class GLContextProvider {
public:
    static GLContext *CreateContext();
    static bool IsCurrentOpenGLVersionSupported();
private:
    static bool isContextOk(GLContext *ctx);
};
