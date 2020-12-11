#ifdef WIN32
#define _USE_MATH_DEFINES
#include <cmath>
#endif

#include <vapor/OSPRay.h>
#include <stdio.h>
#include <stdlib.h>

using namespace VOSP;

#ifdef BUILD_OSPRAY
static int _initialized = false;

void ospErrorCallback(OSPError error, const char *msg)
{
    fprintf(stderr, "OSP error ");
    switch (error) {
#define STRINGIFY(x) case x: fprintf(stderr, #x); break;
        STRINGIFY(OSP_NO_ERROR)
        STRINGIFY(OSP_UNKNOWN_ERROR)
        STRINGIFY(OSP_INVALID_ARGUMENT)
        STRINGIFY(OSP_INVALID_OPERATION)
        STRINGIFY(OSP_OUT_OF_MEMORY)
        STRINGIFY(OSP_UNSUPPORTED_CPU)
        STRINGIFY(OSP_VERSION_MISMATCH)
#undef STRINGIFY
    }
    fprintf(stderr, ": %s\n", msg);
//    exit(1);
}
#endif

int VOSP::Initialize(int *argc, char **argv)
{
#ifdef BUILD_OSPRAY
    if (_initialized)
        return 0;
    
    OSPError init_error = ospInit(argc, (const char **)argv);
    if (init_error != OSP_NO_ERROR)
      return init_error;
    
// Hide deprecated warning.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    // If this is not set, OSPRay will crash upon shutdown
    ospDeviceSetErrorFunc(ospGetCurrentDevice(), ospErrorCallback);
#pragma GCC diagnostic pop
    _initialized = true;
    return 0;
#else
    return 0;
#endif
}


int VOSP::Shutdown()
{
//    ospShutdown(); // Broken. Do not use.
    return 0;
}


std::string VOSP::Version()
{
#ifdef BUILD_OSPRAY
    long major = ospDeviceGetProperty(ospGetCurrentDevice(), OSP_DEVICE_VERSION_MAJOR);
    long minor = ospDeviceGetProperty(ospGetCurrentDevice(), OSP_DEVICE_VERSION_MINOR);
    long patch = ospDeviceGetProperty(ospGetCurrentDevice(), OSP_DEVICE_VERSION_PATCH);
    return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
#else
    return "0.0.0";
#endif
}

bool VOSP::IsVersionAtLeast(int testMajor, int testMinor)
{
#ifdef BUILD_OSPRAY
    long major = ospDeviceGetProperty(ospGetCurrentDevice(), OSP_DEVICE_VERSION_MAJOR);
    long minor = ospDeviceGetProperty(ospGetCurrentDevice(), OSP_DEVICE_VERSION_MINOR);
    
    if (major > testMajor || (major == testMajor && minor >= testMinor))
        return true;
    
    return false;
    
#else
    return false;
#endif
}

#ifdef BUILD_OSPRAY

OSPData VOSP::NewCopiedData(const void *data, OSPDataType type, uint64_t numItems1, uint64_t numItems2, uint64_t numItems3)
{
    OSPData shared = ospNewSharedData(data, type, numItems1, 0, numItems2, 0, numItems3, 0);
    ospCommit(shared);
    OSPData opaque = ospNewData(type, numItems1, numItems2, numItems3);
    ospCommit(opaque);
    ospCopyData(shared, opaque);
    ospCommit(opaque);
    ospRelease(shared);
    return opaque;
}


OSPTexture VOSP::OSPDepthFromGLPerspective(float fovy, float aspect, float zNear, float zFar,
                                           glm::vec3 cameraDir, glm::vec3 cameraUp,
                                           const float *glDepthBuffer, int width, int height)
{
    float *ospDepth = new float[width * height];
    
    // transform OpenGL depth to linear depth
    for (size_t i=0; i<width*height; i++) {
        const double z_n = 2.0 * glDepthBuffer[i] - 1.0;
        ospDepth[i] = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));
    }
    
    // transform from orthogonal Z depth to ray distance t
    glm::vec3 dir_du = normalize(cross(cameraDir, cameraUp));
    glm::vec3 dir_dv = normalize(cross(dir_du, cameraDir));
    
    const float imagePlaneSizeY = 2.f * tanf(fovy/2.f * M_PI/180.f);
    const float imagePlaneSizeX = imagePlaneSizeY * aspect;
    
    dir_du *= imagePlaneSizeX;
    dir_dv *= imagePlaneSizeY;
    
    const glm::vec3 dir_00 = cameraDir - .5f * dir_du - .5f * dir_dv;
    
    for (size_t j=0; j<height; j++) {
        for (size_t i=0; i<width; i++) {
            const glm::vec3 dir_ij = normalize(dir_00 + float(i)/float(width-1) * dir_du + float(j)/float(height-1) * dir_dv);
            
            const float t = ospDepth[j*width+i] / dot(cameraDir, dir_ij);
            ospDepth[j*width+i] = t;
        }
    }
    
    OSPTexture depthTexture = ospNewTexture("texture2d");
    ospSetInt(depthTexture, "format", OSP_TEXTURE_R32F);
    ospSetInt(depthTexture, "filter", OSP_TEXTURE_FILTER_NEAREST);
    
    OSPData data = VOSP::NewCopiedData(ospDepth, OSP_FLOAT, width, height);
    ospCommit(data);
    ospSetObject(depthTexture, "data", data);
    ospRelease(data);
    
    delete[] ospDepth;
    
    ospCommit(depthTexture);
    return depthTexture;
}




// triangle mesh data
namespace TriData {
float translation[] = {0,0,0};
float vertex[] = {
    -1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
    1.0f,  -1.0f, 0.0f,
    1.0f,   1.0f, 1.0f};
float color[] = {
    0.9f, 0.5f, 0.5f, 1.0f,
    0.8f, 0.8f, 0.8f, 1.0f,
    0.8f, 0.8f, 0.8f, 1.0f,
    0.5f, 0.9f, 0.5f, 1.0f};
int32_t index[] = {0, 1, 2, 1, 2, 3};
}

OSPGeometricModel Test::LoadTriangle(glm::vec3 scale, const std::string &rendererType)
{
    // Translate Triangle
    float pos[4*3];
    for (int i = 0; i < 4*3; i++) {
        pos[i] = TriData::vertex[i] * scale[i%3] + TriData::translation[i%3];
    }
    
    // create and setup model and mesh
    OSPGeometry mesh = ospNewGeometry("mesh");

    OSPData data = NewCopiedData(pos, OSP_VEC3F, 4);
    // alternatively with an OSPRay managed OSPData
    // OSPData managed = ospNewData1D(OSP_VEC3F, 4);
    // ospCopyData1D(data, managed, 0);

    ospCommit(data);
    ospSetObject(mesh, "vertex.position", data);
    ospRelease(data); // we are done using this handle

    data = ospNewSharedData1D(TriData::color, OSP_VEC4F, 4);
    ospCommit(data);
    ospSetObject(mesh, "vertex.color", data);
    ospRelease(data);

    data = ospNewSharedData1D(TriData::index, OSP_VEC3UI, 2);
    ospCommit(data);
    ospSetObject(mesh, "index", data);
    ospRelease(data);

    ospCommit(mesh);

    OSPMaterial mat = ospNewMaterial(rendererType.c_str(), "obj");
    ospCommit(mat);

    // put the mesh into a model
    OSPGeometricModel model = ospNewGeometricModel(mesh);
    ospSetObject(model, "material", mat);
    ospCommit(model);
    ospRelease(mesh);
    ospRelease(mat);
    
    return model;
}

#endif
