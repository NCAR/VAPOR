#pragma once

#include <vector>
#include "vapor/ShaderProgram.h"

/*
#ifdef GL_CURRENT_BIT
#define LGL_CURRENT_BIT GL_CURRENT_BIT
#else
#define LGL_CURRENT_BIT 1
#endif
 */

#ifdef GL_QUADS
    #define LGL_QUADS GL_QUADS
#else
    #define LGL_QUADS 105999
#endif

#define LEGACY_TODO(x) printf("TODO %s\n", #x)

namespace VAPoR {

struct GLManager;

//! \class LegacyGL
//! \ingroup Public_Render
//!
//! \brief Replements Legacy OpenGL API using OpenGL Core
//!
//! This class should not be used for any intensive rendering
//!
//! \author Stanislaw Jaroszynski
//! \date   August, 2018

class RENDER_API LegacyGL {
#pragma pack(push, 4)
    struct VertexData {
        float x, y, z;
        float nx, ny, nz;
        float r, g, b, a;
        float s, t;
    };
#pragma pack(pop)

    GLManager *             _glManager;
    std::vector<VertexData> _vertices;
    unsigned int            _mode;
    bool                    _emulateQuads;
    bool                    _firstQuadTriangle;
    unsigned int            _VAO, _VBO;
    float                   _nx, _ny, _nz;
    float                   _r, _g, _b, _a;
    float                   _s, _t;
    bool                    _initialized, _insideBeginEndBlock;
    bool                    _lightingEnabled, _textureEnabled;
    float                   _lightDir[3];

public:
    LegacyGL(GLManager *glManager);
    ~LegacyGL();
    void Initialize();
    void Begin(unsigned int mode);
    void End();
    void Vertex(glm::vec2);
    void Vertex(glm::vec3);
    void Vertex2f(float x, float y);
    void Vertex3f(float x, float y, float z);
    void Vertex3fv(const float *v);
    void Vertex3dv(const double *v);
    void Normal3f(float x, float y, float z);
    void Normal3fv(const float *n);
    void Color(glm::vec3);
    void Color(glm::vec4);
    void Color3f(float r, float g, float b);
    void Color3fv(const float *f);
    void Color4f(float r, float g, float b, float a);
    void Color4fv(const float *f);
    void TexCoord(glm::vec2);
    void TexCoord2f(float s, float t);

    void EnableLighting();
    void DisableLighting();
    void LightDirectionfv(const float *f);
    void EnableTexture();
    void DisableTexture();

#ifndef NDEBUG
    void TestSquare();
#endif

    // void PushAttrib(int flag);
};

}    // namespace VAPoR
