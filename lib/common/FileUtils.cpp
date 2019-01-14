#include "vapor/FileUtils.h"
#include <string.h>
#include <algorithm>
#include <sys/stat.h>
#include <vapor/MyBase.h>

#ifdef WIN32
#include <Windows.h>
#else
#include <libgen.h>
#endif

using namespace Wasp;
using FileUtils::FileType;
using std::string;

#ifdef WIN32
const string FileUtils::Separator = "\\";
#else
const string FileUtils::Separator = "/";
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
        delete [] buf;
        return ret;
    } else {
        return "";
    }
}

std::string FileUtils::Basename(const std::string &path)
{
#ifdef WIN32
	char fileName[_MAX_FNAME];
	char extension[_MAX_EXT];
	_splitpath_s(path.c_str(),
		NULL, 0,
		NULL, 0,
		fileName, _MAX_FNAME,
		extension, _MAX_EXT);
	return string(fileName) + string(extension);
#else
	char *copy = strdup(path.c_str());
	string ret(basename(copy));
	free(copy);
	return ret;
#endif
}

std::string FileUtils::Dirname(const std::string &path)
{
#ifdef WIN32
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	_splitpath_s(path.c_str(),
		drive, _MAX_DRIVE,
		dir, _MAX_DIR,
		NULL, 0,
		NULL, 0);
	return string(drive) + string(dir);
#else
	char *copy = strdup(path.c_str());
	string ret(dirname(copy));
	free(copy);
	return ret;
#endif
}

std::string FileUtils::Extension(const std::string &path)
{
    string basename = Basename(path);
    size_t index = basename.rfind(".");
    if (index == string::npos
        || index == 0) // dotfile
        return "";
    return basename.substr(index + 1);
}

std::string FileUtils::POSIXPathToWindows(std::string path)
{
    std::replace(path.begin(), path.end(), '/', '\\');
    return path;
}

std::string FileUtils::POSIXPathToCurrentOS(const std::string &path)
{
#ifdef WIN32
    return POSIXPathToWindows(path);
#else
    return path;
#endif
}

long FileUtils::GetFileModifiedTime(const string &path)
{
	struct STAT64 attrib;
    STAT64(path.c_str(), &attrib);
    return attrib.st_mtime;
}

bool FileUtils::IsPathAbsolute(const std::string &path)
{
#ifdef WIN32
	char dir[_MAX_DIR];
	_splitpath_s(path.c_str(),
		NULL, 0,
		dir, _MAX_DIR,
		NULL, 0,
		NULL, 0);
	return dir[0] == '/' || dir[0] == '\\';
#else
    return path[0] == '/';
#endif
}

bool FileUtils::Exists(const std::string &path)
{
    return FileUtils::GetFileType(path) != FileType::Does_Not_Exist;
}

bool FileUtils::IsRegularFile(const std::string &path)
{
    return FileUtils::GetFileType(path) == FileType::File;
}

bool FileUtils::IsDirectory(const std::string &path)
{
    return FileUtils::GetFileType(path) == FileType::Directory;
}

FileType FileUtils::GetFileType(const std::string &path)
{
    struct STAT64 s;
    if (STAT64(path.c_str(), &s) == 0) {
        if (s.st_mode & S_IFDIR)
            return FileType::Directory;
        else if (s.st_mode & S_IFREG)
            return FileType::File;
        else
            return FileType::Other;
    }
    else
        return FileType::Does_Not_Exist;
}

std::string FileUtils::JoinPaths(std::initializer_list<std::string> paths)
{
    string path;
    for (auto it = paths.begin(); it != paths.end(); ++it) {
        if (!it->empty()) {
            if (!path.empty())
                path += Separator;
            path += *it;
        }
    }
    return path;
}

const char *FileUtils::LegacyBasename(const char *path)
{
	const char *last;
	last = strrchr(path, Separator[0]);
	if ( !last ) return path;
	else return last + 1;
}
