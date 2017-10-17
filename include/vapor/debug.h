#ifndef DEBUG_H
#define DEBUG_H

#include <signal.h>
#include <stdio.h>

#ifdef NDEBUG
    #define dLog(...)
    #define dBreak()
    #define dBreakIf(x)
#else
    #define dLog(...)                                                     \
        {                                                                 \
            fprintf(stderr, "[%s:%i:%s] ", __FILE__, __LINE__, __func__); \
            fprintf(stderr, __VA_ARGS__);                                 \
            fprintf(stderr, "\n");                                        \
        }
    #define dBreak()            \
        {                       \
            dLog("Breakpoint"); \
            raise(SIGTRAP);     \
        }
    #define dBreakIf(x)          \
        {                        \
            if (x) { dBreak(); } \
        }
#endif

#endif
