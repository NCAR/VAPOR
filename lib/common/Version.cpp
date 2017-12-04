#include <cstdlib>
#include <vector>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vapor/MyBase.h>
#include <vapor/Version.h>

using namespace Wasp;
using namespace std;

//
// If these aren't defined here, the module won't link!
//
string Version::_formatString;
string Version::_dateString;

const int Version::_majorVersion = MAJOR;
const int Version::_minorVersion = MINOR;
const int Version::_minorMinorVersion = MICRO;

const string &Version::GetVersionString()
{
    ostringstream oss;
    oss << _majorVersion << "." << _minorVersion << "." << _minorMinorVersion;
    if (GetRC().length()) oss << "." << GetRC();
    _formatString = oss.str();
    StrRmWhiteSpace(_formatString);
    return (_formatString);
}

int Version::Compare(int major, int minor, int minorminor)
{
    if (major != _majorVersion) { return (major > _majorVersion ? -1 : 1); }
    if (minor != _minorVersion) { return (minor > _minorVersion ? -1 : 1); }
    if (minorminor != _minorMinorVersion) { return (minorminor > _minorMinorVersion ? -1 : 1); }

    return (0);
}

namespace {
vector<string> split(string s, string delim)
{
    size_t         pos = 0;
    vector<string> tokens;
    while ((pos = s.find(delim)) != std::string::npos) {
        tokens.push_back(s.substr(0, pos));
        s.erase(0, pos + delim.length());
    }
    if (!s.empty()) tokens.push_back(s);

    return (tokens);
}
};    // namespace

void Version::Parse(string s, int &major, int &minor, int &minorminor, string &rc)
{
    StrRmWhiteSpace(s);

    major = 0;
    minor = 0;
    minorminor = 0;
    rc = "";

    vector<string> tokens = split(s, ".");

    if (tokens.size() > 0) {
        istringstream ist(tokens[0]);
        if (!ist.eof()) ist >> major;
    }
    if (tokens.size() > 1) {
        istringstream ist(tokens[1]);
        if (!ist.eof()) ist >> minor;
    }
    if (tokens.size() > 2) {
        istringstream ist(tokens[2]);
        if (!ist.eof()) ist >> minorminor;
    }
    if (tokens.size() > 3) { rc = tokens[3]; }
}

int Version::Compare(string v1, string v2)
{
    StrRmWhiteSpace(v1);
    StrRmWhiteSpace(v2);

    int    major1, minor1, minorminor1;
    string rc1;

    int    major2, minor2, minorminor2;
    string rc2;

    Version::Parse(v1, major1, minor1, minorminor1, rc1);
    Version::Parse(v2, major2, minor2, minorminor2, rc2);

    if (major1 < major2)
        return (-1);
    else if (major1 > major2)
        return (1);

    if (minor1 < minor2)
        return (-1);
    else if (minor1 > minor2)
        return (1);

    if (minorminor1 < minorminor2)
        return (-1);
    else if (minorminor1 > minorminor2)
        return (1);

    //
    // comparison for rc token isn't lexicographical: version strings
    // without a rc token are greater than version strings with a rc token
    //
    if (rc1.length() && rc2.length()) {
        if (StrCmpNoCase(rc1, rc2) < 0)
            return (-1);
        else if (StrCmpNoCase(rc1, rc2) > 0)
            return (1);
    } else if (rc1.length() && !rc2.length()) {
        return (-1);
    } else if (!rc1.length() && rc2.length()) {
        return (1);
    }

    return (0);
}
