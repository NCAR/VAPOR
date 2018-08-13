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

public:
    static void CreateState(string name);
    static void DeleteState(string name);
    static void SelectState(string name);

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

    static void Test();
    static void TestUpload();

private:
    static map<string, StateData *> _stateMap;
    static StateData *              _currentState;
    static string                   _currentStateName;

    GLState() {}
    ~GLState() {}
};

}    // namespace VAPoR
