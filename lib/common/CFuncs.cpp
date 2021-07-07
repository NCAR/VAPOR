#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <stack>
#include <sys/types.h>
#include <sys/stat.h>
#include <vapor/MyBase.h>
#include <vapor/CFuncs.h>
#include <vapor/FileUtils.h>

#ifdef Darwin
    #include <mach/mach_time.h>
#endif

#include <iostream>
#include <fstream>

#ifdef WIN32
    #include "windows.h"
    #include "Winbase.h"
    #pragma warning(disable : 4996)
#endif

namespace {

#ifdef WIN32
string Separator = "\\";
#else
string Separator = "/";
#endif

};    // namespace

using namespace Wasp;
using namespace std;

void Wasp::Splitpath(string path, string &volume, string &dir, string &file, bool nofile)
{
    volume.clear();
    dir.clear();
    file.clear();

#ifdef WIN32
    char drive_buf[_MAX_DRIVE];
    char dir_buf[_MAX_DIR];
    char fname_buf[_MAX_FNAME];
    char ext_buf[_MAX_EXT];
    _splitpath(path.c_str(), drive_buf, dir_buf, fname_buf, ext_buf);

    volume = drive_buf;
    dir = dir_buf;
    file = fname_buf;
    file += ext_buf;
#else

    if (nofile || (path.substr(path.size() - 1, 1) == "/") || (path.substr(path.size() - 2, 2) == "/.") || (path.substr(path.size() - 3, 3) == "/..")) {
        dir = path;
    } else {
        dir = Wasp::FileUtils::Dirname(path);
        file = Wasp::FileUtils::Basename(path);
    }
#endif
}

/*
bool Wasp::IsAbsPath(string path) {

    string vol, dir, fname;
    Splitpath(path, vol, dir, fname, true);

    return(dir.substr(0,1) == Separator);
}
 */

double Wasp::GetTime()
{
    double t = -1.0;
#ifdef WIN32    // Windows does not have a nanosecond time function...
    SYSTEMTIME sTime;
    FILETIME   fTime;
    GetSystemTime(&sTime);
    SystemTimeToFileTime(&sTime, &fTime);
    // Resulting system time is in 100ns increments
    __int64 longlongtime = fTime.dwHighDateTime;
    longlongtime <<= 32;
    longlongtime += fTime.dwLowDateTime;
    t = (double)longlongtime;
    t *= 1.e-7;

#endif
#ifndef WIN32
    struct timespec ts;
    ts.tv_sec = ts.tv_nsec = 0;
#endif

#if defined(__linux__) || defined(AIX)
    clock_gettime(CLOCK_REALTIME, &ts);
    t = (double)ts.tv_sec + (double)ts.tv_nsec * 1.0e-9;
#endif

#ifdef Darwin
    uint64_t                  tmac = mach_absolute_time();
    mach_timebase_info_data_t info = {0, 0};
    mach_timebase_info(&info);
    ts.tv_sec = tmac * 1e-9;
    ts.tv_nsec = tmac - (ts.tv_sec * 1e9);
    t = (double)ts.tv_sec + (double)ts.tv_nsec * 1.0e-9;
#endif

    return (t);
}

int Wasp::MkDirHier(const string &dir) { return FileUtils::MakeDir(dir); }

std::string Wasp::GetEnvironmentalVariable(const std::string &name)
{
    const char *env = getenv(name.c_str());
    if (env)
        return string(env);
    else
        return "";
}
