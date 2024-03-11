#pragma once

#include <osgl/GLContextProviderEGL.h>

//! \class GLContextProviderNvidia
//! \ingroup HeadlessGL
//! \brief Implements nvidia's API for headless OpenGL Context creation.
//! \author Stas Jaroszynski

namespace OSGL {

class OSGL_API GLContextProviderNvidia : private GLContextProviderEGL {
public:
    static GLContext *CreateContext();
};

}
