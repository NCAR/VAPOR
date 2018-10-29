#include "vapor/FileUtils.h"
#include <string.h>

#ifdef WIN32
#else
    #include <sys/stat.h>
    #include <libgen.h>
#endif

using namespace VAPoR;
using FileUtils::FileType;
using std::string;

#ifdef WIN32
const char FileUtils::Separator = '\\';
#else
const char FileUtils::Separator = '/';
#endif

string FileUtils::ReadFileToString(const string &path)
{
    FILE *f = fopen(path.c_str(), "r");
    if (f) {
        fseek(f, 0, SEEK_END);
        long length = ftell(f);
        rewind(f);
        char *buf = new char[length + 1];
        fread(buf, length, 1, f);
        buf[length] = 0;
        string ret(buf);
        delete[] buf;
        return ret;
    } else {
        return "";
    }
}

std::string FileUtils::Basename(const std::string &path)
{
#ifdef WIN32
    #error TODO
    return 0;
#else
    char * copy = strdup(path.c_str());
    string ret(basename(copy));
    free(copy);
    return ret;
#endif
}

std::string FileUtils::Dirname(const std::string &path)
{
#ifdef WIN32
    #error TODO
    return path;
#else
    char * copy = strdup(path.c_str());
    string ret(dirname(copy));
    free(copy);
    return ret;
#endif
}

std::string FileUtils::Extension(const std::string &path) { return path.substr(path.rfind(".") + 1); }

long FileUtils::GetFileModifiedTime(const string &path)
{
    struct stat attrib;
    stat(path.c_str(), &attrib);
    return attrib.st_mtime;
}

bool FileUtils::IsPathAbsolute(const std::string &path)
{
#ifdef WIN32
    return !PathIsRelative((LPCTSTR)path.c_str());
#else
    return path[0] == '/';
#endif
}

bool FileUtils::FileExists(const std::string &path) { return FileUtils::GetFileType(path) != FileType::Does_Not_Exist; }

bool FileUtils::IsRegularFile(const std::string &path) { return FileUtils::GetFileType(path) == FileType::File; }

bool FileUtils::IsDirectory(const std::string &path) { return FileUtils::GetFileType(path) == FileType::Directory; }

FileType FileUtils::GetFileType(const std::string &path)
{
    struct stat s;
    if (stat(path.c_str(), &s) == 0) {
        if (s.st_mode & S_IFDIR)
            return FileType::Directory;
        else if (s.st_mode & S_IFREG)
            return FileType::File;
        else
            return FileType::Other;
    } else
        return FileType::Does_Not_Exist;
}
