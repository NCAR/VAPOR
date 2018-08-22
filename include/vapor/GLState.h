#pragma once

#include <map>
#include <string>
#include <stack>
#include <glm/fwd.hpp>
// #include <glm/glm.hpp>

using std::map;
using std::stack;
using std::string;

namespace VAPoR {

class GLState {
    struct StateData;
    enum class Mode { ModelView, Projection };

public:
    static void CreateState(string name);
    static void DeleteState(string name);
    static void SelectState(string name);

    static glm::mat4 GetProjectionMatrix();
    static glm::mat4 GetModelViewMatrix();
    static glm::mat4 GetModelViewProjectionMatrix();

    static void MatrixModeProjection();
    static void MatrixModeModelView();
    static void PushMatrix();
    static void PopMatrix();

    static void LoadMatrixd(const double *m);

    static void LoadIdentity();
    static void Translate(float x, float y, float z);
    static void Scale(float x, float y, float z);
    static void Rotate(float angle, float x, float y, float z);
    static void Perspective(float fovy, float aspect, float zNear, float zFar);
    static void Ortho(float left, float right, float bottom, float top);
    static void Ortho(float left, float right, float bottom, float top, float zNear, float zFar);

    static void        Test();
    static void        TestUpload();
    static int         GetGLMatrixMode();
    static const char *GetGLMatrixModeStr();
    static int         GetGLModelViewStackDepth();
    static int         GetGLProjectionStackDepth();
    static int         GetGLCurrentStackDepth();
    static const char *GetMatrixModeStr();

private:
    static map<string, StateData *> _stateMap;
    static StateData *              _currentState;
    static string                   _currentStateName;

    GLState() {}
    ~GLState() {}
};

}    // namespace VAPoR
