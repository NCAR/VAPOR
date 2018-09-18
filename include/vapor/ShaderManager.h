#pragma once

#include "vapor/IResourceManager.h"
#include "vapor/ShaderProgram2.h"
#include <string>

namespace VAPoR {

class ShaderManager : public IResourceManager<std::string, ShaderProgram2> {
    std::map<std::string, long> _modifiedTimes;

    std::vector<std::string> _getSourceFilePaths(const std::string &name) const;
    unsigned int             _getShaderTypeFromPath(const std::string &path) const;
    static long              _getFileModifiedTime(const std::string &path);
    bool                     _wasFileModified(const std::string &path) const;

public:
    ShaderProgram2 *   GetShader(const std::string &name);
    SmartShaderProgram GetSmartShader(const std::string &name);
    bool               LoadResourceByKey(const std::string &name);
};

}    // namespace VAPoR
