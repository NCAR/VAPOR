#include "GLContextProviderMesa.h"

#if Linux
    #include <GL/osmesa.h>

GLContextProviderMesa::GLContextMesa::GLContextMesa(void *ctx) : _ctx(ctx) {}

GLContextProviderMesa::GLContextMesa::~GLContextMesa()
{
    OSMesaContext ctx = (OSMesaContext)_ctx;
    OSMesaDestroyContext(ctx);
}

void GLContextProviderMesa::GLContextMesa::MakeCurrent()
{
    OSMesaContext ctx = (OSMesaContext)_ctx;
    bool          success = OSMesaMakeCurrent(ctx, _dummyBuffer, GL_UNSIGNED_BYTE, 4, 4);
    assert(success);
}

GLContext *GLContextProviderMesa::CreateContext()
{
    //  Attributes                    Values
    //  --------------------------------------------------------------------------
    //  OSMESA_FORMAT                 OSMESA_RGBA*, OSMESA_BGRA, OSMESA_ARGB, OSMESA_RGB etc.
    //  OSMESA_DEPTH_BITS             0*, 16, 24, 32
    //  OSMESA_STENCIL_BITS           0*, 8
    //  OSMESA_ACCUM_BITS             0*, 16
    //  OSMESA_PROFILE                OSMESA_COMPAT_PROFILE*, OSMESA_CORE_PROFILE
    //  OSMESA_CONTEXT_MAJOR_VERSION  1*, 2, 3
    //  OSMESA_CONTEXT_MINOR_VERSION  0+
    //
    //  Note: * = default value
    int attrs[] = {OSMESA_FORMAT, OSMESA_RGB, OSMESA_PROFILE, OSMESA_CORE_PROFILE, 0};

    OSMesaContext ctx = OSMesaCreateContextAttribs(attrs, NULL);
    if (!ctx) {
        LogWarning("Failed to create context\n");
        return nullptr;
    }

    GLContextMesa *glContext = new GLContextMesa(ctx);

    LogInfo("Created context: %s", glContext->GetVersion().c_str());

    return glContext;
}

#endif
