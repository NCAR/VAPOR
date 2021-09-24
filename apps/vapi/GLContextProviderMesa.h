#pragma once
#include "VPCommon.h"
#include "GLContext.h"

//! \class GLContextProviderMesa
//! \ingroup HeadlessGL
//! \brief Uses OSMesa for headless OpenGL Context creation.
//! \author Stas Jaroszynski

class GLContextProviderMesa {
    
    class GLContextMesa : public GLContext {
        void *_ctx = nullptr;
        char _dummyBuffer[4*4*3];
    public:
        GLContextMesa(void *ctx);
        virtual ~GLContextMesa() override;
        virtual void MakeCurrent() override;
    };
    
public:
    static GLContext *CreateContext();
};
