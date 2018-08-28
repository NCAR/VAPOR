#include "vapor/GLManager.h"

using namespace VAPoR;

GLManager::GLManager() : shaderManager(new ShaderManager), fontManager(new FontManager(this)), matrixManager(new MatrixManager), legacy(new LegacyGL(this)) {}

GLManager::~GLManager()
{
    delete shaderManager;
    delete fontManager;
    delete matrixManager;
    delete legacy;
}
