#include "vapor/VolumeRenderer.h"
#include <vapor/VolumeParams.h>

#include <vapor/MatrixManager.h>
#include <vapor/GLManager.h>
#include <vapor/glutil.h>
#include <glm/glm.hpp>

using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

using namespace VAPoR;

static RendererRegistrar<VolumeRenderer> registrar(VolumeRenderer::GetClassType(),
                                                   VolumeParams::GetClassType());

VolumeRenderer::VolumeRenderer(const ParamsMgr *pm,
                               std::string &winName,
                               std::string &dataSetName,
                               std::string &instName,
                               DataMgr *dataMgr)
    : Renderer(pm,
               winName,
               dataSetName,
               VolumeParams::GetClassType(),
               VolumeRenderer::GetClassType(),
               instName,
               dataMgr) {
    VAO = 0;
    VBO = 0;
    dataTexture = 0;
}

VolumeRenderer::~VolumeRenderer() {
    if (VAO)
        glDeleteVertexArrays(1, &VAO);
    if (VBO)
        glDeleteBuffers(1, &VBO);
    if (dataTexture)
        glDeleteTextures(1, &dataTexture);
    if (LUTTexture)
        glDeleteTextures(1, &LUTTexture);
    if (rayDirTexture)
        glDeleteTextures(1, &rayDirTexture);
    if (cache.tf)
        delete cache.tf;
}

int VolumeRenderer::_initializeGL() {
    float BL = -1;
    float data[] = {
        BL, BL, 0, 0,
        1, BL, 1, 0,
        BL, 1, 0, 1,

        BL, 1, 0, 1,
        1, BL, 1, 0,
        1, 1, 1, 1};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), NULL);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glGenTextures(1, &dataTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, dataTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &LUTTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, LUTTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &rayDirTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, rayDirTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return 0;
}

int VolumeRenderer::_paintGL(bool fast) {
    _loadData();
    _loadTF();
    // _createRayDirTexture();
    cache.needsUpdate = false;

    GLint viewport[4] = {0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    float resolution[2] = {static_cast<float>(viewport[2]), static_cast<float>(viewport[3])};

    Viewpoint *VP = _paramsMgr->GetViewpointParams(_winName)->getCurrentViewpoint();
    double m[16];
    double cameraPos[3], cameraUp[3], cameraDir[3];
    _glManager->matrixManager->GetDoublev(MatrixManager::Mode::ModelView, m);
    VP->ReconstructCamera(m, cameraPos, cameraUp, cameraDir);

    vector<double> minExts, maxExts;
    GetActiveParams()->GetBox()->GetExtents(minExts, maxExts);

    SmartShaderProgram shader = _glManager->shaderManager->GetSmartShader("ray");
    if (!shader.IsValid())
        return -1;
    shader->SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());
    shader->SetUniform("resolution", vec2(resolution[0], resolution[1]));
    shader->SetUniform("cameraPos", vec3(cameraPos[0], cameraPos[1], cameraPos[2]));
    shader->SetUniform("dataBoundsMin", vec3(minExts[0], minExts[1], minExts[2]));
    shader->SetUniform("dataBoundsMax", vec3(maxExts[0], maxExts[1], maxExts[2]));
    shader->SetUniform("LUTMin", (float)cache.mapRange[0]);
    shader->SetUniform("LUTMax", (float)cache.mapRange[1]);
    shader->SetUniform("data", 0);
    shader->SetUniform("LUT", 1);
    shader->SetUniform("dirs", 2);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisable(GL_BLEND);

    glBindVertexArray(0);

    return 0;
}

#define CheckCache(cVar, pVar)    \
    if (cVar != pVar) {           \
        cache.needsUpdate = true; \
        cVar = pVar;              \
    }

void VolumeRenderer::_loadData() {
    RenderParams *RP = GetActiveParams();
    CheckCache(cache.var, RP->GetVariableName());
    CheckCache(cache.ts, RP->GetCurrentTimestep());
    CheckCache(cache.refinement, RP->GetRefinementLevel());
    CheckCache(cache.compression, RP->GetCompressionLevel());
    if (!cache.needsUpdate)
        return;

    Grid *grid = _dataMgr->GetVariable(cache.ts, cache.var, cache.refinement, cache.compression);

    vector<size_t> dims = grid->GetDimensions();
    float *data = new float[dims[0] * dims[1] * dims[2]];

    size_t i = 0;
    auto end = grid->cend();
    for (auto it = grid->cbegin(); it != end; ++it, ++i) {
        data[i] = *it;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, dataTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, dims[0], dims[1], dims[2], 0, GL_RED, GL_FLOAT, data);

    delete[] data;
    delete grid;
}

void VolumeRenderer::_loadTF() {
    MapperFunction *tf = GetActiveParams()->GetMapperFunc(cache.var);

    if (cache.tf && *cache.tf != *tf)
        cache.needsUpdate = true;

    if (!cache.needsUpdate)
        return;

    if (cache.tf)
        delete cache.tf;
    cache.tf = new MapperFunction(*tf);
    cache.mapRange = tf->getMinMaxMapValue();

    float *LUT = new float[4 * 256];
    tf->makeLut(LUT);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, LUTTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_FLOAT, LUT);

    delete[] LUT;
}

void VolumeRenderer::_createRayDirTexture() {
    GLint viewport[4] = {0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    int width = viewport[2];
    int height = viewport[3];
    // width /= 4;
    // height /= 4;
    vec2 size(width, height);
    printf("size = %f, %f\n", size.x, size.y);

    Viewpoint *VP = _paramsMgr->GetViewpointParams(_winName)->getCurrentViewpoint();
    double m[16];
    double cameraPos[3], cameraUp[3], cameraDir[3];
    _glManager->matrixManager->GetDoublev(MatrixManager::Mode::ModelView, m);
    VP->ReconstructCamera(m, cameraPos, cameraUp, cameraDir);
    vec3 camera(cameraPos[0], cameraPos[1], cameraPos[2]);

    vec3 *dirs = new vec3[width * height];

    /*
     vec2 screen = ST*2-1;
     vec4 world = inverse(MVP) * vec4(screen, 1, 0.996f);
     vec3 dir = normalize(world.xyz);
     */
    MatrixManager *mm = _glManager->matrixManager;
    mat4 invMVP = glm::inverse(mm->GetModelViewProjectionMatrix());

    double modelView[16], projection[16];
    mm->GetDoublev(MatrixManager::Mode::ModelView, modelView);
    mm->GetDoublev(MatrixManager::Mode::Projection, projection);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float xf = x, yf = y;
            //            vec2 pixel(xf, yf);
            //            vec2 screen = pixel/size * 2.f - 1.f;
            //            vec4 world = invMVP * vec4(screen, 1, 1);
            //            vec3 dir = glm::normalize(vec3(world));

            double worldX, worldY, worldZ;
            // gluUnProject(x, y, 1, modelView, projection, viewport, &worldX, &worldY, &worldZ);
            vec3 world(worldX, worldY, worldZ);
            // glhUnProjectf(x, y, 1, invMVP, viewport, world);
            vec3 dir = glm::normalize(world - camera);

            dirs[y * width + x] = dir;
        }
    }

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, rayDirTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, dirs);
}
