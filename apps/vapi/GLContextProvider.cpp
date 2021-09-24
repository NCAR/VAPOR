#include "GLContextProvider.h"
#include "GLContextProviderMacOS.h"
#include "GLContextProviderMesa.h"
#include "GLContextProviderEGL.h"
#include "GLContextProviderNvidia.h"
#include "GLUtil.h"
#include "VPCommon.h"

GLContext *GLContextProvider::CreateContext()
{
#define returnIfSupportedContext(ctxProvider) {\
    GLContext *ctx = ctxProvider::CreateContext(); \
    if (isContextOk(ctx)) return ctx; \
}
#if MacOS
    returnIfSupportedContext(GLContextProviderMacOS);
#endif
    
#if Linux
    returnIfSupportedContext(GLContextProviderNvidia);
    returnIfSupportedContext(GLContextProviderEGL);
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
    GLUtil::GetGLVersion(&major, &minor);

    int version = major * 100 + minor;

    if (version >= 303) return true;
    return false;
}
