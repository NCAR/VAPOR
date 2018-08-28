#pragma once

#include "vapor/IResourceManager.h"
#include "vapor/ShaderProgram2.h"
#include <string>

namespace VAPoR {

class ShaderManager : public IResourceManager<std::string, ShaderProgram2> {
public:
    ShaderProgram2 *   GetShader(const std::string name);
    SmartShaderProgram GetSmartShader(const std::string name);
    bool               LoadResourceByKey(const std::string name);
};

}    // namespace VAPoR
