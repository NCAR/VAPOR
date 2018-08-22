#pragma once

#include "vapor/ShaderManager.h"
#include "vapor/MatrixManager.h"

namespace VAPoR {

class LegacyGL;

struct GLManager {
    ShaderManager *shaderManager;
    MatrixManager *matrixManager;
    LegacyGL *     legacy;

    GLManager();
    ~GLManager();
};

}    // namespace VAPoR
