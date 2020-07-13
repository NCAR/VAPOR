#pragma once
#include <ospray/ospray_util.h>
#include <string>
#include <vapor/MapperFunction.h>
#include <glm/glm.hpp>

namespace VOSP {
int Initialize(int *argc, char **argv);
int Shutdown();
std::string Version();
OSPData NewCopiedData(const void *data, OSPDataType type, uint64_t numItems1, uint64_t numItems2=1, uint64_t numItems3=1);
//void LoadTransferFunction(VAPoR::MapperFunction *vtf, OSPTransferFunction otf);
OSPTexture OSPDepthFromGLPerspective(float fovy, float aspect, float zNear, float zFar,
                                     glm::vec3 cameraDir, glm::vec3 cameraUp,
                                     const float *glDepthBuffer, int width, int height);

namespace Test {
OSPGeometricModel LoadTriangle(glm::vec3 scale = glm::vec3(1.f), const std::string &rendererType = "scivis");
}
}
