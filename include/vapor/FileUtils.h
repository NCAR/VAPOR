#pragma once

#include <vapor/common.h>
#include <string>
#include <vector>
#include <initializer_list>

namespace Wasp {
namespace FileUtils {

enum class FileType { File, Directory, Other, Does_Not_Exist };

extern COMMON_API const std::string Separator;

COMMON_API std::string ReadFileToString(const std::string &path);
COMMON_API std::string HomeDir();
COMMON_API std::string Basename(const std::string &path);
COMMON_API std::string Dirname(const std::string &path);
COMMON_API std::string Realpath(const std::string &path);
COMMON_API std::string Relpath(std::string path, std::string to);
COMMON_API std::string CommonAncestor(const std::vector<std::string> &paths);
COMMON_API std::string Extension(const std::string &path);
COMMON_API std::string RemoveExtension(const std::string &path);
COMMON_API std::string POSIXPathToWindows(std::string path);
COMMON_API std::string POSIXPathToCurrentOS(const std::string &path);
COMMON_API std::string CleanupPath(std::string path);
COMMON_API long        GetFileModifiedTime(const std::string &path);
COMMON_API bool        IsPathAbsolute(const std::string &path);
COMMON_API bool        Exists(const std::string &path);
COMMON_API bool        IsRegularFile(const std::string &path);
COMMON_API bool        IsDirectory(const std::string &path);
COMMON_API bool        IsSubpath(const std::string &dir, const std::string &path);
COMMON_API bool        AreSameFile(const std::string &pathA, const std::string &pathB);
COMMON_API FileType    GetFileType(const std::string &path);
COMMON_API long long   GetFileSize(const std::string &path);
COMMON_API std::vector<std::string> ListFiles(const std::string &path);

//! @code JoinPaths({"home", "a/b"}); @endcode
COMMON_API std::string JoinPaths(const std::vector<std::string> &paths);
COMMON_API std::vector<std::string> SplitPath(std::string path);

COMMON_API int MakeDir(const std::string &path);

COMMON_API const char *LegacyBasename(const char *path);

}    // namespace FileUtils
}    // namespace Wasp
