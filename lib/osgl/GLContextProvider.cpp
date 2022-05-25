#include <vapor/GLContextProvider.h>
#include <vapor/GLContextProviderMacOS.h>
#include <vapor/GLContextProviderMesa.h>
#include <vapor/GLContextProviderEGL.h>
#include <vapor/GLContextProviderNvidia.h>
#include <vapor/GLContextProviderUtil.h>

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

#if Linux
#if BUILD_EGL
    returnIfSupportedContext(GLContextProviderNvidia);
    returnIfSupportedContext(GLContextProviderEGL);
#endif
    LogWarning("Could not get an OpenGL context from the display manager. Is one running?");
    LogWarning("Falling back to software rendering");
    returnIfSupportedContext(GLContextProviderMesa);
#endif

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
