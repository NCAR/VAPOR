#pragma once

#include "vapor/ShaderManager.h"
#include "vapor/MatrixManager.h"

namespace VAPoR {

class LegacyGL;
class FontManager;

//! \class GLManager
//! \ingroup Public_Render
//!
//! \brief Contains references to context scope OpenGL data
//!
//! \author Stanislaw Jaroszynski

struct RENDER_API GLManager {
    ShaderManager *shaderManager;
    FontManager *  fontManager;
    MatrixManager *matrixManager;
    LegacyGL *     legacy;

    GLManager();
    ~GLManager();

    //! \retval vector<int>[4] from glGetIntegerv(GL_VIEWPORT)
    //!
    static std::vector<int> GetViewport();
    static glm::vec2        GetViewportSize();

    //! Utility function that pushes the current matrix state and
    //! sets up a pixel coorinate 2D orthographic projection
    //!
    void PixelCoordinateSystemPush();
    void PixelCoordinateSystemPop();

    enum class Vendor { Intel, Nvidia, AMD, Mesa, Other, Unknown };

    static Vendor GetVendor();
    static void   GetGLVersion(int *major, int *minor);
    static int    GetGLSLVersion();
    static bool   IsCurrentOpenGLVersionSupported();
    static bool   CheckError();

#ifndef NDEBUG
    //! Draws the depth buffer in the top right corner
    //!
    void ShowDepthBuffer();
#endif
    static void * BeginTimer();
    static double EndTimer(void *startTime);

private:
    static Vendor _cachedVendor;

    void _queryVendor();
};

}    // namespace VAPoR
