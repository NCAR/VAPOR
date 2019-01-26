#include "vapor/VolumeRenderer.h"
#include <vapor/VolumeParams.h>

#include <vapor/MatrixManager.h>
#include <vapor/GLManager.h>
#include <vapor/glutil.h>
#include <glm/glm.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

using namespace VAPoR;

static RendererRegistrar<VolumeRenderer> registrar( VolumeRenderer::GetClassType(),
                                                VolumeParams::GetClassType() );


VolumeRenderer::VolumeRenderer( const ParamsMgr*    pm,
                        std::string&        winName,
                        std::string&        dataSetName,
                        std::string&        instName,
                        DataMgr*            dataMgr )
          : Renderer(  pm,
                        winName,
                        dataSetName,
                        VolumeParams::GetClassType(),
                        VolumeRenderer::GetClassType(),
                        instName,
                        dataMgr )
{
    VAO = NULL;
    VBO = NULL;
    dataTexture = NULL;
    LUTTexture = NULL;
    algorithm = VolumeAlgorithm::NewAlgorithm("Regular");
}

VolumeRenderer::~VolumeRenderer()
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (dataTexture) glDeleteTextures(1, &dataTexture);
    if (LUTTexture) glDeleteTextures(1, &LUTTexture);
    if (cache.tf) delete cache.tf;
    if (algorithm) delete algorithm;
}

int VolumeRenderer::_initializeGL()
{
    float BL = -1;
    float data[] = {
        BL, BL,    0, 0,
        1, BL,    1, 0,
        BL,  1,    0, 1,
        
        BL,  1,    0, 1,
        1, BL,    1, 0,
        1,  1,    1, 1
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), NULL);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    glGenTextures(1, &dataTexture);
    glBindTexture(GL_TEXTURE_3D, dataTexture);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    glGenTextures(1, &LUTTexture);
    glBindTexture(GL_TEXTURE_1D, LUTTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    
    return 0;
}

#define CheckCache(cVar, pVar) \
if (cVar != pVar) { \
cache.needsUpdate = true; \
cVar = pVar; \
}

int VolumeRenderer::_paintGL(bool fast)
{
    VolumeParams *vp = (VolumeParams *)GetActiveParams();
    if (cache.algorithmName != vp->GetAlgorithm()) {
        cache.algorithmName = vp->GetAlgorithm();
        if (algorithm) delete algorithm;
        algorithm = VolumeAlgorithm::NewAlgorithm(cache.algorithmName);
        cache.needsUpdate = true;
    }
    
    _loadData();
    _loadTF();
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
    
    SmartShaderProgram shader(algorithm->GetShader(_glManager->shaderManager));
    if (!shader.IsValid())
        return -1;
    shader->SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());
    // shader->SetUniform("ModelView", _glManager->matrixManager->GetModelViewMatrix());
    // shader->SetUniform("Projection", _glManager->matrixManager->GetProjectionMatrix());
    // shader->SetUniform("resolution", vec2(resolution[0], resolution[1]));
    shader->SetUniform("cameraPos", vec3(cameraPos[0], cameraPos[1], cameraPos[2]));
    shader->SetUniform("dataBoundsMin", vec3(minExts[0], minExts[1], minExts[2]));
    shader->SetUniform("dataBoundsMax", vec3(maxExts[0], maxExts[1], maxExts[2]));
    shader->SetUniform("LUTMin", (float)cache.mapRange[0]);
    shader->SetUniform("LUTMax", (float)cache.mapRange[1]);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, dataTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, LUTTexture);
    
    shader->SetUniform("data", 0);
    shader->SetUniform("LUT", 1);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisable(GL_BLEND);
    
    glBindVertexArray(0);
    
    return 0;
}

void VolumeRenderer::_loadData()
{
    VolumeParams *RP = (VolumeParams *)GetActiveParams();
    CheckCache(cache.var, RP->GetVariableName());
    CheckCache(cache.ts, RP->GetCurrentTimestep());
    CheckCache(cache.refinement, RP->GetRefinementLevel());
    CheckCache(cache.compression, RP->GetCompressionLevel());
    if (!cache.needsUpdate)
        return;
    
    Grid *grid = _dataMgr->GetVariable(cache.ts, cache.var, cache.refinement, cache.compression);
    
    glBindTexture(GL_TEXTURE_3D, dataTexture);
    algorithm->LoadData(grid);
    delete grid;
}

void VolumeRenderer::_loadTF()
{
    MapperFunction *tf = GetActiveParams()->GetMapperFunc(cache.var);
    
    if (cache.tf && *cache.tf != *tf)
        cache.needsUpdate = true;
    
    if (!cache.needsUpdate)
        return;
    
    if (cache.tf) delete cache.tf;
    cache.tf = new MapperFunction(*tf);
    cache.mapRange = tf->getMinMaxMapValue();
    
    float *LUT = new float[4 * 256];
    tf->makeLut(LUT);
    
    glBindTexture(GL_TEXTURE_1D, LUTTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256, 0, GL_RGBA, GL_FLOAT, LUT);
    
    delete [] LUT;
}
