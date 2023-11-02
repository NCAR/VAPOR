#pragma once

#include <osgl/GLContextProviderCommon.h>

namespace OSGL {

class OSGL_API GLContextProviderUtil {
public:
    static String GetGLVersion();
    static void   GetGLVersion(int *major, int *minor);
};

}
