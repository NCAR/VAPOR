#include <osgl/GLContextProvider.h>
#include <osgl/GLContextProviderMacOS.h>
#include <osgl/GLContextProviderMesa.h>
#include <osgl/GLContextProviderEGL.h>
#include <osgl/GLContextProviderNvidia.h>
#include <osgl/GLContextProviderUtil.h>

using namespace OSGL;

GLContext *GLContextProvider::CreateContext()
{
#define returnIfSupportedContext(ctxProvider)          \
    {                                                  \
        GLContext *ctx = ctxProvider::CreateContext(); \
        if (isContextOk(ctx))                          \
            return ctx;                                \
        else if (ctx)                                  \
            delete ctx;                                \
    }
#if MacOS
    returnIfSupportedContext(GLContextProviderMacOS);
#endif

#if BUILD_EGL
    returnIfSupportedContext(GLContextProviderNvidia);
    returnIfSupportedContext(GLContextProviderEGL);
#endif

#if BUILD_MESA
    LogWarning("Could not get an OpenGL context from the display manager. Is one running?");
    LogWarning("Falling back to software rendering");
    returnIfSupportedContext(GLContextProviderMesa);
#endif

    LogWarning("Could not create OpenGL context");
    return nullptr;
}

bool GLContextProvider::isContextOk(GLContext *ctx)
{
    if (ctx == nullptr) return false;
    ctx->MakeCurrent();
    return IsCurrentOpenGLVersionSupported();
}

bool GLContextProvider::IsCurrentOpenGLVersionSupported()
{
    int major, minor;
    GLContextProviderUtil::GetGLVersion(&major, &minor);

    int version = major * 100 + minor;

    if (version >= 303) return true;
    return false;
}
