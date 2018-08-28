#pragma once

#include "vapor/ShaderManager.h"
#include "vapor/MatrixManager.h"
#include "vapor/LegacyGL.h"
#include "vapor/FontManager.h"

namespace VAPoR {

struct GLManager {
    ShaderManager *shaderManager;
    FontManager *  fontManager;
    MatrixManager *matrixManager;
    LegacyGL *     legacy;

    GLManager();
    ~GLManager();
};

}    // namespace VAPoR
