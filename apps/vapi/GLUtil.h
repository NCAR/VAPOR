#pragma once

#include "VPCommon.h"

class GLUtil {
public:
    static String GetGLVersion();
    static void   GetGLVersion(int *major, int *minor);
};
