#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cerrno>
#include <stack>
#include <sys/types.h>
#include <sys/stat.h>
#include <vapor/MyBase.h>
#include <vapor/CFuncs.h>

#ifdef Darwin
    #include <mach/mach_time.h>
#endif

#include <iostream>
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

string clean_separators(string path)
{
#ifndef WIN32
    return (path);
#else
    string::size_type pos;
    string            unixsep = "/";
    string            winsep = "\\";
    while ((pos = path.find(unixsep)) != string::npos) { path.replace(pos, 1, winsep); }
    return (path);
#endif
}

};    // namespace

using namespace Wasp;

// Ugh. This should be deprecated
//
const char *Wasp::Basename(const char *path)
{
    const char *last;
    last = strrchr(path, Separator[0]);
    if (!last)
        return path;
    else
        return last + 1;
}

string Wasp::Basename(const string &path)
{
    string mypath = clean_separators(path);

    string::size_type pos = mypath.rfind(Separator);
    if (pos == string::npos) return (mypath);

    return (mypath.substr(pos + 1));
}

string Wasp::Dirname(const string &path)
{
    string mypath = clean_separators(path);

    string::size_type pos = mypath.rfind(Separator);
    if (pos == string::npos) return (".");

    return (mypath.substr(0, pos));
}

string Wasp::Catpath(string volume, string dir, string file)
{
    string path;

#ifdef WIN32
    path = volume;
#endif
    path += dir;
    path += Separator;
    path += file;

    return (path);
}

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
        dir = Dirname(path);
        file = Basename(path);
    }
#endif
}

bool Wasp::IsAbsPath(string path)
{
    string vol, dir, fname;
    Splitpath(path, vol, dir, fname, true);

    return (dir.substr(0, 1) == Separator);
}

double Wasp::GetTime()
{
    double t;
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

#if defined(Linux) || defined(AIX)
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

int Wasp::MkDirHier(const string &dir)
{
    stack<string> dirs;

    string::size_type idx;
    string            s = dir;

    dirs.push(s);
    while ((idx = s.find_last_of(Separator)) != string::npos) {
        s = s.substr(0, idx);
        if (!s.empty()) dirs.push(s);
    }

    while (!dirs.empty()) {
        s = dirs.top();
        dirs.pop();
#ifndef WIN32
        if ((mkdir(s.c_str(), 0777) < 0) && dirs.empty() && errno != EEXIST) {
            MyBase::SetErrMsg("mkdir(%s) : %M", s.c_str());
            return (-1);
        }
#else
        // Windows version of mkdir:
        // If it succeeds, return value is nonzero
        if (!CreateDirectory((LPCSTR)s.c_str(), 0)) {
            DWORD dw = GetLastError();
            if (dw != 183) {    // 183 means file already exists
                MyBase::SetErrMsg("mkdir(%s) : %M", s.c_str());
                return (-1);
            }
        }
#endif
    }
    return (0);
}
