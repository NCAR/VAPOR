#include "GLContext.h"
#include "GLUtil.h"

String GLContext::GetVersion()
{
    MakeCurrent();
    return GLUtil::GetGLVersion();
}
