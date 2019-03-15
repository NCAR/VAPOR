#pragma once

#include <string>
#include <vector>

namespace STLUtils {

template<typename T> bool Contains(const std::vector<T> &toSearch, const T &object) { return std::find(toSearch.cbegin(), toSearch.cend(), object) != toSearch.cend(); }

bool                     BeginsWith(const std::string &str, const std::string &match);
std::vector<std::string> Split(std::string str, const std::string &delimeter);
std::string              Join(const std::vector<std::string> &parts, const std::string &delimeter);

}    // namespace STLUtils
