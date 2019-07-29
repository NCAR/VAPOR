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
using glm::ivec2;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

#define CheckCache(cVar, pVar) \
if (cVar != pVar) { \
_cache.needsUpdate = true; \
cVar = pVar; \
}

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
    _lastRenderTime = 10000;
    _lastRenderWasFast = false;
    _framebufferRatio = 1;
    _previousFramebufferRatio = 1;
    
    if (_needToSetDefaultAlgorithm()) {
        VolumeParams *vp = (VolumeParams*)GetActiveParams();
        Grid *grid = _dataMgr->GetVariable(vp->GetCurrentTimestep(), vp->GetVariableName(), vp->GetRefinementLevel(), vp->GetCompressionLevel());
        if (grid) {
            string algorithmName = _getDefaultAlgorithmForGrid(grid);
            vp->SetAlgorithm(algorithmName);
            delete grid;
        } else {
            vp->SetAlgorithm(VolumeRegular::GetName());
        }
    }
}

VolumeRenderer::~VolumeRenderer()
{
    if (_VAO) glDeleteVertexArrays(1, &_VAO);
    if (_VBO) glDeleteBuffers(1, &_VBO);
    if (_VAOChunked) glDeleteVertexArrays(1, &_VAOChunked);
    if (_VBOChunked) glDeleteBuffers(1, &_VBOChunked);
    if (_cache.tf) delete _cache.tf;
    if (_algorithm) delete _algorithm;
}

int VolumeRenderer::_initializeGL()
{
    const float BL = -1;
    const float data[] = {
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
    
    glGenVertexArrays(1, &_VAOChunked);
    glGenBuffers(1, &_VBOChunked);
    glBindVertexArray(_VAOChunked);
    glBindBuffer(GL_ARRAY_BUFFER, _VBOChunked);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), NULL);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    _framebufferSize[0] = -1;
    _framebufferSize[1] = -1;
    
    _framebuffer.EnableDepthBuffer();
    _framebuffer.Generate();
    _LUTTexture.Generate();
    _depthTexture.Generate();
    
    return 0;
}

int VolumeRenderer::_paintGL(bool fast)
{
    if (fast && _wasTooSlowForFastRender())
        return 0;
    
    CheckCache(_cache.useOSPRay, GetActiveParams()->GetValueLong(RenderParams::OSPRayEnabledTag, false));
    if (_initializeAlgorithm() < 0) return -1;
    if (_loadData() < 0) return -1;
    if (_loadSecondaryData() < 0) return -1;
    _loadTF();
    _cache.needsUpdate = false;
    
    _depthTexture.CopyDepthBuffer();
    _initializeFramebuffer(fast);
    
    ShaderProgram *shader = _algorithm->GetShader();
    if (!shader) return -1;
    shader->Bind();
    _setShaderUniforms(shader, fast);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    
    void *start = GLManager::BeginTimer();
    if (_shouldUseChunkedRender(fast))
        _drawScreenQuadChuncked();
    else
        _drawScreenQuad();
    double renderTime = GLManager::EndTimer(start);
    _lastRenderTime = renderTime;
    _lastRenderWasFast = fast;
    
    int ret = _renderFramebufferToDisplay();
    
    glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);
    glBindVertexArray(0);
    ShaderProgram::UnBind();
    
    return ret;
}

void VolumeRenderer::_setShaderUniforms(const ShaderProgram *shader, const bool fast) const
{
    VolumeParams *vp = (VolumeParams *)GetActiveParams();
    ViewpointParams *viewpointParams = _paramsMgr->GetViewpointParams(_winName);
    Viewpoint *viewpoint = viewpointParams->getCurrentViewpoint();
    double m[16];
    double cameraPos[3], cameraUp[3], cameraDir[3];
    _glManager->matrixManager->GetDoublev(MatrixManager::Mode::ModelView, m);
    viewpoint->ReconstructCamera(m, cameraPos, cameraUp, cameraDir);
    
    shader->SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());
    shader->SetUniform("cameraPos", vec3(cameraPos[0], cameraPos[1], cameraPos[2]));
    shader->SetUniform("samplingRateMultiplier", vp->GetSamplingMultiplier());
    shader->SetUniform("lightingEnabled", vp->GetLightingEnabled());
    shader->SetUniform("phongAmbient",   vp->GetPhongAmbient());
    shader->SetUniform("phongDiffuse",   vp->GetPhongDiffuse());
    shader->SetUniform("phongSpecular",  vp->GetPhongSpecular());
    shader->SetUniform("phongShininess", vp->GetPhongShininess());
    
    glm::vec3 dataMin, dataMax, userMin, userMax;
    _getExtents(&dataMin, &dataMax, &userMin, &userMax);
    vec3 extLengths = dataMax - dataMin;
    vec3 extScales = _getVolumeScales();
    vec3 extLengthsScaled = extLengths * extScales;
    float smallestDimension = min(extLengthsScaled[0], min(extLengthsScaled[1], extLengthsScaled[2]));
    float largestDimension = max(extLengthsScaled[0], max(extLengthsScaled[1], extLengthsScaled[2]));
    
    shader->SetUniform("dataBoundsMin", dataMin);
    shader->SetUniform("dataBoundsMax", dataMax);
    shader->SetUniform("userExtsMin", userMin);
    shader->SetUniform("userExtsMax", userMax);
    shader->SetUniform("unitDistance", largestDimension/100.f);
    shader->SetUniform("unitOpacityScalar", largestDimension/smallestDimension);
    shader->SetUniform("scales", extScales);
    
    shader->SetUniform("density", (float)_cache.tf->getOpacityScale());
    shader->SetUniform("LUTMin", (float)_cache.mapRange[0]);
    shader->SetUniform("LUTMax", (float)_cache.mapRange[1]);
    shader->SetUniform("mapOrthoMode", viewpointParams->GetProjectionType() == ViewpointParams::MapOrthographic);
    
    shader->SetSampler("LUT", _LUTTexture);
    shader->SetSampler("sceneDepth", _depthTexture);
    
    shader->SetUniform("fast", fast);
    
    _algorithm->SetUniforms(shader);
}

void VolumeRenderer::_drawScreenQuad()
{
    glBindVertexArray(_VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void VolumeRenderer::_drawScreenQuadChuncked()
{
    // This constant is not correctly parenthesized because of precision issues
#define CHUNKS_PER_DIM_CONSTANT 64/(1000*1000)
    
    int width = _originalViewport[2] - _originalViewport[0];
    int height = _originalViewport[3] - _originalViewport[1];
    float nPixels = width * height;
    
    double chunksPerDim = sqrt(nPixels * CHUNKS_PER_DIM_CONSTANT);
    
    int framebufferChunksPerDim = ceil(chunksPerDim/_framebufferRatio);
    _generateChunkedRenderMesh(framebufferChunksPerDim);
    
    glBindVertexArray(_VAOChunked);
    for (int i = 0; i < _nChunks; i++) {
        glDrawArrays(GL_TRIANGLES, i*6, 6);
        glFinish();
    }
}

void VolumeRenderer::_generateChunkedRenderMesh(const float C)
{
    vector<vec4> d;
    _nChunks = powf(ceil(C),2);
    d.reserve(powf(ceil(C),2)*6);
    float s = 2/(float)C;
    float ts = 1/(float)C;
    for (int yi = 0; yi < C; yi++) {
        float y = 2*yi/(float)C - 1;
        float ty = yi/(float)C;
        float y2 = min(y+s, 1.0f);
        float ty2 = min(ty+ts, 1.0f);
        for (int xi = 0; xi < C; xi++) {
            float x = 2*xi/(float)C - 1;
            float tx = xi/(float)C;
            float x2 = min(x+s, 1.0f);
            float tx2 = min(tx+ts, 1.0f);
            
            d.push_back(vec4(x, y,      tx, ty));
            d.push_back(vec4(x2, y,    tx2, ty));
            d.push_back(vec4(x,  y2,    tx, ty2));
            
            d.push_back(vec4(x,  y2,    tx, ty2));
            d.push_back(vec4(x2, y,    tx2, ty));
            d.push_back(vec4(x2,  y2,    tx2, ty2));
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, _VBOChunked);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*d.size(), d.data(), GL_STATIC_DRAW);
}

#define MAX_FRAMEBUFFER_RATIO 15.0f

bool VolumeRenderer::_wasTooSlowForFastRender() const
{
    float prevFPS = 1/_lastRenderTime;
    
    if (_lastRenderWasFast && prevFPS < 10 && _framebufferRatio == MAX_FRAMEBUFFER_RATIO)
        return true;
    return false;
}

void VolumeRenderer::_computeNewFramebufferRatio()
{
    float prevFPS = 1/_lastRenderTime;
    if (!_lastRenderWasFast)
        prevFPS *= _algorithm->GuestimateFastModeSpeedupFactor();
    
    if (prevFPS < 24 || (prevFPS > 40 && _framebufferRatio > 3) || prevFPS > 60) {
        float ratioTo30FPS = 30/prevFPS;
        float perDimRatio = sqrtf(ratioTo30FPS);
        _framebufferRatio *= perDimRatio;
        _framebufferRatio = min(_framebufferRatio, MAX_FRAMEBUFFER_RATIO);
        _framebufferRatio = max(_framebufferRatio, 1.0f);
    }
}

bool VolumeRenderer::_shouldUseChunkedRender(bool fast) const
{
    if (_algorithm) {
        if (_algorithm->RequiresChunkedRendering())
            return true;
        
        float estimatedTime = _lastRenderTime;
        if (_lastRenderWasFast && !fast) {
            estimatedTime *= powf(_previousFramebufferRatio, 2);
            estimatedTime *= _algorithm->GuestimateFastModeSpeedupFactor();
        }
        
        if (estimatedTime > 1.0f)
            return true;
    }
    return false;
}

bool VolumeRenderer::_usingColorMapData() const
{
    // Overriden by VolumeIsoRenderer
    return false;
}

void VolumeRenderer::_saveOriginalViewport()
{
    glGetIntegerv(GL_VIEWPORT, _originalViewport);
}

void VolumeRenderer::_restoreOriginalViewport()
{
    glViewport(_originalViewport[0], _originalViewport[1], _originalViewport[2], _originalViewport[3]);
}

void VolumeRenderer::_initializeFramebuffer(bool fast)
{
    _previousFramebufferRatio = _framebufferRatio;
    
    if (fast) _computeNewFramebufferRatio();
    else _framebufferRatio = 1;
    
    _saveOriginalViewport();
    ivec2 fbSize(_originalViewport[2], _originalViewport[3]);
    fbSize /= _framebufferRatio;
    _framebuffer.SetSize(fbSize.x, fbSize.y);
    
    _framebuffer.MakeRenderTarget();
    glClearColor(0, 0, 0, 0);
    glDepthMask(true);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int VolumeRenderer::_renderFramebufferToDisplay()
{
    _framebuffer.UnBind();
    _restoreOriginalViewport();
    SmartShaderProgram framebufferShader = _glManager->shaderManager->GetShader("Framebuffer");
    if (!framebufferShader.IsValid())
        return -1;
    framebufferShader->SetSampler("colorBuffer", *_framebuffer.GetColorTexture());
    framebufferShader->SetSampler("depthBuffer", *_framebuffer.GetDepthTexture());
    _drawScreenQuad();
    
    return 0;
}

int VolumeRenderer::_initializeAlgorithm()
{
    VolumeParams *vp = (VolumeParams *)GetActiveParams();
    
    if (_cache.algorithmName != vp->GetAlgorithm()) {
        _cache.algorithmName = vp->GetAlgorithm();
        if (_algorithm) delete _algorithm;
        _algorithm = VolumeAlgorithm::NewAlgorithm(_cache.algorithmName, _glManager);
        _cache.needsUpdate = true;
    }
    if (_algorithm) return 0;
    else return -1;
}

bool VolumeRenderer::_needToLoadData()
{
    VolumeParams *RP = (VolumeParams *)GetActiveParams();
    CheckCache(_cache.var, RP->GetVariableName());
    CheckCache(_cache.ts, RP->GetCurrentTimestep());
    CheckCache(_cache.refinement, RP->GetRefinementLevel());
    CheckCache(_cache.compression, RP->GetCompressionLevel());
    return _cache.needsUpdate;
}

int VolumeRenderer::_loadData()
{
    VolumeParams *RP = (VolumeParams *)GetActiveParams();
    if (!_needToLoadData())
        return 0;
    
    Grid *grid = _dataMgr->GetVariable(_cache.ts, _cache.var, _cache.refinement, _cache.compression);
    if (!grid)
        return -1;
    
    if (dynamic_cast<const UnstructuredGrid *>(grid)) {
        MyBase::SetErrMsg("Unstructured grids are not supported by this renderer");
        return -1;
    }
    
    if (_needToSetDefaultAlgorithm()) {
        RP->SetAlgorithm(_getDefaultAlgorithmForGrid(grid));
        if (_initializeAlgorithm() < 0) {
            delete grid;
            return -1;
        }
    }
    
    int ret = _algorithm->LoadData(grid);
    _lastRenderTime = 10000;
    delete grid;
    return ret;
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
        if (!grid)
            return -1;
        int ret = _algorithm->LoadSecondaryData(grid);
        delete grid;
        return ret;
    } else {
        _algorithm->DeleteSecondaryData();
        return 0;
    }
}

void VolumeRenderer::_getLUTFromTF(const MapperFunction *tf, float *LUT) const
{
    // Constant opacity needs to be removed here and applied in the shader
    // because otherwise we run out of precision in the LUT
    MapperFunction tfSansConstantOpacity(*tf);
    tfSansConstantOpacity.setOpacityScale(1);
    
    tfSansConstantOpacity.makeLut(LUT);
    for (int i = 0; i < 256; i++) {
        LUT[4*i+3] = powf(LUT[4*i+3], 2);
    }
}

MapperFunction *VolumeRenderer::_needToLoadTF()
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
    
    if (_cache.needsUpdate)
        return tf;
    else
        return nullptr;
}

void VolumeRenderer::_loadTF()
{
    MapperFunction *tf = _needToLoadTF();
    if (!tf)
        return;
    
    if (_cache.tf) delete _cache.tf;
    _cache.tf = new MapperFunction(*tf);
    _cache.mapRange = tf->getMinMaxMapValue();
    
    float *LUT = new float[4 * 256];
    _getLUTFromTF(tf, LUT);
    
    _LUTTexture.TexImage(GL_RGBA8, 256, 0, 0, GL_RGBA, GL_FLOAT, LUT);
    
    delete [] LUT;
}

glm::vec3 VolumeRenderer::_getVolumeScales() const
{
    ViewpointParams *vpp = _paramsMgr->GetViewpointParams(_winName);
    Transform *datasetTransform = vpp->GetTransform(GetMyDatasetName());
    Transform *rendererTransform = GetActiveParams()->GetTransform();
    VAssert(datasetTransform && rendererTransform);
 
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
    if (userMin) *userMin = vec3(dMinExts[0], dMinExts[1], dMinExts[2]);
    if (userMax) *userMax = vec3(dMaxExts[0], dMaxExts[1], dMaxExts[2]);
    _dataMgr->GetVariableExtents(_cache.ts, _cache.var, _cache.refinement, dMinExts, dMaxExts);
    if (dataMin) *dataMin = vec3(dMinExts[0], dMinExts[1], dMinExts[2]);
    if (dataMax) *dataMax = vec3(dMaxExts[0], dMaxExts[1], dMaxExts[2]);
    
    // Moving domain allows area outside of data to be selected
    if (userMin) *userMin = glm::max(*userMin, *dataMin);
    if (userMax) *userMax = glm::min(*userMax, *dataMax);
}

std::string VolumeRenderer::_getDefaultAlgorithmForGrid(const Grid *grid) const
{
    if (GLManager::GetVendor() == GLManager::Vendor::Intel)
        return VolumeRegular::GetName();
    
    if (dynamic_cast<const RegularGrid *>   (grid)) return VolumeRegular      ::GetName();
    if (dynamic_cast<const StructuredGrid *>(grid)) return VolumeCellTraversal::GetName();
    MyBase::SetErrMsg("Unsupported grid type: %s", grid->GetType().c_str());
    return "";
}

bool VolumeRenderer::_needToSetDefaultAlgorithm() const
{
    return !((VolumeParams*)GetActiveParams())->GetAlgorithmWasManuallySetByUser();
}

int VolumeRenderer::OSPRayUpdate(OSPModel world)
{
    VolumeParams *vp = (VolumeParams*)GetActiveParams();
    CheckCache(_cache.useOSPRay, vp->GetValueLong(RenderParams::OSPRayEnabledTag, false));
    
    _initializeAlgorithm();
    if (OSPRayLoadData(world) < 0) return -1;
    if (OSPRayLoadTF() < 0) return -1;
    
    ospSet1b(_volume, "singleShade", false);
    ospSet1b(_volume, "gradientShadingEnabled", vp->GetValueLong(VolumeParams::OSPRayLightingEnabledTag, true));
    float samplingRate = vp->GetValueDouble(VolumeParams::OSPRaySamplingRateTag, 1);
    ospSet1f(_volume, "samplingRate", samplingRate);
    ospSet1f(_volume, "adaptiveMaxSamplingRate", samplingRate*OSPRAY_ADAPTIVE_SAMPLING_MULTIPLIER);
    float specular = vp->GetValueDouble(VolumeParams::OSPRaySpecularTag, 0);
    ospSet3f(_volume, "specular", specular, specular, specular);
    
    glm::vec3 dataMin, dataMax, userMin, userMax;
    _getExtents(&dataMin, &dataMax, &userMin, &userMax);
    vec3 ospMin = _ospCache.coordTransform * vec4(userMin, 1.f);
    vec3 ospMax = _ospCache.coordTransform * vec4(userMax, 1.f);
    ospSet3fv(_volume, "volumeClippingBoxUpper", (float*)&ospMax);
    ospSet3fv(_volume, "volumeClippingBoxLower", (float*)&ospMin);
    
//    if (_cache.needsUpdate)
        ospCommit(_volume);
    
    _cache.needsUpdate = false;
    return 0;
}

void VolumeRenderer::OSPRayDelete(OSPModel world)
{
    ospRemoveVolume(world, _volume);
    ospRemoveGeometry(world, sphere);
    
    ospRelease(sphere); sphere = nullptr;
    ospRelease(_volume); _volume = nullptr;
    ospRelease(_tf); _tf = nullptr;
}

bool VolumeRenderer::OSPRayNeedToLoadData()
{
    CheckCache(_ospCache.coordTransform, _getModelMatrix());
    if (!_volume)
        _cache.needsUpdate = true;
    
    return _needToLoadData();
}

int VolumeRenderer::OSPRayLoadData(OSPModel world)
{
    if (!OSPRayNeedToLoadData())
        return 0;
    
    OSPRayDelete(world);
    
    Grid *grid = _dataMgr->GetVariable(_cache.ts, _cache.var, _cache.refinement, _cache.compression);
    
    if (dynamic_cast<UnstructuredGrid*>(grid))
        return OSPRayLoadDataUnstructured(world, grid);
    else if (_cache.algorithmName == VolumeRegular::GetName())
        return OSPRayLoadDataRegular(world, grid);
    else
        return OSPRayLoadDataStructured(world, grid);
    
    delete grid;
    
    return 1;
}

int VolumeRenderer::OSPRayLoadDataRegular(OSPModel world, Grid *grid)
{
    glm::vec3 dataMin, dataMax, userMin, userMax;
    _getExtents(&dataMin, &dataMax, &userMin, &userMax);
    
    if (!_volume) {
        _volume = ospNewVolume("block_bricked_volume");
        ospAddVolume(world, _volume);
        
        sphere = ospNewGeometry("spheres");
        float sphereData[4] = {dataMin.x,dataMin.y,dataMin.z,0};
        OSPData spheres = ospNewData(4, OSP_FLOAT, sphereData);
        ospCommit(spheres);
        ospSetData(sphere, "spheres", spheres);
        //        ospSetf(sphere, "radius", 200000);
        ospSetf(sphere, "radius", 0.1);
        ospCommit(sphere);
        ospAddGeometry(world, sphere);
    }
    
    const vector<size_t> dims = grid->GetDimensions();
    const size_t nVerts = dims[0]*dims[1]*dims[2];
    float *volumeData = new float[nVerts];
    
    auto dataIt = grid->cbegin();
    for (size_t i = 0; i < nVerts; ++i, ++dataIt)
        volumeData[i] = *dataIt;
    
    ospSet3i(_volume, "dimensions", dims[0], dims[1], dims[2]);
    ospSetString(_volume, "voxelType", "float");
    
    ospSetRegion(_volume, volumeData, {0,0,0}, {static_cast<int>(dims[0]), static_cast<int>(dims[1]), static_cast<int>(dims[2])});
    delete [] volumeData;
    
    vec3 gridDims(dims[0], dims[1], dims[2]);
    vec3 gridSpacing = (dataMax-dataMin)/(gridDims-vec3(1));
    
    vec3 origin = _getOrigin();
    vec3 scales = _getTotalScaling();
    mat4 model = _ospCache.coordTransform;
    
    vec3 pos = (dataMin-origin)*scales + origin;
    gridSpacing *= scales;
    
    pos = model * vec4(dataMin, 1.0f);
    
    ospSet3fv(_volume, "gridOrigin", glm::value_ptr(pos));
    ospSet3fv(_volume, "gridSpacing", glm::value_ptr(gridSpacing));
    
    return 0;
}

int VolumeRenderer::OSPRayLoadDataStructured(OSPModel world, Grid *grid)
{
    _volume = ospNewVolume("unstructured_volume");
    ospAddVolume(world, _volume);
    
    const vector<size_t> dims = grid->GetDimensions();
    const int VW = dims[0], VH = dims[1], VD = dims[2];
    const size_t nVerts = dims[0]*dims[1]*dims[2];
    float *scalarData = new float[nVerts];
    float *coordData = new float[nVerts*3];
    
    auto dataIt = grid->cbegin();
    for (size_t i = 0; i < nVerts; ++i, ++dataIt)
        scalarData[i] = *dataIt;
    
    OSPData ospData = ospNewData(nVerts, OSP_FLOAT, scalarData);
    ospCommit(ospData);
    ospSetData(_volume, "field", ospData);
    ospRelease(ospData);
    
    
    vec3 *coords = (vec3*)coordData;
    mat4 model = _ospCache.coordTransform;
    auto coord = grid->ConstCoordBegin();
    for (size_t i = 0; i < nVerts; ++i, ++coord) {
        coordData[i*3  ] = (*coord)[0];
        coordData[i*3+1] = (*coord)[1];
        coordData[i*3+2] = (*coord)[2];
        
//        coords[i] = (coords[i]-origin)*scales + origin;
        coords[i] = model * vec4(coords[i], 1.0f);
    }
    ospData = ospNewData(nVerts, OSP_FLOAT3, coordData);
    ospCommit(ospData);
    ospSetData(_volume, "vertices", ospData);
    ospRelease(ospData);
    delete [] coordData;
    
//    mat4 m = glm::mat4(1.0);
//    m = glm::translate(m, origin);
//    m = glm::scale(m, scales);
//    m = glm::translate(m, -origin);
//    _ospCoordTransform = m;
    
    typedef struct {
        int i0, i1, i2, i3, i4, i5, i6, i7;
    } Cell;
    Cell *indices = new Cell[nVerts];
#define I(x,y,z) (int)((z)*VH*VW+(y)*VW+(x))
    
    int VCD = VD-1;
    int VCH = VH-1;
    int VCW = VW-1;
    
    bool hasMissing = grid->HasMissingData();
    float missingValue = grid->GetMissingValue();
    size_t indexId = 0;
    
    for (int z = 0; z < VCD; z++) {
        for (int y = 0; y < VCH; y++) {
            for (int x = 0; x < VCW; x++) {
                if (hasMissing) {
                    if (scalarData[I(x  , y  , z  )] == missingValue ||
                        scalarData[I(x+1, y  , z  )] == missingValue ||
                        scalarData[I(x+1, y+1, z  )] == missingValue ||
                        scalarData[I(x  , y+1, z  )] == missingValue ||
                        scalarData[I(x  , y  , z+1)] == missingValue ||
                        scalarData[I(x+1, y  , z+1)] == missingValue ||
                        scalarData[I(x+1, y+1, z+1)] == missingValue ||
                        scalarData[I(x  , y+1, z+1)] == missingValue
                        ) {
                        continue;
                    }
                }
                indices[indexId++] = {
                    I(x  ,y  ,z  ), I(x+1,y  ,z  ), I(x+1,y+1,z  ), I(x  ,y+1,z  ),
                    I(x  ,y  ,z+1), I(x+1,y  ,z+1), I(x+1,y+1,z+1), I(x  ,y+1,z+1),
                };
            }
        }
    }
    
    ospData = ospNewData(indexId*2, OSP_INT4, indices);
    ospCommit(ospData);
    ospSetData(_volume, "indices", ospData);
    ospRelease(ospData);
    delete [] indices;
    delete [] scalarData;
    
    
    return 0;
}

int VolumeRenderer::OSPRayLoadDataUnstructured(OSPModel world, Grid *grid)
{
    return -1;
}

int VolumeRenderer::OSPRayLoadTF()
{
    MapperFunction *tf = _needToLoadTF();
    if (!tf)
        return 0;
    
    if (_cache.tf) delete _cache.tf;
    _cache.tf = new MapperFunction(*tf);
    _cache.mapRange = tf->getMinMaxMapValue();
    
    float *LUT = new float[4 * 256];
    tf->makeLut(LUT);
    
    if (!_tf) {
        _tf = ospNewTransferFunction("piecewise_linear");
        ospSetObject(_volume, "transferFunction", _tf);
    }
    
    float colors[3*256];
    float opacities[256];
    for (int i = 0; i < 256; i++) {
        colors[i*3]   = LUT[i*4];
        colors[i*3+1] = LUT[i*4+1];
        colors[i*3+2] = LUT[i*4+2];
        opacities[i]  = LUT[i*4+3];
    }
    delete [] LUT;
    
    OSPData colorData = ospNewData(256, OSP_FLOAT3, colors);
    OSPData opacityData = ospNewData(256, OSP_FLOAT, opacities);
    ospCommit(colorData);
    ospCommit(opacityData);
    ospSetData(_tf, "colors", colorData);
    ospSetData(_tf, "opacities", opacityData);
    ospRelease(colorData);
    ospRelease(opacityData);
    
    osp::vec2f valueRange = {(float)_cache.mapRange[0], (float)_cache.mapRange[1]};
    ospSetVec2f(_tf, "valueRange", valueRange);
    
    ospCommit(_tf);
    
    return 0;
}

glm::vec3 VolumeRenderer::_getTotalScaling() const
{
    Transform *datasetTransform = _paramsMgr->GetViewpointParams(_winName)->GetTransform(_dataSetName);
    vector<double> datasetScaleD = datasetTransform->GetScales();
    vector<double> scaleD        = GetActiveParams()->GetTransform()->GetScales();
    vec3 datasetScale(datasetScaleD[0], datasetScaleD[1], datasetScaleD[2]);
    vec3 scale(scaleD[0], scaleD[1], scaleD[2]);
    
    return datasetScale * scale;
}

glm::vec3 VolumeRenderer::_getOrigin() const
{
    vector<double> originD = GetActiveParams()->GetTransform()->GetOrigin();
    return vec3(originD[0], originD[1], originD[2]);
}
