#include "vapor/VolumeRenderer.h"
#include <vapor/VolumeParams.h>

#include <vapor/MatrixManager.h>
#include <vapor/GLManager.h>
#include <vapor/glutil.h>
#include <glm/glm.hpp>
#include <vapor/VolumeRegular.h>
#include <vapor/VolumeCellTraversal.h>

using std::vector;
using std::string;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

using namespace VAPoR;

static RendererRegistrar<VolumeRenderer> registrar( VolumeRenderer::GetClassType(),
                                                VolumeParams::GetClassType() );


VolumeRenderer::VolumeRenderer(
                        const ParamsMgr* pm,
                        std::string&     winName,
                        std::string&     dataSetName,
                        std::string&     instName,
                        DataMgr*         dataMgr )
: VolumeRenderer(
           pm,
           winName,
           dataSetName,
           VolumeParams::GetClassType(),
           VolumeRenderer::GetClassType(),
           instName,
           dataMgr)
{}

VolumeRenderer::VolumeRenderer(
               const ParamsMgr* pm,
               std::string&     winName,
               std::string&     dataSetName,
               std::string      paramsType,
               std::string      classType,
               std::string&     instName,
               DataMgr*         dataMgr )
          : Renderer(pm,
                     winName,
                     dataSetName,
                     paramsType,
                     classType,
                     instName,
                     dataMgr)
{
    VAO = NULL;
    VBO = NULL;
    LUTTexture = NULL;
    depthTexture = NULL;
    algorithm = NULL;
    lastRenderTime = 100;
    
    if (_needToSetDefaultAlgorithm()) {
        VolumeParams *vp = (VolumeParams*)GetActiveParams();
        Grid *grid = _dataMgr->GetVariable(vp->GetCurrentTimestep(), vp->GetVariableName(), vp->GetRefinementLevel(), vp->GetCompressionLevel());
        string algorithmName = _getDefaultAlgorithmForGrid(grid);
        vp->SetAlgorithm(algorithmName);
        delete grid;
    }
}

VolumeRenderer::~VolumeRenderer()
{
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (LUTTexture)   glDeleteTextures(1, &LUTTexture);
    if (depthTexture) glDeleteTextures(1, &depthTexture);
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
    
    glGenTextures(1, &LUTTexture);
    glBindTexture(GL_TEXTURE_1D, LUTTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    return 0;
}

#define CheckCache(cVar, pVar) \
if (cVar != pVar) { \
cache.needsUpdate = true; \
cVar = pVar; \
}

int VolumeRenderer::_paintGL(bool fast)
{
    if (fast && lastRenderTime > 0.1)
        return 0;
    
    VolumeParams *vp = (VolumeParams *)GetActiveParams();
    if (cache.algorithmName != vp->GetAlgorithm()) {
        cache.algorithmName = vp->GetAlgorithm();
        if (algorithm) delete algorithm;
        algorithm = VolumeAlgorithm::NewAlgorithm(cache.algorithmName, _glManager);
        cache.needsUpdate = true;
    }
    
    if (_loadData() < 0) return -1;
    if (_loadSecondaryData() < 0) return -1;
    _loadTF();
    cache.needsUpdate = false;
    
    GLint viewport[4] = {0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    // float resolution[2] = {static_cast<float>(viewport[2]), static_cast<float>(viewport[3])};
    
    Viewpoint *VP = _paramsMgr->GetViewpointParams(_winName)->getCurrentViewpoint();
    double m[16];
    double cameraPos[3], cameraUp[3], cameraDir[3];
    _glManager->matrixManager->GetDoublev(MatrixManager::Mode::ModelView, m);
    VP->ReconstructCamera(m, cameraPos, cameraUp, cameraDir);
    
    glm::vec3 dataMin, dataMax, userMin, userMax;
    _getExtents(&dataMin, &dataMax, &userMin, &userMax);
    vec3 extLengths = dataMax - dataMin;
    vec3 extScales = _getVolumeScales();
    vec3 extLengthsScaled = extLengths * extScales;
    float smallestDimension = min(extLengthsScaled[0], min(extLengthsScaled[1], extLengthsScaled[2]));
    
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,   viewport[0], viewport[1], viewport[2], viewport[3], 0);
    
    SmartShaderProgram shader(algorithm->GetShader());
    if (!shader.IsValid())
        return -1;
    shader->SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());
    shader->SetUniform("cameraPos", vec3(cameraPos[0], cameraPos[1], cameraPos[2]));
    shader->SetUniform("dataBoundsMin", dataMin);
    shader->SetUniform("dataBoundsMax", dataMax);
    shader->SetUniform("userExtsMin", userMin);
    shader->SetUniform("userExtsMax", userMax);
    shader->SetUniform("LUTMin", (float)cache.mapRange[0]);
    shader->SetUniform("LUTMax", (float)cache.mapRange[1]);
    shader->SetUniform("unitDistance", smallestDimension/100.f);
    shader->SetUniform("scales", extScales);
    shader->SetUniform("lightingEnabled", vp->GetLightingEnabled());
    shader->SetUniform("phongAmbient",   vp->GetPhongAmbient());
    shader->SetUniform("phongDiffuse",   vp->GetPhongDiffuse());
    shader->SetUniform("phongSpecular",  vp->GetPhongSpecular());
    shader->SetUniform("phongShininess", vp->GetPhongShininess());
    if (shader->HasUniform("isoValue")) {
        vector<double> isoValuesD = vp->GetIsoValues();
        vector<float> isoValues(isoValuesD.begin(), isoValuesD.end());
        vector<bool> enabledIsoValues = vp->GetEnabledIsoValues();
        shader->SetUniformArray("isoValue", 4, isoValues.data());
        shader->SetUniform("isoEnabled[0]", (bool)enabledIsoValues[0]);
        shader->SetUniform("isoEnabled[1]", (bool)enabledIsoValues[1]);
        shader->SetUniform("isoEnabled[2]", (bool)enabledIsoValues[2]);
        shader->SetUniform("isoEnabled[3]", (bool)enabledIsoValues[3]);
    }
    if (cache.constantColor.size() == 4)
        shader->SetUniform("constantColor", *(vec4*)cache.constantColor.data());
    
    algorithm->SetUniforms();
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, LUTTexture);
    shader->SetUniform("LUT", 1);
    
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    shader->SetUniform("sceneDepth", 7);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glBindVertexArray(VAO);
    
    void *start = GLManager::BeginTimer();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    lastRenderTime = GLManager::EndTimer(start);
    printf("Render time = %f\n", lastRenderTime);
    
    glDisable(GL_BLEND);
    GL_ERR_BREAK();
    
    glBindVertexArray(0);
    
    return 0;
}

int VolumeRenderer::_loadData()
{
    VolumeParams *RP = (VolumeParams *)GetActiveParams();
    CheckCache(cache.var, RP->GetVariableName());
    CheckCache(cache.ts, RP->GetCurrentTimestep());
    CheckCache(cache.refinement, RP->GetRefinementLevel());
    CheckCache(cache.compression, RP->GetCompressionLevel());
    if (!cache.needsUpdate)
        return 0;
    
    Grid *grid = _dataMgr->GetVariable(cache.ts, cache.var, cache.refinement, cache.compression);
    
    if (_needToSetDefaultAlgorithm()) {
        if (algorithm) delete algorithm;
        string algorithmName = _getDefaultAlgorithmForGrid(grid);
        algorithm = VolumeAlgorithm::NewAlgorithm(algorithmName, _glManager);
        RP->SetAlgorithm(algorithmName);
    }
    
    int ret = algorithm->LoadData(grid);
    delete grid;
    return ret;
}

bool VolumeRenderer::_usingColorMapData() const
{
    return false;
}

int VolumeRenderer::_loadSecondaryData()
{
    VolumeParams *vp = (VolumeParams *)GetActiveParams();
    CheckCache(cache.useColorMapVar, _usingColorMapData());
    CheckCache(cache.colorMapVar, vp->GetColorMapVariableName());
    if (!cache.needsUpdate)
        return 0;
    
    if (cache.useColorMapVar) {
        Grid *grid = _dataMgr->GetVariable(cache.ts, cache.colorMapVar, cache.refinement, cache.compression);
        int ret = algorithm->LoadSecondaryData(grid);
        delete grid;
        return ret;
    } else {
        algorithm->DeleteSecondaryData();
        return 0;
    }
}

void VolumeRenderer::_loadTF()
{
    VolumeParams *vp = (VolumeParams *)GetActiveParams();
    MapperFunction *tf;
    if (cache.useColorMapVar) {
        tf = vp->GetMapperFunc(cache.colorMapVar);
    } else {
        tf = vp->GetMapperFunc(cache.var);
        vector<float> constantColor = vp->GetConstantColor();
        constantColor.push_back(tf->getOpacityScale());
        CheckCache(cache.constantColor, constantColor);
    }
    
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

glm::vec3 VolumeRenderer::_getVolumeScales() const
{
    ViewpointParams *vpp = _paramsMgr->GetViewpointParams(_winName);
    Transform *datasetTransform = vpp->GetTransform(GetMyDatasetName());
    Transform *rendererTransform = GetActiveParams()->GetTransform();
    assert(datasetTransform && rendererTransform);
 
    vector<double> datasetScales, rendererScales;
    datasetScales = datasetTransform->GetScales();
    rendererScales = rendererTransform->GetScales();
    
    return glm::vec3 (
        datasetScales[0] * rendererScales[0],
        datasetScales[1] * rendererScales[1],
        datasetScales[2] * rendererScales[2]
    );
}

void VolumeRenderer::_getExtents(glm::vec3 *dataMin, glm::vec3 *dataMax, glm::vec3 *userMin, glm::vec3 *userMax) const
{
    VolumeParams *vp = (VolumeParams *)GetActiveParams();
    vector<double> dMinExts, dMaxExts;
    vp->GetBox()->GetExtents(dMinExts, dMaxExts);
    *userMin = vec3(dMinExts[0], dMinExts[1], dMinExts[2]);
    *userMax = vec3(dMaxExts[0], dMaxExts[1], dMaxExts[2]);
    _dataMgr->GetVariableExtents(cache.ts, cache.var, cache.refinement, dMinExts, dMaxExts);
    *dataMin = vec3(dMinExts[0], dMinExts[1], dMinExts[2]);
    *dataMax = vec3(dMaxExts[0], dMaxExts[1], dMaxExts[2]);
}

std::string VolumeRenderer::_getDefaultAlgorithmForGrid(const Grid *grid) const
{
    const RegularGrid* regular = dynamic_cast<const RegularGrid*>(grid);
    if (regular)
        return VolumeRegular::GetName();
    return VolumeCellTraversal::GetName();
}

bool VolumeRenderer::_needToSetDefaultAlgorithm() const
{
    return !((VolumeParams*)GetActiveParams())->GetAlgorithmWasManuallySetByUser();
}
