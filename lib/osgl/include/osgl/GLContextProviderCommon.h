#pragma once

#include <cassert>
#include <string>
#include <vector>


namespace OSGL {
typedef std::string String;
using std::vector;
}

#undef MacOS
#undef Linux
#undef Windows
#if __APPLE__
    #define MacOS 1
#elif __linux__
    #define Linux 1
#elif WIN32
    #define Windows 1
#endif

// #include <osgl/common.h>
#ifdef WIN32
    #ifdef OSGL_EXPORTS
        #define OSGL_API __declspec(dllexport)
    #else
        #define OSGL_API __declspec(dllimport)
    #endif
#else
    #define OSGL_API
#endif

#include "Log.h"
