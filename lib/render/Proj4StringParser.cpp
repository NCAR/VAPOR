#include "vapor/Proj4StringParser.h"
#include <geotiffio.h>

using namespace VAPoR;
using std::map;
using std::pair;
using std::string;

Proj4StringParser::Proj4StringParser(const std::string &projString) : _string(projString), _tokens(Proj4StringToParameterMap(projString)) {}

pair<string, string> Proj4StringParser::Proj4ParameterToKeyValuePair(string proj)
{
    if (proj[0] != '+') { return pair<string, string>("NULL", "NULL"); }
    proj = proj.substr(1);
    string key = proj.substr(0, proj.find("="));
    string value = proj.substr(proj.find("=") + 1);
    return pair<string, string>(key, value);
}

map<string, string> Proj4StringParser::Proj4StringToParameterMap(string proj)
{
    const string        delimeter = " ";
    size_t              index;
    string              token;
    map<string, string> tokens;
    while ((index = proj.find(delimeter)) != string::npos) {
        token = proj.substr(0, index);
        proj.erase(0, index + delimeter.length());
        tokens.insert(Proj4ParameterToKeyValuePair(token));
    }
    tokens.insert(Proj4ParameterToKeyValuePair(proj));
    return tokens;
}

std::string Proj4StringParser::GetString(const std::string &key, const std::string &defaultValue) const
{
    auto it = _tokens.find(key);
    if (it == _tokens.end()) return defaultValue;
    return it->second;
}

double Proj4StringParser::GetDouble(const std::string &key, const double defaultValue) const
{
    auto it = _tokens.find(key);
    if (it == _tokens.end()) return defaultValue;
    return std::stod(it->second);
}

int Proj4StringParser::Proj4EllipseStringToGeoTIFEnum(const std::string &proj)
{
    if (0)
        ;
    else if (proj == "NWL9D")
        return Ellipse_NWL_9D;
    else if (proj == "WGS84")
        return Ellipse_WGS_84;
    else if (proj == "airy")
        return Ellipse_Airy_1830;
    else if (proj == "bess_nam")
        return Ellipse_Bessel_Namibia;
    else if (proj == "bessel")
        return Ellipse_Bessel_1841;
    else if (proj == "clrk66")
        return Ellipse_Clarke_1866;
    else if (proj == "clrk80")
        return Ellipse_Clarke_1880;
    else if (proj == "clrk80ign")
        return Ellipse_Clarke_1880_IGN;
    else if (proj == "helmert")
        return Ellipse_Helmert_1906;
    else if (proj == "krass")
        return Ellipse_Krassowsky_1940;
    else if (proj == "plessis")
        return Ellipse_Plessis_1817;
    else if (proj == "sphere")
        return Ellipse_Sphere;
    SetErrMsg("Invalid Proj4 ellipse parameter \"%s\"", proj.c_str());
    return -1;
}
