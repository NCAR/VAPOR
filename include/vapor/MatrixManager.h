#pragma once

#include <map>
#include <string>
#include <stack>
#include <glm/fwd.hpp>
#include <glm/mat4x4.hpp>

using std::map;
using std::stack;
using std::string;

namespace VAPoR {

class MatrixManager {
    enum class Mode { ModelView, Projection };

public:
    MatrixManager();

    glm::mat4 GetProjectionMatrix();
    glm::mat4 GetModelViewMatrix();
    glm::mat4 GetModelViewProjectionMatrix();

    void MatrixModeProjection();
    void MatrixModeModelView();
    void PushMatrix();
    void PopMatrix();

    void LoadMatrixd(const double *m);

    void LoadIdentity();
    void Translate(float x, float y, float z);
    void Scale(float x, float y, float z);
    void Rotate(float angle, float x, float y, float z);
    void Perspective(float fovy, float aspect, float zNear, float zFar);
    void Ortho(float left, float right, float bottom, float top);
    void Ortho(float left, float right, float bottom, float top, float zNear, float zFar);

    void        Test();
    void        TestUpload();
    int         GetGLMatrixMode();
    const char *GetGLMatrixModeStr();
    int         GetGLModelViewStackDepth();
    int         GetGLProjectionStackDepth();
    int         GetGLCurrentStackDepth();
    const char *GetMatrixModeStr();

private:
    stack<glm::mat4>  _modelviewStack;
    stack<glm::mat4>  _projectionStack;
    stack<glm::mat4> *_currentStack;
    Mode              _mode;

    glm::mat4 &      top();
    const glm::mat4 &top() const;
};

}    // namespace VAPoR
