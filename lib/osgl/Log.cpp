#include <vapor/Log.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include <algorithm>
#endif

bool Log::InfoLevelEnabled = true;

void Log::printc(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

void Log::fatal() { exit(1); }

String Log::AddLocationToFormat(const String &fmt, const char *file, int line)
{
    const char *base = strrchr(file, '/');
    base = base ? base + 1 : file;
    int extraPadding = std::max(4 - (int)std::to_string(line).size(), 0);
    return "[" + String(base) + ":" + std::to_string(line) + "]" + std::string(extraPadding, ' ') + fmt;
}
