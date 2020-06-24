#include <vapor/glutil.h>
#include "vapor/MatrixManager.h"
#include "vapor/VAssert.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace VAPoR;
using glm::mat4;
using glm::vec3;
using glm::vec4;
using std::pair;

MatrixManager::MatrixManager()
{
    _modelviewStack.push(glm::mat4(1.0));
    _projectionStack.push(glm::mat4(1.0));
    _currentStack = &_modelviewStack;
    _mode = Mode::ModelView;
}

glm::mat4 MatrixManager::GetCurrentMatrix() const { return top(); }

mat4 MatrixManager::GetProjectionMatrix() const { return _projectionStack.top(); }

mat4 MatrixManager::GetModelViewMatrix() const { return _modelviewStack.top(); }

mat4 MatrixManager::GetModelViewProjectionMatrix() const { return GetProjectionMatrix() * GetModelViewMatrix(); }

void MatrixManager::SetCurrentMatrix(const glm::mat4 m) { top() = m; }

void MatrixManager::MatrixModeModelView()
{
    _currentStack = &_modelviewStack;
    _mode = Mode::ModelView;
}

void MatrixManager::MatrixModeProjection()
{
    _currentStack = &_projectionStack;
    _mode = Mode::Projection;
}

void MatrixManager::PushMatrix() { _currentStack->push(top()); }

void MatrixManager::PopMatrix() { _currentStack->pop(); }

void MatrixManager::LoadMatrixd(const double *m) { top() = glm::make_mat4(m); }

void MatrixManager::GetDoublev(Mode mode, double *m) const
{
    const float *data = nullptr;
    if (mode == Mode::ModelView)
        data = glm::value_ptr(_modelviewStack.top());
    else if (mode == Mode::Projection)
        data = glm::value_ptr(_projectionStack.top());

    if (data) {
        for (int i = 0; i < 16; i++) m[i] = data[i];
    }
}

void MatrixManager::LoadIdentity() { top() = glm::mat4(1.0); }

void MatrixManager::Translate(float x, float y, float z) { top() = glm::translate(top(), vec3(x, y, z)); }

void MatrixManager::Scale(float x, float y, float z) { top() = glm::scale(top(), vec3(x, y, z)); }

void MatrixManager::Rotate(float angle, float x, float y, float z) { top() = glm::rotate(top(), angle, vec3(x, y, z)); }

void MatrixManager::Perspective(float fovy, float aspect, float zNear, float zFar)
{
    top() = glm::perspective(fovy, aspect, zNear, zFar);
    _projectionAspectRatio = aspect;

#ifndef NDEBUG
    Near = zNear;
    Far = zFar;
#endif
}

void MatrixManager::Ortho(float left, float right, float bottom, float top)
{
    this->top() = glm::ortho(left, right, bottom, top);
    _projectionAspectRatio = (right - left) / (top - bottom);
}

void MatrixManager::Ortho(float left, float right, float bottom, float top, float zNear, float zFar) { this->top() = glm::ortho(left, right, bottom, top, zNear, zFar); }

glm::vec2 MatrixManager::ProjectToScreen(float x, float y, float z) const { return ProjectToScreen(vec3(x, y, z)); }

glm::vec2 MatrixManager::ProjectToScreen(const glm::vec3 &v) const
{
    vec4 vs = GetModelViewProjectionMatrix() * vec4(v, 1.0f);
    vs /= vs.w;
    return vs;
}

float MatrixManager::GetProjectionAspectRatio() const
{
    VAssert(_projectionAspectRatio != 0);
    return _projectionAspectRatio;
}

#ifndef NDEBUG

int MatrixManager::GetGLMatrixMode()
{
    int mode;
    glGetIntegerv(GL_MATRIX_MODE, &mode);
    return mode;
}

const char *MatrixManager::GetGLMatrixModeStr()
{
    int mode = GetGLMatrixMode();
    switch (mode) {
    case GL_MODELVIEW: return "GL_MODELVIEW";
    case GL_PROJECTION: return "GL_PROJECTION";
    case GL_TEXTURE: return "GL_TEXTURE";
    case GL_COLOR: return "GL_COLOR";
    default: return "UNKNOWN_MODE";
    }
}

int MatrixManager::GetGLModelViewStackDepth()
{
    int depth;
    glGetIntegerv(GL_MODELVIEW_STACK_DEPTH, &depth);
    return depth;
}

int MatrixManager::GetGLProjectionStackDepth()
{
    int depth;
    glGetIntegerv(GL_PROJECTION_STACK_DEPTH, &depth);
    return depth;
}

int MatrixManager::GetGLCurrentStackDepth()
{
    if (GetGLMatrixMode() == GL_MODELVIEW) return GetGLModelViewStackDepth();
    if (GetGLMatrixMode() == GL_PROJECTION) return GetGLProjectionStackDepth();
    return -1;
}

const char *MatrixManager::GetMatrixModeStr()
{
    if (_mode == Mode::ModelView) return "ModelView";
    return "Projection";
}

#endif

glm::mat4 &MatrixManager::top() { return _currentStack->top(); }

const glm::mat4 &MatrixManager::top() const { return _currentStack->top(); }
