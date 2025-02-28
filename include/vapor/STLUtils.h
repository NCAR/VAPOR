#pragma once

#include <vapor/MyBase.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <functional>
#include <iterator>

namespace STLUtils {

template<typename T> bool Contains(const std::vector<T> &toSearch, const T &object) { return std::find(toSearch.cbegin(), toSearch.cend(), object) != toSearch.cend(); }

template<typename T> void AppendTo(std::vector<T> &a, const std::vector<T> &b) { a.insert(a.end(), b.begin(), b.end()); }

template<typename T> std::vector<T> Slice(const std::vector<T> &a, int from, int to = -1) { return std::vector<T>(a.begin() + from, to < 0 ? a.end() : a.begin() + to); }
template<typename T, typename K> const std::vector<T> MapKeys(const std::map<T, K> m) {
    // const auto keys = std::views::keys(m);
    // return vector<T>(keys.begin(), keys.end());
    vector<T> keys;
    keys.reserve(m.size());
    for (const auto &it : m)
        keys.push_back(it.first);
    return keys;
}

template<typename T> vector<T> Filter(const std::vector<T> &v, std::function<bool(const T&)> f) {
    vector<T> v2;
    std::copy_if(v.begin(), v.end(), std::back_inserter(v2), f);
    return v2;
}

template<typename T> bool BeginsWith(const T &str, const T &match) {
    return str.size() >= match.size() && equal(match.begin(), match.end(), str.begin());
}
COMMON_API bool BeginsWith(const std::string &str, const std::string &match);
COMMON_API bool Contains(const std::string &toSearch, const std::string &query);
COMMON_API bool ContainsIgnoreCase(const std::string &toSearch, const std::string &query);
COMMON_API bool EndsWith(const std::string &str, const std::string &match);
COMMON_API std::string ToLower(std::string str);
COMMON_API std::vector<std::string> Split(std::string str, const std::string &delimeter);
COMMON_API std::string Join(const std::vector<std::string> &parts, const std::string &delimeter);
COMMON_API std::string ReplaceAll(std::string source, const std::string &oldSegment, const std::string &newSegment);
COMMON_API std::string GetUserTime();

template<typename T>
vector<T> SyncToRemove(const vector<T> &truth, const vector<T> &cache)
{
    vector<T> toRemove;
    for (const auto &x : cache)
        if (!STLUtils::Contains(truth, x))
            toRemove.push_back(x);
    return toRemove;
}

template<typename T>
vector<T> SyncToAdd(const vector<T> &truth, const vector<T> &cache)
{
    vector<T> toAdd;
    for (const auto &x : truth)
        if (!STLUtils::Contains(cache, x))
            toAdd.push_back(x);
    return toAdd;
}

}    // namespace STLUtils
