#include "vapor/ResourcePath.h"
#include "vapor/CMakeConfig.h"
#include "vapor/FileUtils.h"

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
    path = string(getenv("VAPOR3_HOME"));
#else
    path = string(getenv("VAPOR_HOME"));
#endif

    return path;
}

std::string Wasp::GetResourcePath(const std::string &name)
{
    string posixPath;

#if FORCE_USE_DEV_LIBS
    posixPath = FileUtils::JoinPaths({SOURCE_DIR, name});
#else
    posixPath = FileUtils::JoinPaths({GetInstalledResourceRoot(), name});

    if (!FileUtils::Exists(FileUtils::POSIXPathToCurrentOS(posixPath))) posixPath = FileUtils::JoinPaths({SOURCE_DIR, name});

    if (!FileUtils::Exists(FileUtils::POSIXPathToCurrentOS(posixPath))) posixPath = CallGetAppPathForResourceName(name);
#endif

    string path = FileUtils::POSIXPathToCurrentOS(posixPath);
    if (!FileUtils::Exists(path)) path = "";

    return path;
}

std::string Wasp::GetSharePath(const std::string &name) { return GetResourcePath("share/" + name); }

std::string Wasp::GetPythonPath()
{
    string path = GetResourcePath("lib/python" + string(PYTHON_VERSION));

    if (!FileUtils::Exists(path)) path = string(PYTHON_PATH);

    return path;
}

std::string Wasp::GetPythonDir()
{
    string path = GetResourcePath("");

    if (!FileUtils::Exists(path)) path = string(PYTHON_DIR);

    return path;
}
