#ifndef DEBUG_H
#define DEBUG_H

#include <signal.h>
#include <stdio.h>

#define DLOG_BASENAME_ONLY 1

#ifdef WIN32
#undef DLOG_BASENAME_ONLY
#endif

#if DLOG_BASENAME_ONLY 
#include <libgen.h>
#define dLog_pre() fprintf(stderr, "[%s:%i:%s] ", basename(__FILE__), __LINE__, __func__)
#else
#define dLog_pre() fprintf(stderr, "[%s:%i:%s] ", __FILE__, __LINE__, __func__)
#endif

#ifdef NDEBUG
#define dLog(...)
#define dBreak()
#define dBreakIf(x)
#define dBreakCount(x)
#else
#define dLog(...) { dLog_pre(); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); }
#define dBreak() { dLog("Breakpoint"); raise(SIGTRAP); }
#define dBreakIf(x) { if (x) { dBreak(); } }
#define dBreakCount(x) { static int count = 0; if (++count == (x)) dBreak(); }
#endif

#endif
