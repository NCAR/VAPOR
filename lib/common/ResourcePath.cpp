#include <climits>
#include <vapor/ResourcePath.h>
#include <vapor/CMakeConfig.h>
#include <vapor/FileUtils.h>
#include <vapor/CFuncs.h>
#include <cassert>

#define INCLUDE_DEPRECATED_GET_APP_PATH
#include "vapor/GetAppPath.h"

#define FORCE_USE_DEV_LIBS 0
#if FORCE_USE_DEV_LIBS
    #ifndef NDEBUG
        #error Forced use of development libraries enabled for release build
    #endif
#endif

#ifdef Darwin
    #include <CoreFoundation/CFBundle.h>

string GetMacBundlePath()
{
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef    resourcesURL = CFBundleCopyBundleURL(mainBundle);
    CFStringRef str = CFURLCopyFileSystemPath(resourcesURL, kCFURLPOSIXPathStyle);
    CFRelease(resourcesURL);
    char path[PATH_MAX];

    CFStringGetCString(str, path, PATH_MAX, kCFStringEncodingASCII);
    CFRelease(str);

    return string(path);
}
#endif

string CallGetAppPathForResourceName(const string &name)
{
    vector<string> dirs;
    string         getAppPathName;

    Wasp::SplitString(name, '/', dirs);

    if (dirs.empty()) {
        getAppPathName = "home";
    } else {
        getAppPathName = dirs[0];
        dirs.erase(dirs.begin());
    }

    return Wasp::GetAppPath("VAPOR", getAppPathName, dirs);
}

string GetInstalledResourceRoot()
{
    string path;

#if defined Darwin
    path = GetMacBundlePath();
    path = Wasp::FileUtils::JoinPaths({path, "Contents"});
#elif defined WIN32
    path = string(Wasp::GetEnvironmentalVariable("VAPOR3_HOME"));
#else
    path = string(Wasp::GetEnvironmentalVariable("VAPOR_HOME"));
#endif

    return path;
}

#define TRY_PATH(p) {                                 \
    string path = Wasp::FileUtils::POSIXPathToCurrentOS(p); \
    if (Wasp::FileUtils::Exists(path)) return path;         \
}

std::string (*resourceFinderCB)(const std::string &) = nullptr;

string GetResourcePathFromCallback(const std::string &name)
{
    if (resourceFinderCB)
        return resourceFinderCB(name);
    return "";
}

std::string Wasp::GetResourcePath(const std::string &name)
{
#if FORCE_USE_DEV_LIBS
    TRY_PATH(FileUtils::JoinPaths({SOURCE_DIR, name}));
#else
    TRY_PATH(GetResourcePathFromCallback(name));
    TRY_PATH(FileUtils::JoinPaths({GetInstalledResourceRoot(), name}));
    TRY_PATH(FileUtils::JoinPaths({SOURCE_DIR, name}));
    TRY_PATH(FileUtils::JoinPaths({THIRD_PARTY_DIR, name}));
    TRY_PATH(CallGetAppPathForResourceName(name));
#endif

    return "";
}

std::string Wasp::GetSharePath(const std::string &name) { return GetResourcePath("share/" + name); }

#if defined(WIN32)
    #define PYTHON_INSTALLED_PATH ("python" + string(PYTHON_VERSION))
#elif defined(__aarch64__)
    #define PYTHON_INSTALLED_PATH ("Resources/python")
#else
    #define PYTHON_INSTALLED_PATH ("lib/python" + string(PYTHON_VERSION))
#endif

std::string Wasp::GetPythonVersion() { return std::string(PYTHON_VERSION); }

std::string Wasp::GetPythonPath()
{
    string path = GetResourcePath(PYTHON_INSTALLED_PATH);

    if (!FileUtils::Exists(path)) path = string(PYTHON_PATH);

    return path;
}

std::string Wasp::GetPythonDir()
{
#ifdef WIN32
    return GetPythonPath();
#endif
    string path = string(PYTHON_DIR);  // Try our third-party-library directory first
    if (!FileUtils::Exists(path)) {
        path = GetResourcePath("");
        string exists = FileUtils::JoinPaths({path, PYTHON_INSTALLED_PATH});
        if (!exists.empty()) path = exists; // If the third-party-library directory doesn't exist, use the python installed path.  Otherwise, use the root.
    }
    return path;
}

void Wasp::RegisterResourceFinder(std::string (*cb)(const std::string &))
{
    assert(resourceFinderCB == nullptr);
    resourceFinderCB = cb;
}
