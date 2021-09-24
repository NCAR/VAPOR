#pragma once

#include "GLContextProviderEGL.h"

//! \class GLContextProviderNvidia
//! \ingroup HeadlessGL
//! \brief Implements nvidia's API for headless OpenGL Context creation.
//! \author Stas Jaroszynski

class GLContextProviderNvidia : private GLContextProviderEGL {
public:
    static GLContext *CreateContext();
};
