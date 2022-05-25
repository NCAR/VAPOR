#include <vapor/GLContextProviderMesa.h>

#if Linux
    #include <vapor/GLAD.h>
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

void GLContextProviderMesa::GLContextMesa::DumpParameters() const
{
#define OSMPP(p) { int v; OSMesaGetIntegerv(p, &v); printf("\t%s = %i", #p, v); }
    printf("OSMesa Parameters\n");
    OSMPP(OSMESA_WIDTH);
    OSMPP(OSMESA_HEIGHT);
    OSMPP(OSMESA_FORMAT);
    OSMPP(OSMESA_TYPE);
    OSMPP(OSMESA_ROW_LENGTH);
    OSMPP(OSMESA_Y_UP);
#undef OSMPP
}

GLContext *GLContextProviderMesa::CreateContext()
{
    LogInfo("OSMesa %i.%i.%i", OSMESA_MAJOR_VERSION, OSMESA_MINOR_VERSION, OSMESA_PATCH_VERSION);
    
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
    //  Note: NULL terminated
    int attrs[] = {
        OSMESA_FORMAT, OSMESA_RGB,
        OSMESA_PROFILE, OSMESA_CORE_PROFILE,
        OSMESA_CONTEXT_MAJOR_VERSION, 3,
        OSMESA_CONTEXT_MINOR_VERSION, 3,
        NULL
    };

    OSMesaContext ctx = nullptr;
    
    ctx = OSMesaCreateContextAttribs(attrs, NULL);
    
    if (!ctx) {
        LogWarning("OSMesaCreateContextAttribs failed");
        OSMesaCreateContextExt(OSMESA_RGB, 0, 0, 0, NULL);
    }
    
    if (!ctx) {
        LogWarning("OSMesaCreateContextExt failed");
        ctx = OSMesaCreateContext(OSMESA_RGB, NULL);
    }
    
    if (!ctx) {
        LogWarning("Failed to create context");
        return nullptr;
    }

    GLContextMesa *glContext = new GLContextMesa(ctx);
    
    glContext->MakeCurrent();
    if (!gladLoadGL((GLADloadfunc)OSMesaGetProcAddress)) {
        LogWarning("Failed to load GL");
        delete glContext;
        return nullptr;
    }

    LogInfo("Created context: %s", glContext->GetVersion().c_str());

    return glContext;
}

#endif
