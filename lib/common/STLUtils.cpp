#include <vapor/STLUtils.h>

using std::string;
using std::vector;

bool STLUtils::Contains(const std::string &toSearch, const std::string &query) {
    return toSearch.find(query) != string::npos;
}

bool STLUtils::BeginsWith(const std::string &str, const std::string &match) {
    return str.size() >= match.size() && equal(match.begin(), match.end(), str.begin());
}

std::string STLUtils::ToLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

std::vector<std::string> STLUtils::Split(std::string str, const std::string &delimeter) {
    size_t index;
    vector<string> parts;
    while ((index = str.find(delimeter)) != string::npos) {
        parts.push_back(str.substr(0, index));
        str.erase(0, index + delimeter.length());
    }
    parts.push_back(str);
    return parts;
}

std::string STLUtils::Join(const std::vector<std::string> &parts, const std::string &delimeter) {
    string whole;
    auto itr = parts.begin();
    whole = *itr++;
    for (; itr != parts.end(); ++itr)
        whole += delimeter + *itr;
    return whole;
}
