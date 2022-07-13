#pragma once

#include <vapor/GLContextProviderCommon.h>

class GLContextProviderUtil {
public:
    static String GetGLVersion();
    static void   GetGLVersion(int *major, int *minor);
};
