#pragma once

#include "vapor/ShaderManager.h"
#include "vapor/MatrixManager.h"
#include "vapor/FontManager.h"

namespace VAPoR {

class LegacyGL;

struct GLManager {
    ShaderManager *shaderManager;
    FontManager *  fontManager;
    MatrixManager *matrixManager;
    LegacyGL *     legacy;

    GLManager();
    ~GLManager();

    static std::vector<int> GetViewport();
    void                    PixelCoordinateSystemPush();
    void                    PixelCoordinateSystemPop();

    static bool CheckError();

#ifndef NDEBUG
    void ShowDepthBuffer();
#endif
};

}    // namespace VAPoR
