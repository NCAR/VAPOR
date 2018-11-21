#pragma once

#include <vapor/common.h>
#include <string>

namespace Wasp {
namespace FileUtils {

enum class FileType { File, Directory, Other, Does_Not_Exist };

extern const std::string Separator;

COMMON_API std::string ReadFileToString(const std::string &path);
COMMON_API std::string Basename(const std::string &path);
COMMON_API std::string Dirname(const std::string &path);
COMMON_API std::string Extension(const std::string &path);
COMMON_API long        GetFileModifiedTime(const std::string &path);
COMMON_API bool        IsPathAbsolute(const std::string &path);
COMMON_API bool        FileExists(const std::string &path);
COMMON_API bool        IsRegularFile(const std::string &path);
COMMON_API bool        IsDirectory(const std::string &path);
COMMON_API FileType    GetFileType(const std::string &path);

COMMON_API const char *LegacyBasename(const char *path);

}    // namespace FileUtils
}    // namespace Wasp
