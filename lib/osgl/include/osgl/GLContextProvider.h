#pragma once

#include <osgl/GLContext.h>

//! \class GLContextProvider
//! \ingroup HeadlessGL
//! \brief Interface for creating an OpenGL context.
//! \author Stas Jaroszynski

namespace OSGL {

class OSGL_API GLContextProvider {
public:
    //! Creates an OpenGL context. The returned pointer is optional for managing
    //! multiple contexts.
    static GLContext *CreateContext();
    static bool       IsCurrentOpenGLVersionSupported();

private:
    static bool isContextOk(GLContext *ctx);
};

}
