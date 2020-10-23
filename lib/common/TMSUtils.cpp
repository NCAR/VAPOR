#include <iostream>
#include <sstream>
#include <algorithm>
#include <sys/stat.h>
#include <vapor/MyBase.h>
#include <vapor/TMSUtils.h>

using namespace Wasp;

//! Returns true if \p path is a Tile Mapping Service master file
//
bool TMSUtils::IsTMSFile(std::string path)
{
    if (path.rfind(".tms", path.size() - 4) != string::npos) { return true; }
    return false;
}

//! Returns the path to the TMS file containing the specified tile
//
std::string TMSUtils::TilePath(string file, size_t tileX, size_t tileY, int lod)
{
    // If we're given a file instead of a directory, remove the .tms extension
    //
    if (file.rfind(".tms", file.size() - 4) != string::npos) { file.erase(file.length() - 4, 4); }

    size_t tmsTileY = tileY;

    ostringstream oss;
    oss << file;
    oss << "/";
    oss << lod;
    oss << "/";
    oss << tileX;
    oss << "/";
    oss << tmsTileY;

    string base = oss.str();

    string path = base + ".tif";

    struct stat statbuf;
    if (stat(path.c_str(), &statbuf) == 0) return (path);

    path = base + ".tiff";

    if (stat(path.c_str(), &statbuf) == 0) return (path);

    // Tile does not exist
    //
    return ("");
}

//! Returns the number of LODs in the TMS file referened by \p file
//
int TMSUtils::GetNumTMSLODs(std::string file)
{
    int lod = 0;
    while (TilePath(file, 0, 0, lod) != "") { lod++; }
    return lod;
}
