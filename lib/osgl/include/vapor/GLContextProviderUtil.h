#pragma once

#include <vapor/GLContextProviderCommon.h>

class OSGL_API GLContextProviderUtil {
public:
    static String GetGLVersion();
    static void   GetGLVersion(int *major, int *minor);
};
