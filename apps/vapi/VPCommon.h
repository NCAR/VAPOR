#pragma once

#include <cassert>
#include <string>
#include <vector>

typedef std::string String;
using std::vector;

namespace VAPoR {
class ControlExec;
class ParamsMgr;
struct GLManager;
class Framebuffer;
class ViewpointParams;
} using namespace VAPoR;
class GUIStateParams;
class AnimationParams;

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
