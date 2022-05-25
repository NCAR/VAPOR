#include <vapor/GLContext.h>
#include <vapor/GLContextProviderUtil.h>

String GLContext::GetVersion()
{
    MakeCurrent();
    return GLContextProviderUtil::GetGLVersion();
}
