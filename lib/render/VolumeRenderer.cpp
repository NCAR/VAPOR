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
    _VAO = NULL;
    _VBO = NULL;
    _VAOChunked = NULL;
    _VBOChunked = NULL;
    _LUTTexture = NULL;
    _depthTexture = NULL;
    _algorithm = NULL;
    _lastRenderTime = 100;
    
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
    if (_VAO) glDeleteVertexArrays(1, &_VAO);
    if (_VBO) glDeleteBuffers(1, &_VBO);
    if (_VAOChunked) glDeleteVertexArrays(1, &_VAOChunked);
    if (_VBOChunked) glDeleteBuffers(1, &_VBOChunked);
    if (_LUTTexture)   glDeleteTextures(1, &_LUTTexture);
    if (_depthTexture) glDeleteTextures(1, &_depthTexture);
    if (_cache.tf) delete _cache.tf;
    if (_algorithm) delete _algorithm;
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
    
    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);
    glBindVertexArray(_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), NULL);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    vector<vec4> d;
    float s = 2/(float)8;
    float ts = 1/(float)8;
    for (int yi = 0; yi < 8; yi++) {
        float y = 2*yi/(float)8 - 1;
        float ty = yi/(float)8;
        for (int xi = 0; xi < 8; xi++) {
            float x = 2*xi/(float)8 - 1;
            float tx = xi/(float)8;
            
            d.push_back(vec4(x, y,      tx, ty));
            d.push_back(vec4(x+s, y,    tx+ts, ty));
            d.push_back(vec4(x,  y+s,    tx, ty+ts));
            
            d.push_back(vec4(x,  y+s,    tx, ty+ts));
            d.push_back(vec4(x+s, y,    tx+ts, ty));
            d.push_back(vec4(x+s,  y+s,    tx+ts, ty+ts));
        }
    }
    
    glGenVertexArrays(1, &_VAOChunked);
    glGenBuffers(1, &_VBOChunked);
    glBindVertexArray(_VAOChunked);
    glBindBuffer(GL_ARRAY_BUFFER, _VBOChunked);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*d.size(), d.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), NULL);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    glGenTextures(1, &_LUTTexture);
    glBindTexture(GL_TEXTURE_1D, _LUTTexture);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    
    glGenTextures(1, &_depthTexture);
    glBindTexture(GL_TEXTURE_2D, _depthTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    return 0;
}

#define CheckCache(cVar, pVar) \
if (cVar != pVar) { \
_cache.needsUpdate = true; \
cVar = pVar; \
}

int VolumeRenderer::_paintGL(bool fast)
{
    if (fast && _algorithm && _algorithm->IsSlow() && _lastRenderTime > 0.1) {
        return 0;
    }
    
    VolumeParams *vp = (VolumeParams *)GetActiveParams();
    if (_cache.algorithmName != vp->GetAlgorithm()) {
        _cache.algorithmName = vp->GetAlgorithm();
        if (_algorithm) delete _algorithm;
        _algorithm = VolumeAlgorithm::NewAlgorithm(_cache.algorithmName, _glManager);
        _cache.needsUpdate = true;
    }
    
    if (_loadData() < 0) return -1;
    if (_loadSecondaryData() < 0) return -1;
    _loadTF();
    _cache.needsUpdate = false;
    
    GLint viewport[4] = {0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    
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
    
    glBindTexture(GL_TEXTURE_2D, _depthTexture);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,   viewport[0], viewport[1], viewport[2], viewport[3], 0);
    
    SmartShaderProgram shader(_algorithm->GetShader());
    if (!shader.IsValid())
        return -1;
    shader->SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());
    shader->SetUniform("cameraPos", vec3(cameraPos[0], cameraPos[1], cameraPos[2]));
    shader->SetUniform("dataBoundsMin", dataMin);
    shader->SetUniform("dataBoundsMax", dataMax);
    shader->SetUniform("userExtsMin", userMin);
    shader->SetUniform("userExtsMax", userMax);
    shader->SetUniform("LUTMin", (float)_cache.mapRange[0]);
    shader->SetUniform("LUTMax", (float)_cache.mapRange[1]);
    shader->SetUniform("unitDistance", smallestDimension/100.f);
    shader->SetUniform("scales", extScales);
    shader->SetUniform("fast", fast);
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
    if (_cache.constantColor.size() == 4)
        shader->SetUniform("constantColor", *(vec4*)_cache.constantColor.data());
    
    _algorithm->SetUniforms();
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, _LUTTexture);
    shader->SetUniform("LUT", 1);
    
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, _depthTexture);
    shader->SetUniform("sceneDepth", 7);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    
    void *start = GLManager::BeginTimer();
    if (_algorithm->IsSlow()) {
        glBindVertexArray(_VAOChunked);
        for (int i = 0; i < 8*8; i++) {
            glDrawArrays(GL_TRIANGLES, i*6, 6);
            glFinish();
        }
    } else {
        glBindVertexArray(_VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    _lastRenderTime = GLManager::EndTimer(start);
    printf("Render time = %f\n", _lastRenderTime);
    
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    GL_ERR_BREAK();
    
    glBindVertexArray(0);
    
    return 0;
}

int VolumeRenderer::_loadData()
{
    VolumeParams *RP = (VolumeParams *)GetActiveParams();
    CheckCache(_cache.var, RP->GetVariableName());
    CheckCache(_cache.ts, RP->GetCurrentTimestep());
    CheckCache(_cache.refinement, RP->GetRefinementLevel());
    CheckCache(_cache.compression, RP->GetCompressionLevel());
    if (!_cache.needsUpdate)
        return 0;
    
    Grid *grid = _dataMgr->GetVariable(_cache.ts, _cache.var, _cache.refinement, _cache.compression);
    
    if (_needToSetDefaultAlgorithm()) {
        if (_algorithm) delete _algorithm;
        string algorithmName = _getDefaultAlgorithmForGrid(grid);
        _algorithm = VolumeAlgorithm::NewAlgorithm(algorithmName, _glManager);
        RP->SetAlgorithm(algorithmName);
    }
    
    int ret = _algorithm->LoadData(grid);
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
    CheckCache(_cache.useColorMapVar, _usingColorMapData());
    CheckCache(_cache.colorMapVar, vp->GetColorMapVariableName());
    if (!_cache.needsUpdate)
        return 0;
    
    if (_cache.useColorMapVar) {
        Grid *grid = _dataMgr->GetVariable(_cache.ts, _cache.colorMapVar, _cache.refinement, _cache.compression);
        int ret = _algorithm->LoadSecondaryData(grid);
        delete grid;
        return ret;
    } else {
        _algorithm->DeleteSecondaryData();
        return 0;
    }
}

void VolumeRenderer::_loadTF()
{
    VolumeParams *vp = (VolumeParams *)GetActiveParams();
    MapperFunction *tf;
    if (_cache.useColorMapVar) {
        tf = vp->GetMapperFunc(_cache.colorMapVar);
    } else {
        tf = vp->GetMapperFunc(_cache.var);
        vector<float> constantColor = vp->GetConstantColor();
        constantColor.push_back(tf->getOpacityScale());
        CheckCache(_cache.constantColor, constantColor);
    }
    
    if (_cache.tf && *_cache.tf != *tf)
        _cache.needsUpdate = true;
    
    if (!_cache.needsUpdate)
        return;
    
    if (_cache.tf) delete _cache.tf;
    _cache.tf = new MapperFunction(*tf);
    _cache.mapRange = tf->getMinMaxMapValue();
    
    float *LUT = new float[4 * 256];
    tf->makeLut(LUT);
    
    glBindTexture(GL_TEXTURE_1D, _LUTTexture);
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
    _dataMgr->GetVariableExtents(_cache.ts, _cache.var, _cache.refinement, dMinExts, dMaxExts);
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
