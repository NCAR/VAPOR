#include <osgl/GLContext.h>
#include <osgl/GLContextProviderUtil.h>

using namespace OSGL;

String GLContext::GetVersion()
{
    MakeCurrent();
    return GLContextProviderUtil::GetGLVersion();
}
