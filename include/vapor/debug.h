#ifndef DEBUG_H
#define DEBUG_H

#ifdef NDEBUG
    #define dLog(...)
    #define dBreak()
    #define dBreakIf(x)
    #define dBreakCount(x)
    #define FTRACE(...)
    #define PERF_TIMER_START
    #define PERF_TIMER_STOP
    #define PERF_TIMER_DELTA 1
#else

    #include <signal.h>
    #include <stdio.h>
    #include <string>
    #include <vector>

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

    #define dLog(...)                     \
        {                                 \
            dLog_pre();                   \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, "\n");        \
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
    #define dBreakCount(x)                \
        {                                 \
            static int count = 0;         \
            if (++count == (x)) dBreak(); \
        }

    #if WIN32
        #define PERF_TIMER_START
        #define PERF_TIMER_STOP
        #define PERF_TIMER_DELTA 1
    #else
        #include <sys/time.h>
        #define PERF_TIMER_START                         \
            struct timeval PERF_TIMER_T1, PERF_TIMER_T2; \
            gettimeofday(&PERF_TIMER_T1, NULL);
        #define PERF_TIMER_STOP  gettimeofday(&PERF_TIMER_T2, NULL);
        #define PERF_TIMER_DELTA ((PERF_TIMER_T2.tv_usec - PERF_TIMER_T1.tv_usec) / 1000000.0 + (double)(PERF_TIMER_T2.tv_sec - PERF_TIMER_T1.tv_sec))
    #endif

using std::string;
using std::vector;

    #define VDC_LIBTRACE
    // #define VDC_LIBTRACE_RUNNABLE
    #ifdef VDC_LIBTRACE

        #define NARG(...)                                         NARG_(__VA_ARGS__, RSEQ_N())
        #define NARG_(...)                                        ARG_N(__VA_ARGS__)
        #define ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, N, ...) N
        #define RSEQ_N()                                          9, 8, 7, 6, 5, 4, 3, 2, 1, 0

void PRINTARG() {}
void PRINTARG(int x) { printf("%i", x); }
void PRINTARG(long x) { printf("%li", x); }
void PRINTARG(size_t x) { printf("%li", x); }
void PRINTARG(float x) { printf("%g", x); }
void PRINTARG(double x) { printf("%g", x); }
void PRINTARG(bool x) { printf("%s", x ? "true" : "false"); }
void PRINTARG(const std::string x) { printf("\"%s\"", x.c_str()); }
void PRINTARG(const char *x) { printf("\"%s\"", x); }
        #ifdef VDC_LIBTRACE_RUNNABLE
void PRINTARG(const int *i) { printf("(int[]){%i}", *i); }
void PRINTARG(const float *f) { printf("(float[]){%f}", *f); }
void PRINTARG(const double *d) { printf("(double[]){%d}", *d); }
void PRINTARG(const size_t *li) { printf("(size_t[]){%li}", *li); }
void PRINTARG(const long *li) { printf("(long[]){%li}", *li); }
        #else
void PRINTARG(const int *i) { printf("*[%i]", *i); }
void PRINTARG(const float *f) { printf("*[%f]", *f); }
void PRINTARG(const double *d) { printf("*[%f]", *d); }
void PRINTARG(const size_t *li) { printf("*[%li]", *li); }
void PRINTARG(const long *li) { printf("*[%li]", *li); }
        #endif
void PRINTARG(const void *x) { printf("%s", x ? "<ptr>" : "NULL"); }
// void PRINTARG(const VAPoR::VDC::AccessMode x) { printf("VDC::AccessMode::%s", x==VAPoR::VDC::AccessMode::R?"R":x==VAPoR::VDC::AccessMode::W?"W":"A"); }
// void PRINTARG(const VAPoR::DC::XType x)
// {
// 	switch (x) {
// 		case VAPoR::DC::XType::INVALID: printf("DC::INVALID"); break;
// 		case VAPoR::DC::XType::FLOAT:   printf("DC::FLOAT"); break;
// 		case VAPoR::DC::XType::DOUBLE:  printf("DC::DOUBLE"); break;
// 		case VAPoR::DC::XType::UINT8:   printf("DC::UINT8"); break;
// 		case VAPoR::DC::XType::INT8:    printf("DC::INT8"); break;
// 		case VAPoR::DC::XType::INT32:   printf("DC::INT32"); break;
// 		case VAPoR::DC::XType::INT64:   printf("DC::INT64"); break;
// 		case VAPoR::DC::XType::TEXT:    printf("DC::TEXT"); break;
// 	}
// }
template<class T> void PRINTARG(const std::vector<T> v)
{
        #ifdef VDC_LIBTRACE_RUNNABLE
    if (v.size() == 1) {
        string t(__PRETTY_FUNCTION__);
        int    s = t.find("[T = ") + strlen("[T = ");
        int    f = t.find("]", s);
        t = t.substr(s, f - s);
        printf("vector<%s>(1, ", t.c_str());
    } else
        #endif
        printf("{");
    for (int i = 0; i < v.size(); i++) {
        PRINTARG(v[i]);
        if (i != v.size() - 1) printf(", ");
    }
        #ifdef VDC_LIBTRACE_RUNNABLE
    if (v.size() == 1)
        printf(")");
    else
        #endif
        printf("}");
}

inline std::string classScopeFromPrettyFunc(const std::string &pretty)
{
    size_t args = pretty.find("(");
    size_t colons = pretty.substr(0, args).rfind("::");
    if (colons == std::string::npos) return "";
    size_t begin = pretty.substr(0, colons).rfind(" ") + 1;
    size_t end = colons - begin;

    return pretty.substr(begin, end) + "::";
}

        #define _CONCAT(a, b) a##b
        #define PRINTARGS_1(x, ...) \
            {                       \
                PRINTARG(x);        \
            }
        #define PRINTARGS_2(x, ...)       \
            {                             \
                PRINTARG(x);              \
                printf(", ");             \
                PRINTARGS_1(__VA_ARGS__); \
            }
        #define PRINTARGS_3(x, ...)       \
            {                             \
                PRINTARG(x);              \
                printf(", ");             \
                PRINTARGS_2(__VA_ARGS__); \
            }
        #define PRINTARGS_4(x, ...)       \
            {                             \
                PRINTARG(x);              \
                printf(", ");             \
                PRINTARGS_3(__VA_ARGS__); \
            }
        #define PRINTARGS_5(x, ...)       \
            {                             \
                PRINTARG(x);              \
                printf(", ");             \
                PRINTARGS_4(__VA_ARGS__); \
            }
        #define PRINTARGS_6(x, ...)       \
            {                             \
                PRINTARG(x);              \
                printf(", ");             \
                PRINTARGS_5(__VA_ARGS__); \
            }
        #define PRINTARGS_7(x, ...)       \
            {                             \
                PRINTARG(x);              \
                printf(", ");             \
                PRINTARGS_6(__VA_ARGS__); \
            }
        #define PRINTARGS_8(x, ...)       \
            {                             \
                PRINTARG(x);              \
                printf(", ");             \
                PRINTARGS_7(__VA_ARGS__); \
            }
        #define PRINTARGS_9(x, ...)       \
            {                             \
                PRINTARG(x);              \
                printf(", ");             \
                PRINTARGS_8(__VA_ARGS__); \
            }
        #define _PRINTARGS(n, ...) \
            _CONCAT(PRINTARGS_, n) \
            (__VA_ARGS__)
        #define PRINTARGS(...) _PRINTARGS(NARG(__VA_ARGS__), __VA_ARGS__)
        #define FTRACE_PRE()   printf("%s%s(", classScopeFromPrettyFunc(__PRETTY_FUNCTION__).c_str(), __func__)
        #define FTRACE(...)             \
            do {                        \
                FTRACE_PRE();           \
                PRINTARGS(__VA_ARGS__); \
                printf(");\n");         \
            } while (0)

    #else
        #define FTRACE(...)
    #endif

#endif

#endif
