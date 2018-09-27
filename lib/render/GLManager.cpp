#include "vapor/glutil.h"
#include "vapor/GLManager.h"
#include "vapor/LegacyGL.h"

using namespace VAPoR;

GLManager::GLManager() : shaderManager(new ShaderManager), fontManager(new FontManager(this)), matrixManager(new MatrixManager), legacy(new LegacyGL(this)) {}

GLManager::~GLManager()
{
    delete shaderManager;
    delete fontManager;
    delete matrixManager;
    delete legacy;
}

std::vector<int> GLManager::GetViewport()
{
    GLint v[4] = {0};
    glGetIntegerv(GL_VIEWPORT, v);
    return {v[0], v[1], v[2], v[3]};
}

void GLManager::PixelCoordinateSystemPush()
{
    MatrixManager *  mm = matrixManager;
    std::vector<int> viewport = GLManager::GetViewport();

    mm->MatrixModeProjection();
    mm->PushMatrix();
    mm->Ortho(viewport[0], viewport[2], viewport[1], viewport[3]);
    mm->MatrixModeModelView();
    mm->PushMatrix();
    mm->LoadIdentity();
}

void GLManager::PixelCoordinateSystemPop()
{
    MatrixManager *mm = matrixManager;
    mm->PopMatrix();
    mm->MatrixModeProjection();
    mm->PopMatrix();
    mm->MatrixModeModelView();
}

bool GLManager::CheckError()
{
    int err = glGetError();
    if (err != GL_NO_ERROR) { return false; }
    return true;
}

#ifndef NDEBUG
void GLManager::ShowDepthBuffer()
{
    static bool         initialized = false;
    static unsigned int VAO = 0;
    static unsigned int VBO = 0;
    static unsigned int texID = 0;
    static float        BL = 0.5;
    static float        data[] = {BL, BL, 0, 0, 1, BL, 1, 0, BL, 1, 0, 1,

                           BL, 1,  0, 1, 1, BL, 1, 0, 1,  1, 1, 1};
    if (!initialized) {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glGenTextures(1, &texID);
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    GLint viewport[4] = {0};
    glGetIntegerv(GL_VIEWPORT, viewport);

    glBindTexture(GL_TEXTURE_2D, texID);
    // glReadBuffer(GL_BACK);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, viewport[0], viewport[1], viewport[2], viewport[3], 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glBindVertexArray(VAO);
    SmartShaderProgram shader = shaderManager->GetSmartShader("DepthBuffer");
    shader->SetUniform("near", matrixManager->Near);
    shader->SetUniform("far", matrixManager->Far);
    shader->SetUniform("linearize", true);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
#endif
