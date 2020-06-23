#include "vapor/glutil.h"
#include "vapor/GLManager.h"
#include "vapor/LegacyGL.h"
#include "vapor/FontManager.h"
#include "vapor/STLUtils.h"
#include <chrono>

using namespace VAPoR;
using namespace std::chrono;

GLManager::Vendor GLManager::_cachedVendor = Vendor::Unknown;

GLManager::GLManager() : shaderManager(new ShaderManager), fontManager(new FontManager(this)), matrixManager(new MatrixManager), legacy(new LegacyGL(this)) { _queryVendor(); }

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

glm::vec2 GLManager::GetViewportSize()
{
    const auto v = GetViewport();
    return glm::vec2(v[2] - v[0], v[3] - v[1]);
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

GLManager::Vendor GLManager::GetVendor() { return _cachedVendor; }

void GLManager::_queryVendor()
{
    string vendorString((const char *)glGetString(GL_VERSION));
    vendorString = STLUtils::ToLower(vendorString);

    if (STLUtils::Contains(vendorString, "intel"))
        _cachedVendor = Vendor::Intel;
    else if (STLUtils::Contains(vendorString, "nvidia"))
        _cachedVendor = Vendor::Nvidia;
    else if (STLUtils::Contains(vendorString, "amd"))
        _cachedVendor = Vendor::AMD;
    else if (STLUtils::Contains(vendorString, "mesa"))
        _cachedVendor = Vendor::Mesa;
    else
        _cachedVendor = Vendor::Other;
}

void GLManager::GetGLVersion(int *major, int *minor)
{
    // Only >=3.0 guarentees glGetIntegerv

    string version = string((const char *)glGetString(GL_VERSION));
    version = version.substr(0, version.find(" "));
    const string majorString = version.substr(0, version.find("."));
    *major = std::stoi(majorString);
    if (majorString.length() < version.length()) {
        version = version.substr(majorString.length() + 1);
        const string minorString = version.substr(0, version.find("."));
        if (!minorString.empty())
            *minor = std::stoi(minorString);
        else
            *minor = 0;
    }
}

int GLManager::GetGLSLVersion()
{
    int major, minor;
    GetGLVersion(&major, &minor);

    // This does not work for < OpenGL 3.3 but we do not support it anyway
    return 100 * major + 10 * minor;
}

bool GLManager::IsCurrentOpenGLVersionSupported()
{
    int major, minor;
    GetGLVersion(&major, &minor);

    int version = major * 100 + minor;

    if (version >= 303) return true;

    Wasp::MyBase::SetErrMsg("OpenGL Version \"%s\" is too low and is not supported", glGetString(GL_VERSION));
    return false;
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
    // Since this is just for debugging purposes, initialized stays false to support use in multiple contexts
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

void *GLManager::BeginTimer()
{
    glFinish();
    auto start = new chrono::time_point<chrono::high_resolution_clock>;
    *start = chrono::high_resolution_clock::now();
    return start;
}

double GLManager::EndTimer(void *startTime)
{
    VAssert(startTime);
    auto start = (chrono::time_point<chrono::high_resolution_clock> *)startTime;

    glFinish();
    auto stop = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(stop - *start);

    delete start;

    return (double)duration.count() / 1000000.0;
}
