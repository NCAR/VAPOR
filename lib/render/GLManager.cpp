#include "vapor/GLManager.h"
#include "vapor/LegacyGL.h"

using namespace VAPoR;

GLManager::GLManager() : shaderManager(new ShaderManager), matrixManager(new MatrixManager), legacy(new LegacyGL(this)) {}

GLManager::~GLManager()
{
    delete shaderManager;
    delete matrixManager;
    delete legacy;
}
