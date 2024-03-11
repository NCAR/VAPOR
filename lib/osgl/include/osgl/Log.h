#pragma once

#include <osgl/GLContextProviderCommon.h>

namespace OSGL {

class OSGL_API Log {
    template<typename... Args> static void print(String fmt, Args... args)
    {
        fmt += "\n";
        printc(fmt.c_str(), args...);
    }

    static void printc(const char *format, ...);
    static void fatal();

public:
    static bool InfoLevelEnabled;
    
    static String AddLocationToFormat(const String &fmt, const char *file, int line);

    template<typename... Args> static void Fatal(const String &fmt, Args... args)
    {
        print(fmt, args...);
        fatal();
    }

    template<typename... Args> static void Message(const String &fmt, Args... args) { print(fmt, args...); }

    template<typename... Args> static void Info(const String &fmt, Args... args)
    {
        if (InfoLevelEnabled)
            print(fmt, args...);
    }

    template<typename... Args> static void Warning(const String &fmt, Args... args) { print(fmt, args...); }
};

#define _Log(type, fmt, ...) Log::type(Log::AddLocationToFormat(fmt, __FILE__, __LINE__), ##__VA_ARGS__)
#define LogFatal(fmt, ...)   _Log(Fatal, fmt, ##__VA_ARGS__)
#define LogMessage(fmt, ...) _Log(Message, fmt, ##__VA_ARGS__)
#define LogInfo(fmt, ...)    _Log(Info, fmt, ##__VA_ARGS__)
#define LogWarning(fmt, ...) _Log(Warning, fmt, ##__VA_ARGS__)

}
