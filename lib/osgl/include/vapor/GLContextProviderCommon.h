#pragma once

#include <vapor/common.h>
#include <cassert>
#include <string>
#include <vector>

typedef std::string String;
using std::vector;

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

#include "Log.h"
