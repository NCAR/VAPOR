#pragma once
#ifdef BUILD_OSPRAY
    #include <ospray/ospray_util.h>
#endif
#include <string>
#include <glm/glm.hpp>
#include <vapor/common.h>

namespace VOSP {
RENDER_API int Initialize(int *argc, char **argv);
RENDER_API int Shutdown();
RENDER_API std::string Version();
RENDER_API bool        IsVersionAtLeast(int major, int minor);

#ifdef BUILD_OSPRAY
OSPData NewCopiedData(const void *data, OSPDataType type, uint64_t numItems1, uint64_t numItems2 = 1, uint64_t numItems3 = 1);
// void LoadTransferFunction(VAPoR::MapperFunction *vtf, OSPTransferFunction otf);
OSPTexture OSPDepthFromGLPerspective(float fovy, float aspect, float zNear, float zFar, glm::vec3 cameraDir, glm::vec3 cameraUp, const float *glDepthBuffer, int width, int height);

namespace Test {
OSPGeometricModel LoadTriangle(glm::vec3 scale = glm::vec3(1.f), const std::string &rendererType = "scivis");
}
#endif
}    // namespace VOSP
