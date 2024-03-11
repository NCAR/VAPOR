#pragma once

#include <osgl/GLContextProviderCommon.h>

//! \class GLContext
//! \ingroup HeadlessGL
//! \brief Object that abstracts an OpenGL context since they are differnet in every case.
//! \author Stas Jaroszynski

namespace OSGL {

class OSGL_API GLContext {
public:
    virtual ~GLContext() {}
    virtual void MakeCurrent() = 0;
    String       GetVersion();
};

}
