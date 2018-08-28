#include "vapor/glutil.h"
#include "vapor/LegacyGL.h"
#include <cassert>
#include <glm/glm.hpp>
#include "vapor/GLManager.h"
// #include <glm/gtc/matrix_transform.hpp>
// #include <glm/gtc/type_ptr.hpp>

using namespace VAPoR;
using std::vector;

extern const char *SHADER_SOURCE_VERTEX;
extern const char *SHADER_SOURCE_FRAGMENT;

LegacyGL::LegacyGL(GLManager *glManager) : _glManager(glManager), _mode(0), _VAO(0), _VBO(0), _r(1), _g(1), _b(1), _a(1), _initialized(false), _insideBeginEndBlock(false) { Initialize(); }

void LegacyGL::Initialize()
{
    assert(!_initialized);
    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);
    assert(_VAO);
    assert(_VBO);
    assert(_shader.AddShaderFromSource(GL_VERTEX_SHADER, SHADER_SOURCE_VERTEX));
    assert(_shader.AddShaderFromSource(GL_FRAGMENT_SHADER, SHADER_SOURCE_FRAGMENT));
    assert(_shader.Link());

    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), NULL);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(struct VertexData, r));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    _initialized = true;
}

void LegacyGL::Begin(unsigned int mode)
{
    _mode = mode;
    _insideBeginEndBlock = true;
}

void LegacyGL::End()
{
    if (!_initialized) Initialize();
    assert(_insideBeginEndBlock);

    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * _vertices.size(), _vertices.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    _shader.Bind();
    _shader.SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());
    glDrawArrays(_mode, 0, _vertices.size());

    glBindVertexArray(0);
    _shader.UnBind();

    _insideBeginEndBlock = false;
    _vertices.clear();
}

void LegacyGL::Vertex3f(float x, float y, float z)
{
    assert(_insideBeginEndBlock);
    _vertices.push_back({x, y, z, _r, _g, _b, _a});
}

void LegacyGL::Color3f(float r, float g, float b)
{
    _r = r;
    _g = g;
    _b = b;
    _a = 1.0f;
}

void LegacyGL::Color4f(float r, float g, float b, float a)
{
    _r = r;
    _g = g;
    _b = b;
    _a = a;
}

const char *SHADER_SOURCE_VERTEX = R"(
#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec4 vColor;

out vec4 fColor;

uniform mat4 MVP;

void main() {
    gl_Position = MVP * vec4(vPos, 1.0f);
    fColor = vColor;
}
)";

const char *SHADER_SOURCE_FRAGMENT = R"(
#version 330 core

in  vec4 fColor;
out vec4 fragment;

void main() {
    fragment = fColor;
}
)";
