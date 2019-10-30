#include "vapor/glutil.h"
#include "vapor/LegacyGL.h"
#include "vapor/VAssert.h"
#include <glm/glm.hpp>
#include "vapor/GLManager.h"
#include "vapor/ShaderProgram.h"
#include <glm/gtc/type_ptr.hpp>

using namespace VAPoR;
using std::vector;

LegacyGL::LegacyGL(GLManager *glManager)
: _glManager(glManager), _mode(0), _emulateQuads(false), _firstQuadTriangle(true), _VAO(0), _VBO(0), _nx(0), _ny(0), _nz(0), _r(1), _g(1), _b(1), _a(1), _s(0), _t(0), _initialized(false),
  _insideBeginEndBlock(false), _lightingEnabled(false), _textureEnabled(false), _lightDir{0}
{
}

LegacyGL::~LegacyGL()
{
    if (_VAO) glDeleteVertexArrays(1, &_VAO);
    if (_VBO) glDeleteBuffers(1, &_VBO);
}

void LegacyGL::Initialize()
{
    GL_ERR_BREAK();
    VAssert(!_initialized);

    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);

    VAssert(_VAO);
    VAssert(_VBO);

    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), NULL);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(struct VertexData, nx));
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(struct VertexData, r));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void *)offsetof(struct VertexData, s));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    _initialized = true;
    GL_ERR_BREAK();
}

void LegacyGL::Begin(unsigned int mode)
{
    VAssert(!_insideBeginEndBlock);

    if (mode == LGL_QUADS) {
        _emulateQuads = true;
        mode = GL_TRIANGLES;
    }

    _mode = mode;
    _insideBeginEndBlock = true;
}

void LegacyGL::End()
{
    if (!_initialized) Initialize();
    VAssert(_insideBeginEndBlock);

    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexData) * _vertices.size(), _vertices.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    ShaderProgram *_shader = _glManager->shaderManager->GetShader("Legacy");
    _shader->Bind();
    _shader->SetUniform("P", _glManager->matrixManager->GetProjectionMatrix());
    _shader->SetUniform("MV", _glManager->matrixManager->GetModelViewMatrix());
    _shader->SetUniform("lightingEnabled", _lightingEnabled);
    _shader->SetUniform("textureEnabled", _textureEnabled);
    _shader->SetUniform("lightDir", glm::make_vec3(_lightDir));

    VAssert(glIsVertexArray(_VAO) == GL_TRUE);
    VAssert(glIsBuffer(_VBO) == GL_TRUE);

    glDrawArrays(_mode, 0, _vertices.size());

    glBindVertexArray(0);
    _shader->UnBind();

    _emulateQuads = false;
    _insideBeginEndBlock = false;
    _vertices.clear();
}

void LegacyGL::Vertex(glm::vec2 v) { Vertex2f(v.x, v.y); }

void LegacyGL::Vertex(glm::vec3 v) { Vertex3f(v.x, v.y, v.z); }

void LegacyGL::Vertex2f(float x, float y) { Vertex3f(x, y, 0); }

extern bool PRINT_VERTS;

void LegacyGL::Vertex3f(float x, float y, float z)
{
    VAssert(_insideBeginEndBlock);
    _vertices.push_back({x, y, z, _nx, _ny, _nz, _r, _g, _b, _a, _s, _t});

    if (_emulateQuads && _vertices.size() % 3 == 0) {
        if (_firstQuadTriangle) {
            VertexData v1 = _vertices[_vertices.size() - 3];
            VertexData v3 = _vertices[_vertices.size() - 1];
            _vertices.push_back(v1);
            _vertices.push_back(v3);
            _firstQuadTriangle = false;
        } else {
            _firstQuadTriangle = true;
        }
    }
}

void LegacyGL::Vertex3fv(const float *v) { Vertex3f(v[0], v[1], v[2]); }

void LegacyGL::Vertex3dv(const double *v) { Vertex3f((float)v[0], (float)v[1], (float)v[2]); }

void LegacyGL::Normal3f(float x, float y, float z)
{
    _nx = x;
    _ny = y;
    _nz = z;
}

void LegacyGL::Normal3fv(const float *n) { Normal3f(n[0], n[1], n[2]); }

void LegacyGL::Color(glm::vec3 v) { Color3f(v.r, v.g, v.b); }

void LegacyGL::Color(glm::vec4 v) { Color4f(v.r, v.g, v.b, v.a); }

void LegacyGL::Color3f(float r, float g, float b)
{
    _r = r;
    _g = g;
    _b = b;
    _a = 1.0f;
}

void LegacyGL::Color3fv(const float *f) { Color3f(f[0], f[1], f[2]); }

void LegacyGL::Color4f(float r, float g, float b, float a)
{
    _r = r;
    _g = g;
    _b = b;
    _a = a;
}

void LegacyGL::Color4fv(const float *f) { Color4f(f[0], f[1], f[2], f[3]); }

void LegacyGL::TexCoord(glm::vec2 st) { TexCoord2f(st.s, st.t); }

void LegacyGL::TexCoord2f(float s, float t)
{
    _s = s;
    _t = t;
}

void LegacyGL::EnableLighting() { _lightingEnabled = true; }
void LegacyGL::DisableLighting() { _lightingEnabled = false; }

void LegacyGL::LightDirectionfv(const float *f)
{
    glm::vec3 dir = glm::make_vec3(f);
    // mimic legacy setlightdirectionfv
    //    dir = _glManager->matrixManager->GetModelViewMatrix() * glm::vec4(dir, 0.f);
    _lightDir[0] = dir.x;
    _lightDir[1] = dir.y;
    _lightDir[2] = dir.z;
}

void LegacyGL::EnableTexture() { _textureEnabled = true; }
void LegacyGL::DisableTexture() { _textureEnabled = false; }
