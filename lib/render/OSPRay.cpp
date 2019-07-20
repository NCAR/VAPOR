#include <vapor/OSPRay.h>
#include <glm/glm.hpp>

OSPError OSPInitStatus = OSP_UNKNOWN_ERROR;
const char *OSPInitStatusMessage = "";

bool OSPRayInitialized()
{
    return OSPInitStatus == OSP_NO_ERROR;
}

// Adapted from https://github.com/ospray/ospray/blob/release-1.2.x/apps/volumeViewer/glUtil/util.cpp#L42-L62

OSPTexture OSPRayGetDepthTextureFromOpenGLPerspective(const double &fovy,
                                                     const double &aspect,
                                                     const double &zNear,
                                                     const double &zFar,
                                                     const glm::vec3 &cameraDir,
                                                     const glm::vec3 &cameraUp,
                                                     const float *glDepthBuffer,
                                                     const int &width,
                                                     const int &height)
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
    
    osp::vec2i texSize = {width, height};
//    OSPTexture2D depthTexture = ospNewTexture2D((osp::vec2i&)texSize, OSP_TEXTURE_R32F, ospDepth, OSP_TEXTURE_FILTER_NEAREST);
    OSPTexture depthTexture = ospNewTexture("texture2d");
    ospSetVec2i(depthTexture, "size", texSize);
    ospSet1i(depthTexture, "type", OSP_TEXTURE_R32F);
    ospSet1i(depthTexture, "flags", OSP_TEXTURE_FILTER_NEAREST);
    
    OSPData data = ospNewData(width*height, OSP_FLOAT, ospDepth);
    ospCommit(data);
    ospSetData(depthTexture, "data", data);
    ospRelease(data);
    
    delete[] ospDepth;
    
    ospCommit(depthTexture);
    return depthTexture;
}
