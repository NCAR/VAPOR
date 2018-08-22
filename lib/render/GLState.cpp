#include <vapor/glutil.h>
#include "vapor/GLState.h"
#include <cassert>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace VAPoR;
using glm::mat4;
using glm::vec3;
using std::pair;

struct GLState::StateData {
    stack<glm::mat4>  modelviewStack;
    stack<glm::mat4>  projectionStack;
    stack<glm::mat4> *currentStack;
    Mode              mode;

    StateData();
    glm::mat4 &      top();
    const glm::mat4 &top() const;
};

map<string, GLState::StateData *> GLState::_stateMap;
GLState::StateData *              GLState::_currentState = nullptr;
string                            GLState::_currentStateName;

void GLState::CreateState(string name)
{
    assert(_stateMap.count(name) == 0);
    _stateMap.insert(pair<string, StateData *>(name, new StateData));
    if (!_currentState) SelectState(name);
}

void GLState::DeleteState(string name)
{
    int nErased = _stateMap.erase(name);
    assert(nErased == 1);
}

void GLState::SelectState(string name)
{
    auto it = _stateMap.find(name);
    assert(it != _stateMap.end());
    _currentStateName = it->first;
    _currentState = it->second;
}

mat4 GLState::GetProjectionMatrix()
{
    assert(_currentState);
    return _currentState->projectionStack.top();
}

mat4 GLState::GetModelViewMatrix()
{
    assert(_currentState);
    return _currentState->modelviewStack.top();
}

mat4 GLState::GetModelViewProjectionMatrix()
{
    assert(_currentState);
    return GLState::GetProjectionMatrix() * GLState::GetModelViewMatrix();
}

void GLState::MatrixModeModelView()
{
    assert(_currentState);
    _currentState->currentStack = &_currentState->modelviewStack;
    _currentState->mode = Mode::ModelView;
}

void GLState::MatrixModeProjection()
{
    assert(_currentState);
    _currentState->currentStack = &_currentState->projectionStack;
    _currentState->mode = Mode::Projection;
}

void GLState::PushMatrix()
{
    assert(_currentState);
    assert(_currentState->currentStack->size() < 128);    // Catch unmatched push/pop
    _currentState->currentStack->push(_currentState->top());
    printf("%s Push mode=%s depth=%li glMode=%s glDepth=%i\n", _currentStateName.c_str(), GetMatrixModeStr(), _currentState->currentStack->size(), GetGLMatrixModeStr(), GetGLCurrentStackDepth());
    assert(GetGLCurrentStackDepth() == _currentState->currentStack->size());
}

void GLState::PopMatrix()
{
    assert(_currentState);
    assert(_currentState->currentStack->size() > 1);
    _currentState->currentStack->pop();
    printf("%s Push mode=%s depth=%li glMode=%s glDepth=%i\n", _currentStateName.c_str(), GetMatrixModeStr(), _currentState->currentStack->size(), GetGLMatrixModeStr(), GetGLCurrentStackDepth());
    assert(GetGLCurrentStackDepth() == _currentState->currentStack->size());
}

void GLState::LoadMatrixd(const double *m)
{
    assert(_currentState);
    _currentState->top() = glm::make_mat4(m);
}

void GLState::LoadIdentity()
{
    assert(_currentState);
    _currentState->top() = glm::mat4(1.0);
}

void GLState::Translate(float x, float y, float z)
{
    assert(_currentState);
    _currentState->top() = glm::translate(_currentState->top(), vec3(x, y, z));
}

void GLState::Scale(float x, float y, float z)
{
    assert(_currentState);
    _currentState->top() = glm::scale(_currentState->top(), vec3(x, y, z));
}

void GLState::Rotate(float angle, float x, float y, float z)
{
    assert(_currentState);
    _currentState->top() = glm::rotate(_currentState->top(), angle, vec3(x, y, z));
}

void GLState::Perspective(float fovy, float aspect, float zNear, float zFar)
{
    assert(_currentState);
    _currentState->top() = glm::perspective(fovy, aspect, zNear, zFar);
}

void GLState::Ortho(float left, float right, float bottom, float top)
{
    assert(_currentState);
    _currentState->top() = glm::ortho(left, right, bottom, top);
}

void GLState::Ortho(float left, float right, float bottom, float top, float zNear, float zFar)
{
    assert(_currentState);
    _currentState->top() = glm::ortho(left, right, bottom, top, zNear, zFar);
}

#ifndef GLDEMO
    #define GLM_ENABLE_EXPERIMENTAL 1
    #include <glm/gtx/string_cast.hpp>
void GLState::Test()
{
    assert(_currentState);
    float gl_modelview_matrix[16], gl_projection_matrix[16];
    int   gl_modelview_stack_depth, gl_projection_stack_depth;

    glGetIntegerv(GL_MODELVIEW_STACK_DEPTH, &gl_modelview_stack_depth);
    glGetIntegerv(GL_PROJECTION_STACK_DEPTH, &gl_projection_stack_depth);
    glGetFloatv(GL_MODELVIEW_MATRIX, gl_modelview_matrix);
    glGetFloatv(GL_PROJECTION_MATRIX, gl_projection_matrix);

    if (gl_modelview_stack_depth != _currentState->modelviewStack.size()) printf("ModelView stack size not equal gl=%i, my=%i\n", gl_modelview_stack_depth, (int)_currentState->modelviewStack.size());
    if (gl_projection_stack_depth != _currentState->projectionStack.size())
        printf("Projection stack size not equal gl=%i, my=%i\n", gl_projection_stack_depth, (int)_currentState->projectionStack.size());

    bool equal = true;
    for (int i = 0; i < 16; i++)
        if (gl_modelview_matrix[i] != glm::value_ptr(_currentState->modelviewStack.top())[i]) equal = false;
    if (!equal) {
        printf("ModelView matrix not equal\n");
        std::cout << "gl: " << glm::to_string(glm::make_mat4(gl_modelview_matrix)) << std::endl << "my: " << glm::to_string(_currentState->modelviewStack.top()) << std::endl;
    }
    equal = true;
    for (int i = 0; i < 16; i++)
        if (gl_projection_matrix[i] != glm::value_ptr(_currentState->projectionStack.top())[i]) equal = false;
    if (!equal) {
        printf("Projection matrix not equal\n");
        std::cout << "gl: " << glm::to_string(glm::make_mat4(gl_projection_matrix)) << std::endl << "my: " << glm::to_string(_currentState->projectionStack.top()) << std::endl;
    }
}

void GLState::TestUpload()
{
    assert(_currentState);
    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(_currentState->modelviewStack.top()));
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(_currentState->projectionStack.top()));
    glPopAttrib();
}

int GLState::GetGLMatrixMode()
{
    int mode;
    glGetIntegerv(GL_MATRIX_MODE, &mode);
    return mode;
}

const char *GLState::GetGLMatrixModeStr()
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

int GLState::GetGLModelViewStackDepth()
{
    int depth;
    glGetIntegerv(GL_MODELVIEW_STACK_DEPTH, &depth);
    return depth;
}

int GLState::GetGLProjectionStackDepth()
{
    int depth;
    glGetIntegerv(GL_PROJECTION_STACK_DEPTH, &depth);
    return depth;
}

int GLState::GetGLCurrentStackDepth()
{
    if (GetGLMatrixMode() == GL_MODELVIEW) return GetGLModelViewStackDepth();
    if (GetGLMatrixMode() == GL_PROJECTION) return GetGLProjectionStackDepth();
    return -1;
}

const char *GLState::GetMatrixModeStr()
{
    if (_currentState->mode == Mode::ModelView) return "ModelView";
    return "Projection";
}

#endif

GLState::StateData::StateData()
{
    modelviewStack.push(glm::mat4(1.0));
    projectionStack.push(glm::mat4(1.0));
    currentStack = &modelviewStack;
    mode = Mode::ModelView;
}

glm::mat4 &GLState::StateData::top() { return currentStack->top(); }

const glm::mat4 &GLState::StateData::top() const { return currentStack->top(); }
