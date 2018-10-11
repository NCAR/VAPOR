#pragma once

#include "vapor/ShaderManager.h"
#include "vapor/MatrixManager.h"
#include "vapor/FontManager.h"

namespace VAPoR {

class LegacyGL;

//! \class GLManager
//! \ingroup Public_Render
//!
//! \brief Contains references to context scope OpenGL data
//!
//! \author Stanislaw Jaroszynski

struct GLManager {
    ShaderManager *shaderManager;
    FontManager *  fontManager;
    MatrixManager *matrixManager;
    LegacyGL *     legacy;

    GLManager();
    ~GLManager();

    //! \retval vector<int>[4] from glGetIntegerv(GL_VIEWPORT)
    //!
    static std::vector<int> GetViewport();

    //! Utility function that pushes the current matrix state and
    //! sets up a pixel coorinate 2D orthographic projection
    //!
    void PixelCoordinateSystemPush();
    void PixelCoordinateSystemPop();

    static bool CheckError();

#ifndef NDEBUG
    //! Draws the depth buffer in the top right corner
    //!
    void ShowDepthBuffer();
#endif
};

}    // namespace VAPoR
