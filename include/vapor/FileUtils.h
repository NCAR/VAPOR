#pragma once

#include <string>

namespace VAPoR {
namespace FileUtils {

enum class FileType { File, Directory, Other, Does_Not_Exist };

extern const char Separator;

std::string ReadFileToString(const std::string &path);
std::string Basename(const std::string &path);
std::string Extension(const std::string &path);
long        GetFileModifiedTime(const std::string &path);
bool        FileExists(const std::string &path);
bool        IsRegularFile(const std::string &path);
bool        IsDirectory(const std::string &path);
FileType    GetFileType(const std::string &path);

}    // namespace FileUtils
}    // namespace VAPoR
