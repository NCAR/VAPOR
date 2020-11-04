#pragma once

#include <map>
#include <string>
#include <stack>
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::map;
using std::stack;
using std::string;

namespace VAPoR {

//! \class MatrixManager
//! \ingroup Public_Render
//!
//! \brief Replaces the OpenGL matrix stack.
//!
//! Most functions are one to one replacements and have the same
//! effects as their OpenGL or GLUT equivalents.
//!
//! \author Stanislaw Jaroszynski
//! \date   August, 2018

class RENDER_API MatrixManager {
public:
    enum class Mode { ModelView, Projection };

    MatrixManager();

    glm::mat4 GetCurrentMatrix() const;
    glm::mat4 GetProjectionMatrix() const;
    glm::mat4 GetModelViewMatrix() const;
    glm::mat4 GetModelViewProjectionMatrix() const;
    void      SetCurrentMatrix(const glm::mat4 m);

    void MatrixModeProjection();
    void MatrixModeModelView();
    void PushMatrix();
    void PopMatrix();

    void LoadMatrixd(const double *m);
    void GetDoublev(Mode mode, double *m) const;

    void LoadIdentity();
    void Translate(float x, float y, float z);
    void Scale(float x, float y, float z);
    void Rotate(float angle, float x, float y, float z);
    void Perspective(float fovy, float aspect, float zNear, float zFar);
    void Ortho(float left, float right, float bottom, float top);
    void Ortho(float left, float right, float bottom, float top, float zNear, float zFar);

    glm::vec2 ProjectToScreen(float x, float y, float z) const;
    glm::vec2 ProjectToScreen(const glm::vec3 &v) const;

    float GetProjectionAspectRatio() const;

#ifndef NDEBUG
    int         GetGLMatrixMode();
    const char *GetGLMatrixModeStr();
    int         GetGLModelViewStackDepth();
    int         GetGLProjectionStackDepth();
    int         GetGLCurrentStackDepth();
    const char *GetMatrixModeStr();
#endif

    float Near, Far, FOV, Aspect;

private:
    stack<glm::mat4>  _modelviewStack;
    stack<glm::mat4>  _projectionStack;
    stack<glm::mat4> *_currentStack;
    Mode              _mode;
    float             _projectionAspectRatio = 0;

    glm::mat4 &      top();
    const glm::mat4 &top() const;
};

}    // namespace VAPoR
