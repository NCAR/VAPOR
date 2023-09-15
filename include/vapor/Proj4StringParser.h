#pragma once

#include <string>
#include <map>
#include <vapor/MyBase.h>

namespace VAPoR {
class RENDER_API Proj4StringParser : Wasp::MyBase {
    std::string                        _string;
    std::map<std::string, std::string> _tokens;

    static std::pair<std::string, std::string> Proj4ParameterToKeyValuePair(std::string proj);
    static std::map<std::string, std::string>  Proj4StringToParameterMap(std::string proj);

public:
    Proj4StringParser(const std::string &projString);

    bool        HasKey(const std::string &key) const;
    std::string GetString(const std::string &key, const std::string &defaultValue = "") const;
    double      GetDouble(const std::string &key, const double defaultValue = 0.0) const;

    static int Proj4EllipseStringToGeoTIFEnum(const std::string &proj);
};
}    // namespace VAPoR
