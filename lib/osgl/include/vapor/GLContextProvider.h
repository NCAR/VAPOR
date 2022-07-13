#pragma once

#include <vapor/GLContext.h>

//! \class GLContextProvider
//! \ingroup HeadlessGL
//! \brief Interface for creating an OpenGL context.
//! \author Stas Jaroszynski

class GLContextProvider {
public:
    //! Creates an OpenGL context. The returned pointer is optional for managing
    //! multiple contexts.
    static GLContext *CreateContext();
    static bool       IsCurrentOpenGLVersionSupported();

private:
    static bool isContextOk(GLContext *ctx);
};
