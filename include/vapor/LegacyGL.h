#pragma once

#include <vector>
#include "vapor/ShaderProgram2.h"

namespace VAPoR {

struct GLManager;

class LegacyGL {
#pragma pack(push, 4)
    struct VertexData {
        float x, y, z;
        float r, g, b, a;
    };
#pragma pack(pop)

    GLManager *             _glManager;
    std::vector<VertexData> _vertices;
    unsigned int            _mode;
    unsigned int            _VAO, _VBO;
    ShaderProgram2          _shader;
    float                   _r, _g, _b, _a;
    bool                    _initialized, _insideBeginEndBlock;

public:
    LegacyGL(GLManager *glManager);
    void Initialize();
    void Begin(unsigned int mode);
    void End();
    void Vertex3f(float x, float y, float z);
    void Color3f(float r, float g, float b);
    void Color4f(float r, float g, float b, float a);
};

}    // namespace VAPoR
