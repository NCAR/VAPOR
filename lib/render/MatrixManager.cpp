#include <vapor/glutil.h>
#include "vapor/MatrixManager.h"
#include <cassert>
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

void MatrixManager::PushMatrix()
{
    _currentStack->push(top());
#ifndef GLDEMO
    // printf("Push mode=%s depth=%li glMode=%s glDepth=%i\n", GetMatrixModeStr(), _currentStack->size(), GetGLMatrixModeStr(), GetGLCurrentStackDepth());
    // assert(GetGLCurrentStackDepth() == _currentStack->size());
#endif
}

void MatrixManager::PopMatrix()
{
    _currentStack->pop();
#ifndef GLDEMO
    // printf("Push mode=%s depth=%li glMode=%s glDepth=%i\n", GetMatrixModeStr(), _currentStack->size(), GetGLMatrixModeStr(), GetGLCurrentStackDepth());
    // assert(GetGLCurrentStackDepth() == _currentStack->size());
#endif
}

void MatrixManager::LoadMatrixd(const double *m) { top() = glm::make_mat4(m); }

void MatrixManager::GetDoublev(Mode mode, double *m) const
{
    const float *data;
    if (mode == Mode::ModelView)
        data = glm::value_ptr(_modelviewStack.top());
    else if (mode == Mode::Projection)
        data = glm::value_ptr(_projectionStack.top());

    for (int i = 0; i < 16; i++) m[i] = data[i];
}

void MatrixManager::LoadIdentity() { top() = glm::mat4(1.0); }

void MatrixManager::Translate(float x, float y, float z) { top() = glm::translate(top(), vec3(x, y, z)); }

void MatrixManager::Scale(float x, float y, float z) { top() = glm::scale(top(), vec3(x, y, z)); }

void MatrixManager::Rotate(float angle, float x, float y, float z) { top() = glm::rotate(top(), angle, vec3(x, y, z)); }

void MatrixManager::Perspective(float fovy, float aspect, float zNear, float zFar)
{
    top() = glm::perspective(fovy, aspect, zNear, zFar);

#ifndef NDEBUG
    Near = zNear;
    Far = zFar;
#endif
}

void MatrixManager::Ortho(float left, float right, float bottom, float top) { this->top() = glm::ortho(left, right, bottom, top); }

void MatrixManager::Ortho(float left, float right, float bottom, float top, float zNear, float zFar) { this->top() = glm::ortho(left, right, bottom, top, zNear, zFar); }

glm::vec2 MatrixManager::ProjectToScreen(float x, float y, float z) const { return ProjectToScreen(vec3(x, y, z)); }

glm::vec2 MatrixManager::ProjectToScreen(const glm::vec3 &v) const
{
    vec4 vs = GetModelViewProjectionMatrix() * vec4(v, 1.0f);
    vs /= vs.w;
    return vs;
}

#ifndef NDEBUG
    #ifndef GLDEMO
        #define GLM_ENABLE_EXPERIMENTAL 1
        #include <glm/gtx/string_cast.hpp>
void MatrixManager::Test()
{
        #ifndef Darwin
    // return;
    float gl_modelview_matrix[16], gl_projection_matrix[16];
    int   gl_modelview_stack_depth, gl_projection_stack_depth;

    glGetIntegerv(GL_MODELVIEW_STACK_DEPTH, &gl_modelview_stack_depth);
    glGetIntegerv(GL_PROJECTION_STACK_DEPTH, &gl_projection_stack_depth);
    glGetFloatv(GL_MODELVIEW_MATRIX, gl_modelview_matrix);
    glGetFloatv(GL_PROJECTION_MATRIX, gl_projection_matrix);

    if (gl_modelview_stack_depth != _modelviewStack.size()) printf("ModelView stack size not equal gl=%i, my=%i\n", gl_modelview_stack_depth, (int)_modelviewStack.size());
    if (gl_projection_stack_depth != _projectionStack.size()) printf("Projection stack size not equal gl=%i, my=%i\n", gl_projection_stack_depth, (int)_projectionStack.size());

    bool equal = true;
    for (int i = 0; i < 16; i++)
        if (gl_modelview_matrix[i] != glm::value_ptr(_modelviewStack.top())[i]) equal = false;
    if (!equal) {
        printf("ModelView matrix not equal\n");
        std::cout << "gl: " << glm::to_string(glm::make_mat4(gl_modelview_matrix)) << std::endl << "my: " << glm::to_string(_modelviewStack.top()) << std::endl;
    }
    equal = true;
    for (int i = 0; i < 16; i++)
        if (gl_projection_matrix[i] != glm::value_ptr(_projectionStack.top())[i]) equal = false;
    if (!equal) {
        printf("Projection matrix not equal\n");
        std::cout << "gl: " << glm::to_string(glm::make_mat4(gl_projection_matrix)) << std::endl << "my: " << glm::to_string(_projectionStack.top()) << std::endl;
    }
        #endif
}

void MatrixManager::TestUpload()
{
        #ifndef Darwin
    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(_modelviewStack.top()));
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(_projectionStack.top()));
    glPopAttrib();
        #endif
}

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
#endif

glm::mat4 &MatrixManager::top() { return _currentStack->top(); }

const glm::mat4 &MatrixManager::top() const { return _currentStack->top(); }
