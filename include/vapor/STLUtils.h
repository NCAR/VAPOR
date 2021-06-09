#pragma once

#include <vapor/MyBase.h>
#include <string>
#include <vector>
#include <algorithm>

namespace STLUtils {

template<typename T> bool Contains(const std::vector<T> &toSearch, const T &object) { return std::find(toSearch.cbegin(), toSearch.cend(), object) != toSearch.cend(); }

template<typename T> void AppendTo(std::vector<T> &a, const std::vector<T> &b) { a.insert(a.end(), b.begin(), b.end()); }

COMMON_API bool Contains(const std::string &toSearch, const std::string &query);
COMMON_API bool ContainsIgnoreCase(const std::string &toSearch, const std::string &query);
COMMON_API bool BeginsWith(const std::string &str, const std::string &match);
COMMON_API bool EndsWith(const std::string &str, const std::string &match);
COMMON_API std::string ToLower(std::string str);
COMMON_API std::vector<std::string> Split(std::string str, const std::string &delimeter);
COMMON_API std::string Join(const std::vector<std::string> &parts, const std::string &delimeter);
COMMON_API std::string ReplaceAll(std::string source, const std::string &oldSegment, const std::string &newSegment);

}    // namespace STLUtils
