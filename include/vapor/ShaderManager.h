#pragma once

#include "vapor/IResourceManager.h"
#include "vapor/ShaderProgram.h"
#include <string>

namespace VAPoR {

class ShaderManager : public IResourceManager<std::string, ShaderProgram> {
    std::map<std::string, long> _modifiedTimes;

    std::vector<std::string> _getSourceFilePaths(const std::string &name) const;
    unsigned int             _getShaderTypeFromPath(const std::string &path) const;
    static long              _getFileModifiedTime(const std::string &path);
    bool                     _wasFileModified(const std::string &path) const;

public:
    ShaderProgram *    GetShader(const std::string &name);
    SmartShaderProgram GetSmartShader(const std::string &name);
    bool               LoadResourceByKey(const std::string &name);
};

}    // namespace VAPoR
