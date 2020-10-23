#pragma once

#include <vapor/common.h>
#include <string>
#include <vector>

namespace Wasp {
namespace TMSUtils {

COMMON_API bool IsTMSFile(std::string path);
COMMON_API std::string TilePath(std::string file, size_t tileX, size_t tileY, int lod);
COMMON_API int         GetNumTMSLODs(std::string file);

}    // namespace TMSUtils
}    // namespace Wasp
