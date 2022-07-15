#pragma once

#include <vapor/GLContextProviderCommon.h>

//! \class GLContext
//! \ingroup HeadlessGL
//! \brief Object that abstracts an OpenGL context since they are differnet in every case.
//! \author Stas Jaroszynski

class GLContext {
public:
    virtual ~GLContext() {}
    virtual void MakeCurrent() = 0;
    String       GetVersion();
};
