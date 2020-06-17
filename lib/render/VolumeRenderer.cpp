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
        vector <double> minExt, maxExt;
        vp->GetBox()->GetExtents(minExt, maxExt);

        Grid *grid = _dataMgr->GetVariable(vp->GetCurrentTimestep(), vp->GetVariableName(), vp->GetRefinementLevel(), vp->GetCompressionLevel(), minExt, maxExt);
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
    _ospDelete();
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
    _LUT2Texture.Generate();
    _depthTexture.Generate();
    
    return _ospInit();
}

int VolumeRenderer::_paintGL(bool fast)
{
    if (fast && _wasTooSlowForFastRender())
        return 0;
    
    CheckCache(_cache.ospEnabled, _ospEnabled());
    CheckCache(_cache.ospMaxCells, GetActiveParams()->GetValueLong("osp_max_cells", 1));
    CheckCache(_cache.ospTestCellId, GetActiveParams()->GetValueLong("osp_test_cells", 1));
    CheckCache(_cache.osp_force_regular, GetActiveParams()->GetValueLong("osp_force_regular", 0));
    CheckCache(_cache.osp_test_volume, GetActiveParams()->GetValueLong("osp_test_volume", 0));
    
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
    
    if (_cache.ospEnabled) {
        _ospRender(fast);
    } else {
        if (_shouldUseChunkedRender(fast))
            _drawScreenQuadChuncked();
        else
            _drawScreenQuad();
    }
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

std::string VolumeRenderer::_getColorbarVariableName() const
{
    VolumeParams *vp = dynamic_cast<VolumeParams *>(GetActiveParams());
    if (vp->GetValueLong(VolumeParams::UseColormapVariableTag, 0))
        return vp->GetColorMapVariableName();
    else
        return vp->GetVariableName();
}

void VolumeRenderer::_setShaderUniforms(const ShaderProgram *shader, const bool fast) const
{
    VolumeParams *vp = dynamic_cast<VolumeParams *>(GetActiveParams());
    ViewpointParams *viewpointParams = _paramsMgr->GetViewpointParams(_winName);
    Viewpoint *viewpoint = viewpointParams->getCurrentViewpoint();
    double m[16];
    double cameraPos[3], cameraUp[3], cameraDir[3];
    _glManager->matrixManager->GetDoublev(MatrixManager::Mode::ModelView, m);
    viewpoint->ReconstructCamera(m, cameraPos, cameraUp, cameraDir);
    
    shader->SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());
    shader->SetUniform("cameraPos", vec3(cameraPos[0], cameraPos[1], cameraPos[2]));
    shader->SetUniform("samplingRateMultiplier", (float)vp->GetSamplingMultiplier());
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
    
    float density = vp->GetValueDouble(VolumeParams::VolumeDensityTag, 1);
    density = powf(density, 4);
    
    shader->SetUniform("density", (float)density);
    shader->SetUniform("LUTMin", (float)_cache.tf->getMinMapValue());
    shader->SetUniform("LUTMax", (float)_cache.tf->getMaxMapValue());
    shader->SetUniform("mapOrthoMode", viewpointParams->GetProjectionType() == ViewpointParams::MapOrthographic);
    
    shader->SetSampler("LUT", _LUTTexture);
    shader->SetSampler("sceneDepth", _depthTexture);
    
    if (_cache.useColorMapVar) {
        shader->SetUniform("LUTMin2", (float)_cache.tf2->getMinMapValue());
        shader->SetUniform("LUTMax2", (float)_cache.tf2->getMaxMapValue());
        shader->SetSampler("LUT2", _LUT2Texture);
    }
    
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
    return GetActiveParams()->GetValueLong(VAPoR::VolumeParams::UseColormapVariableTag, false);
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
    
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &_originalFramebuffer);
    _framebuffer.MakeRenderTarget();
    glClearColor(0, 0, 0, 0);
    glDepthMask(true);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int VolumeRenderer::_renderFramebufferToDisplay()
{
    _framebuffer.UnBind();
    glBindFramebuffer(GL_FRAMEBUFFER, _originalFramebuffer);
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
    
    if (_cache.ospEnabled) {
        if (_cache.algorithmName != "NULL") {
            if (_algorithm) delete _algorithm;
            _algorithm = new VolumeAlgorithmNull(_glManager);
            _cache.algorithmName = "NULL";
        }
        return 0;
    }
    
    if (_cache.algorithmName != vp->GetAlgorithm()) {
        _cache.algorithmName = vp->GetAlgorithm();
        if (_cache.algorithmName == "")
            _cache.algorithmName = "NULL";
        if (_algorithm) delete _algorithm;
        _algorithm = VolumeAlgorithm::NewAlgorithm(_cache.algorithmName, _glManager);
        _cache.needsUpdate = true;
    }
    if (_algorithm) return 0;
    else return -1;
}

int VolumeRenderer::_loadData()
{
    VolumeParams *RP = (VolumeParams *)GetActiveParams();
    vector <double> minExt, maxExt;
    RP->GetBox()->GetExtents(minExt, maxExt);

    CheckCache(_cache.var, RP->GetVariableName());
    CheckCache(_cache.ts, RP->GetCurrentTimestep());
    CheckCache(_cache.refinement, RP->GetRefinementLevel());
    CheckCache(_cache.compression, RP->GetCompressionLevel());
    CheckCache(_cache.minExt, minExt);
    CheckCache(_cache.maxExt, maxExt);
    if (!_cache.needsUpdate)
        return 0;
    
    Grid *grid = _dataMgr->GetVariable(_cache.ts, _cache.var, _cache.refinement, _cache.compression, _cache.minExt, _cache.maxExt);
    if (!grid)
        return -1;
    
    if (dynamic_cast<const UnstructuredGrid *>(grid) && !_cache.ospEnabled) {
        MyBase::SetErrMsg("Unstructured grids are not supported by the GPU renderer");
        return -1;
    }

	// Actual min and max extents of returned grid, which are in general 
	// larger than requested extents.
	//
	grid->GetUserExtents(_dataMinExt, _dataMaxExt);
    
    if (_needToSetDefaultAlgorithm() && !_cache.ospEnabled) {
        RP->SetAlgorithm(_getDefaultAlgorithmForGrid(grid));
        if (_initializeAlgorithm() < 0) {
            delete grid;
            return -1;
        }
    }
    
    int ret;
    if (_ospEnabled()) {
        ret = _ospLoadData(grid);
    } else {
        ret = _algorithm->LoadData(grid);
    }
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
        Grid *grid = _dataMgr->GetVariable(_cache.ts, _cache.colorMapVar, _cache.refinement, _cache.compression, _cache.minExt, _cache.maxExt);
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

void VolumeRenderer::_loadTF()
{
    VolumeParams *vp = (VolumeParams *)GetActiveParams();
    
    vector<float> constantColor = vp->GetConstantColor();
    constantColor.push_back(vp->GetMapperFunc(_cache.var)->getOpacityScale());
    CheckCache(_cache.constantColor, constantColor);
    
    _loadTF(&_LUTTexture, vp->GetMapperFunc(_cache.var), &_cache.tf);
    
    if (_cache.useColorMapVar) {
        _loadTF(&_LUT2Texture, vp->GetMapperFunc(_cache.colorMapVar), &_cache.tf2);
    }
}

void VolumeRenderer::_loadTF(Texture1D *texture, MapperFunction *tf, MapperFunction **cacheTF)
{
    if (!*cacheTF || **cacheTF != *tf)
        _cache.needsUpdate = true;
    
    if (!_cache.needsUpdate)
        return;
    
    if (*cacheTF) delete *cacheTF;
    *cacheTF = new MapperFunction(*tf);
    
    float *LUT = new float[4 * 256];
    _getLUTFromTF(tf, LUT);
    texture->TexImage(GL_RGBA8, 256, 0, 0, GL_RGBA, GL_FLOAT, LUT);
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
    *userMin = vec3(_cache.minExt[0], _cache.minExt[1], _cache.minExt[2]);
    *userMax = vec3(_cache.maxExt[0], _cache.maxExt[1], _cache.maxExt[2]);
    *dataMin = vec3(_dataMinExt[0], _dataMinExt[1], _dataMinExt[2]);
    *dataMax = vec3(_dataMaxExt[0], _dataMaxExt[1], _dataMaxExt[2]);
    
    // Moving domain allows area outside of data to be selected
    *userMin = glm::max(*userMin, *dataMin);
    *userMax = glm::min(*userMax, *dataMax);
}

std::string VolumeRenderer::_getDefaultAlgorithmForGrid(const Grid *grid) const
{
    if (GLManager::GetVendor() == GLManager::Vendor::Intel)
        return VolumeRegular::GetName();
    
    if (dynamic_cast<const RegularGrid *>   (grid)) return VolumeRegular      ::GetName();
    if (dynamic_cast<const StructuredGrid *>(grid)) return VolumeCellTraversal::GetName();
    if (dynamic_cast<const UnstructuredGrid *>(grid)) return VolumeAlgorithmNull::GetName();
    MyBase::SetErrMsg("Unsupported grid type: %s", grid->GetType().c_str());
    return "";
}

bool VolumeRenderer::_needToSetDefaultAlgorithm() const
{
    return !((VolumeParams*)GetActiveParams())->GetAlgorithmWasManuallySetByUser();
}

bool VolumeRenderer::_ospEnabled()
{
    return GetActiveParams()->GetValueLong("osp_enable", false);
}

int VolumeRenderer::_ospInit()
{
    _ospCamera = ospNewCamera("perspective");
    _ospWorld = ospNewWorld();
    
    OSPLight lightAmbient = ospNewLight("ambient");
    ospSetFloat(lightAmbient, "intensity", 0.2);
    ospCommit(lightAmbient);
//    ospSetObjectAsData(_ospWorld, "light", OSP_LIGHT, lightAmbient);
//    ospRelease(lightAmbient);
    
    OSPLight lightDistant = ospNewLight("distant");
    ospSetVec3f(lightDistant, "direction", 0, 0, -1);
    ospSetFloat(lightDistant, "angularDiameter", 1);
    ospSetFloat(lightDistant, "insensity", 4);
    ospCommit(lightDistant);
    
    vector<OSPLight> lights = {lightAmbient, lightDistant};
    OSPData lightsData = VOSP::NewCopiedData(lights.data(), OSP_LIGHT, lights.size());
    ospCommit(lightsData);
    

    // Although these are already pointers, unlike every other function,
    // in the case of this function you need to reference here otherwise
    // it will crash.
    ospSetParam(_ospWorld, "light", OSP_LIGHT, &lightsData);
    
    ospRelease(lightsData);
    ospRelease(lightAmbient);
    _ospLightDistant = lightDistant;
    ospRetain(lightDistant);
    ospRelease(lightDistant);
    
    
//    auto p = GetActiveParams();
//    vector <double> minExt, maxExt;
//    p->GetBox()->GetExtents(minExt, maxExt);
//    Grid *grid = _dataMgr->GetVariable(p->GetCurrentTimestep(), p->GetVariableName(), p->GetRefinementLevel(), p->GetCompressionLevel(), minExt, maxExt);
//    assert(grid);
//    _ospLoadData(grid);
//    delete grid;
    
//    OSPGeometricModel triangleModel = VOSP::Test::LoadTriangle();
//
//    OSPGroup group = ospNewGroup();
//    ospSetObjectAsData(group, "geometry", OSP_GEOMETRIC_MODEL, triangleModel);
//    ospCommit(group);
//    ospRelease(triangleModel);
//
//    _ospInstance = ospNewInstance(group);
//    ospCommit(_ospInstance);
//    ospRelease(group);
//
//    ospSetObjectAsData(_ospWorld, "instance", OSP_INSTANCE, _ospInstance);
//    ospCommit(_ospWorld);
    
    OSPBounds b = ospGetBounds(_ospWorld);
    printf("world bounds: ({%f, %f, %f}, {%f, %f, %f}\n\n", b.lower[0],b.lower[1],b.lower[2],b.upper[0],b.upper[1],b.upper[2]);
    
    _ospRenderTexture.Generate();
    
    return 0;
}

int VolumeRenderer::_ospRender(bool fast)
{
    auto viewport = GLManager::GetViewport();
    ivec2 fbSize(viewport[2], viewport[3]);
    OSPFrameBuffer framebuffer = ospNewFrameBuffer(fbSize.x, fbSize.y, OSP_FB_SRGBA, OSP_FB_COLOR | /*OSP_FB_DEPTH |*/ OSP_FB_ACCUM);
    ospResetAccumulation(framebuffer);
    
    _ospSetupCamera();
    _ospLoadTF();
    _ospApplyTransform();
    _ospSetupRenderer(fast);
    
    ospSetFloat(_ospVolumeModel, "densityScale", GetActiveParams()->GetValueDouble("osp_density", 1));
    ospCommit(_ospVolumeModel);
    
    ospRenderFrameBlocking(framebuffer, _ospRenderer, _ospCamera, _ospWorld);
    
    const uint32_t *fb = (uint32_t *)ospMapFrameBuffer(framebuffer, OSP_FB_COLOR);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowSize.x, windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, fb);
    _ospRenderTexture.TexImage(GL_RGBA, fbSize.x, fbSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, fb);
    ospUnmapFrameBuffer(fb, framebuffer);
    ospRelease(framebuffer);
    
    
    SmartShaderProgram framebufferShader = _glManager->shaderManager->GetShader("FramebufferND");
    if (!framebufferShader.IsValid()) return -1;
    framebufferShader->SetSampler("colorBuffer", _ospRenderTexture);
    _drawScreenQuad();
    
    return 0;
}

void VolumeRenderer::_ospSetupRenderer(bool fast)
{
    bool usePT = GetActiveParams()->GetValueLong("osp_usePT", 0);
    
    if (!_ospRenderer || _cache.ospPT != usePT) {
        ospRelease(_ospRenderer);
        _ospRenderer = ospNewRenderer(usePT ? "pathtracer" : "scivis");
        _cache.ospPT = usePT;
    }
    
    vector<double> bgColor;
    int spp = GetActiveParams()->GetValueLong("osp_spp", 1);
    _paramsMgr->GetAnnotationParams(_winName)->GetBackgroundColor(bgColor);
    ospSetVec3f(_ospRenderer, "backgroundColor", bgColor[0], bgColor[1], bgColor[2]);
    ospSetFloat(_ospRenderer, "volumeSamplingRate", (fast ? 0.1 : 1) * GetActiveParams()->GetValueDouble("osp_volumeSamplingRate", 1));
    ospSetInt(_ospRenderer, "pixelSamples", fast ? 1 : spp);
    ospCommit(_ospRenderer);
}

void VolumeRenderer::_ospSetupCamera()
{
    ViewpointParams* vp = _paramsMgr->GetViewpointParams(_winName);
    auto viewport = GLManager::GetViewport();
    ivec2 fbSize(viewport[2], viewport[3]);
    
    double matrix[16], dpos[3], dup[3], ddir[3];
    vp->GetModelViewMatrix(matrix);
    assert(vp->ReconstructCamera(matrix, dpos, dup, ddir));
    vec3 pos(dpos[0], dpos[1], dpos[2]);
    vec3 up ( dup[0],  dup[1],  dup[2]);
    vec3 dir(ddir[0], ddir[1], ddir[2]);
    
//    printf("Camera Pos = (%f, %f, %f)\n", pos.x, pos.y, pos.z);
//    printf("Camera Up  = (%f, %f, %f)\n", up.x,  up.y,  up.z);
//    printf("Camera Dir = (%f, %f, %f)\n", dir.x, dir.y, dir.z);
    
    ospSetFloat(_ospCamera, "aspect", fbSize.x / (float)fbSize.y);
    ospSetFloat(_ospCamera, "fovy", vp->GetFOV());
    ospSetParam(_ospCamera, "position", OSP_VEC3F, &pos);
    ospSetParam(_ospCamera, "direction", OSP_VEC3F, &dir);
    ospSetParam(_ospCamera, "up", OSP_VEC3F, &up);
    ospCommit(_ospCamera);
    
    float intensity = GetActiveParams()->GetValueDouble("osp_light_intensity", 4);
    float brightness = GetActiveParams()->GetValueDouble("osp_light_brightness", 1);
    ospSetFloat(_ospLightDistant, "insensity", intensity);
    ospSetVec3f(_ospLightDistant, "direction", dir.x, dir.y, dir.z);
    ospSetVec3f(_ospLightDistant, "color", brightness, brightness, brightness);
    ospCommit(_ospLightDistant);
}

void VolumeRenderer::_ospLoadTF()
{
    if (!_ospTF) {
        fprintf(stderr, "Warning: _ospTF = NULL\n");
        return;
    }
    auto p = GetActiveParams();
    auto vtf = p->GetMapperFunc(p->GetVariableName());
    auto range = vtf->getMinMaxMapValue();
    
    float *LUT = new float[4 * 256];
    vtf->makeLut(LUT);
    for (int i = 0; i < 256; i++) {
        LUT[4*i+3] = powf(LUT[4*i+3], 2);
    }
    
    vec3 *cLUT = new vec3[256];
    float *oLUT = new float[256];
    for (int i = 0; i < 256; i++) {
        cLUT[i].r = LUT[4*i+0];
        cLUT[i].g = LUT[4*i+1];
        cLUT[i].b = LUT[4*i+2];
        oLUT[i]   = LUT[4*i+3];
    }
    
    OSPData data = VOSP::NewCopiedData(oLUT, OSP_FLOAT, 256);
    ospCommit(data);
    ospSetObject(_ospTF, "opacity", data);
    ospRelease(data);
    data = VOSP::NewCopiedData(cLUT, OSP_VEC3F, 256);
    ospCommit(data);
    ospSetObject(_ospTF, "color", data);
    ospRelease(data);
    ospSetVec2f(_ospTF, "valueRange", range[0], range[1]);
    ospCommit(_ospTF);
    
    delete [] LUT;
    delete [] cLUT;
    delete [] oLUT;
}

vec3 D2V(const vector<double> &dv)
{
    VAssert(dv.size() >= 3);
    return vec3(dv[0], dv[1], dv[2]);
}

#define PrintVec3(v) printf("%s = (%f, %f, %f)\n", #v, v.x, v.y, v.z)

void VolumeRenderer::_ospApplyTransform()
{
    auto p = GetActiveParams();
    vec3 translate = D2V(p->GetTransform()->GetTranslations());
    vec3 rotate    = D2V(p->GetTransform()->GetRotations());
    vec3 scale     = D2V(p->GetTransform()->GetScales());
    vec3 origin    = D2V(p->GetTransform()->GetOrigin());
    Transform *datasetTransform = _paramsMgr->GetViewpointParams(_winName)->GetTransform(_dataSetName);
    vec3 datasetScales = D2V(datasetTransform->GetScales());
    vec3 datasetRotation = D2V(datasetTransform->GetRotations());
    vec3 datasetTranslation = D2V(datasetTransform->GetTranslations());
    vec3 datasetOrigin = D2V(datasetTransform->GetOrigin());

    mat4 m(1.f);

    m = glm::translate(m, datasetTranslation);
    m = glm::translate(m, datasetOrigin);
    m = glm::rotate(m, glm::radians(datasetRotation.x), vec3(1,0,0));
    m = glm::rotate(m, glm::radians(datasetRotation.y), vec3(0,1,0));
    m = glm::rotate(m, glm::radians(datasetRotation.z), vec3(0,0,1));
    m = glm::scale(m, datasetScales);
    m = glm::translate(m, -datasetOrigin);
    
    m = glm::scale(m, 1.f/datasetScales);
    m = glm::translate(m, translate);
    m = glm::translate(m, origin);
    m = glm::rotate(m, glm::radians((float)rotate[0]), vec3(1, 0, 0));
    m = glm::rotate(m, glm::radians((float)rotate[1]), vec3(0, 1, 0));
    m = glm::rotate(m, glm::radians((float)rotate[2]), vec3(0, 0, 1));
    m = glm::scale(m, scale);
    m = glm::translate(m, -origin);
    m = glm::scale(m, datasetScales);
//    PrintVec3(datasetScales);
    
    struct {
        vec3 x, y, z, o;
    } affine;
    
    affine.x = m*vec4(vec3(1,0,0),0);
    affine.y = m*vec4(vec3(0,1,0),0);
    affine.z = m*vec4(vec3(0,0,1),0);
    affine.o = m*vec4(vec3(0,0,0),1);
    
    ospSetParam(_ospInstance, "xfm", OSP_AFFINE3F, &affine);
    ospCommit(_ospInstance);
    ospCommit(_ospWorld);
}

int VolumeRenderer::_ospLoadData(const Grid *grid)
{
    auto p = GetActiveParams();
    OSPVolume volume;
    
    if (p->GetValueLong("osp_test_volume", 0)) volume = _ospLoadVolumeTest(grid);
    else if (dynamic_cast<const RegularGrid *>     (grid) || p->GetValueLong("osp_force_regular", false)) volume = _ospLoadVolumeRegular(grid);
    else if (dynamic_cast<const StructuredGrid *>  (grid)) volume = _ospLoadVolumeStructured(grid);
    else if (dynamic_cast<const UnstructuredGrid *>(grid)) volume = _ospLoadVolumeUnstructured(grid);
    else volume=0, VAssert(!"Unknown grid type");
    
    ospRelease(_ospVolumeModel);
    _ospVolumeModel = ospNewVolumetricModel(volume);
    ospRelease(volume);
    
    ospRelease(_ospTF);
    _ospTF = ospNewTransferFunction("piecewiseLinear");
    OSPData data = VOSP::NewCopiedData((float[]){0,0,0,1,1,1}, OSP_VEC3F, 2);
    ospCommit(data);
    ospSetObject(_ospTF, "color", data);
    ospRelease(data);
    data = VOSP::NewCopiedData((float[]){0,1}, OSP_FLOAT, 2);
    ospCommit(data);
    ospSetObject(_ospTF, "opacity", data);
    ospRelease(data);
    ospSetVec2f(_ospTF, "valueRange", -FLT_MAX, 0); // These initial values are necessary to work around bugs.
    ospCommit(_ospTF);
    ospSetObject(_ospVolumeModel, "transferFunction", _ospTF);
    ospCommit(_ospVolumeModel);
    
    
    OSPGroup group = ospNewGroup();
    ospSetObjectAsData(group, "volume", OSP_VOLUMETRIC_MODEL, _ospVolumeModel);
    ospCommit(group);
    
//    OSPGeometricModel triangleModel = VOSP::Test::LoadTriangle(vec3(1000000,1000000,1000000));
//    ospSetObjectAsData(group, "geometry", OSP_GEOMETRIC_MODEL, triangleModel);
//    ospCommit(group);
//    ospRelease(triangleModel);
    
    ospRelease(_ospInstance);
    _ospInstance = ospNewInstance(group);
    ospCommit(_ospInstance);
    ospRelease(group);
    
    ospSetObjectAsData(_ospWorld, "instance", OSP_INSTANCE, _ospInstance);
    ospCommit(_ospWorld);
    
    if (dynamic_cast<const RegularGrid *>(grid))
        p->SetValueDouble("osp_volumeSamplingRate", "", 1.f);
    else
        p->SetValueDouble("osp_volumeSamplingRate", "", _ospGuessSamplingRateScalar(grid));
    
    return 0;
}

float VolumeRenderer::_ospGuessSamplingRateScalar(const Grid *grid) const
{
    std::vector<double> dataMinExtD, dataMaxExtD;
    grid->GetUserExtents(dataMinExtD, dataMaxExtD);
    vec3 dataMinExt(dataMinExtD[0], dataMinExtD[1], dataMinExtD[2]);
    vec3 dataMaxExt(dataMaxExtD[0], dataMaxExtD[1], dataMaxExtD[2]);
    vec3 lens = dataMaxExt-dataMinExt;
    float longest = max(lens.x, max(lens.y, lens.z));
    
    // I was going to try to come up with a continuous equation but I'm
    // not sure how the original sample rate is determined so I'm just
    // going off the following two samples of resonable performance vs
    // quality:
    //
    // 4E7 = 0.001
    // 3E6 = 0.1
    
    return longest < 3E6f ? glm::mix(1.f, 0.1f, longest/3E6f) :
            glm::mix(0.1f, 0.001f, (longest-3E6f)/(4.05E7f-3E6f));
}

OSPVolume VolumeRenderer::_ospLoadVolumeRegular(const Grid *grid)
{
    printf("Load Regular Volume");
    
    const vector<size_t> dims = grid->GetDimensions();
    const size_t nVerts = dims[0]*dims[1]*dims[2];
    std::vector<double> dataMinExtD, dataMaxExtD;
    grid->GetUserExtents(dataMinExtD, dataMaxExtD);
    vec3 dataMinExt(dataMinExtD[0], dataMinExtD[1], dataMinExtD[2]);
    vec3 dataMaxExt(dataMaxExtD[0], dataMaxExtD[1], dataMaxExtD[2]);
    vec3 dimsf(dims[0], dims[1], dims[2]);
    vec3 gridSpacing = (dataMaxExt-dataMinExt)/(dimsf-1.f);
    float missingValue = grid->HasMissingData() ? grid->GetMissingValue() : NAN;
    
    float *fdata = new float[nVerts];
    if (!fdata) {
        Wasp::MyBase::SetErrMsg("Could not allocate enough RAM to load data");
        return nullptr;
    }
    auto dataIt = grid->cbegin();
    for (size_t i = 0; i < nVerts; ++i, ++dataIt) {
        fdata[i] = *dataIt == missingValue ? NAN : *dataIt;
    }
    
    OSPData data = VOSP::NewCopiedData(fdata, OSP_FLOAT, dims[0], dims[1], dims[2]);
    ospCommit(data);
    delete [] fdata;
    
    OSPVolume volume = ospNewVolume("structuredRegular");
    
    ospSetObject(volume, "data", data);
    ospRelease(data);
    
    ospSetVec3f(volume, "gridOrigin", dataMinExt.x, dataMinExt.y, dataMinExt.z);
    ospSetVec3f(volume, "gridSpacing", gridSpacing.x, gridSpacing.y, gridSpacing.z);
    
    ospCommit(volume);
    return volume;
}

OSPVolume VolumeRenderer::_ospLoadVolumeStructured(const Grid *grid)
{
    printf("Load Structured Volume");
    
    const vector<size_t> dims = grid->GetDimensions();
    const size_t nVerts = dims[0]*dims[1]*dims[2];
    float missingValue = grid->HasMissingData() ? grid->GetMissingValue() : NAN;
    
    float *vdata = new float[nVerts];
    auto dataIt = grid->cbegin();
    for (size_t i = 0; i < nVerts; ++i, ++dataIt)
        vdata[i] = *dataIt == missingValue ? NAN : *dataIt;
    
    float *cdata = new float[nVerts*3];
    auto coord = grid->ConstCoordBegin();
    for (size_t i = 0; i < nVerts; ++i, ++coord) {
        cdata[i*3  ] = (*coord)[0];
        cdata[i*3+1] = (*coord)[1];
        cdata[i*3+2] = (*coord)[2];
    }
    
    OSPVolume volume = ospNewVolume("unstructured");
    OSPData data;
    
    data = VOSP::NewCopiedData(vdata, OSP_FLOAT, nVerts);
    ospCommit(data);
    ospSetObject(volume, "vertex.data", data);
    ospRelease(data);
    delete [] vdata;
    
    data = VOSP::NewCopiedData(cdata, OSP_VEC3F, nVerts);
    ospCommit(data);
    ospSetObject(volume, "vertex.position", data);
    ospRelease(data);
    delete [] cdata;
    
    int xd = dims[0];
    int yd = dims[1];
    int zd = dims[2];
    int cxd = xd-1;
    int cyd = yd-1;
    int czd = zd-1;
    
    // "indexPrefixed" is broken
    
    typedef struct {
        unsigned int i0, i1, i2, i3, i4, i5, i6, i7;
    } Cell;
    Cell *indices = new Cell[cxd*cyd*czd];
    
#define I(x,y,z) (unsigned int)((z)*yd*xd+(y)*xd+(x))

    for (int z = 0; z < czd; z++) {
        for (int y = 0; y < cyd; y++) {
            for (int x = 0; x < cxd; x++) {
                indices[z*cyd*cxd + y*cxd + x] = {
                    I(x  ,y  ,z  ), I(x+1,y  ,z  ), I(x+1,y+1,z  ), I(x  ,y+1,z  ),
                    I(x  ,y  ,z+1), I(x+1,y  ,z+1), I(x+1,y+1,z+1), I(x  ,y+1,z+1),
                };
            }
        }
    }
#undef I
    
    data = VOSP::NewCopiedData(indices, OSP_UINT, cxd*cyd*czd*8);
    ospCommit(data);
    ospSetObject(volume, "index", data);
    ospRelease(data);
    delete [] indices;
    
    int *startIndex = new int[czd*cyd*cxd];
    for (int i = 0; i < czd*cyd*cxd; i++)
        startIndex[i] = i*8;
    
    data = VOSP::NewCopiedData(startIndex, OSP_UINT, czd*cyd*cxd);
    ospCommit(data);
    ospSetObject(volume, "cell.index", data);
    ospRelease(data);
    delete [] startIndex;
    
    unsigned char *cellType = new unsigned char[czd*cyd*cxd];
    for (int i = 0; i < czd*cyd*cxd; i++)
        cellType[i] = OSP_HEXAHEDRON;
    
    data = VOSP::NewCopiedData(cellType, OSP_UCHAR, czd*cyd*cxd);
    ospCommit(data);
    ospSetObject(volume, "cell.type", data);
    ospRelease(data);
    delete [] cellType;
    
    ospCommit(volume);
    return volume;
}

enum WindingOrder { CCW, CW };
WindingOrder GetWindingOrderRespectToZ(vec3 a, vec3 b, vec3 c)
{
    return glm::cross(b-a, c-b).z > 0 ? CCW : CW;
}

WindingOrder GetWindingOrderTetra(vec3 a, vec3 b, vec3 c, vec3 d)
{
    vec3 n = glm::normalize(glm::cross(glm::normalize(b-a), glm::normalize(c-b)));
    vec3 tc = (a+b+c)/3.f;
    return glm::dot(glm::normalize(d-tc), n) < 0 ? CCW : CW;
}

const char * to_string(WindingOrder o)
{
    return o == CCW ? "CCW" : "CW";
}

OSPVolume VolumeRenderer::_ospLoadVolumeUnstructured(const Grid *grid)
{
    printf("Load Unstructured Volume");
    const vector<size_t> nodeDims = grid->GetDimensions();
    size_t nodeDim = nodeDims.size();
    const size_t nVerts = nodeDims[0]*nodeDims[1];
    const vector<size_t> cellDims = grid->GetCellDimensions();
    const size_t nCells = cellDims[0]*cellDims[1];
    VAssert(nodeDim == 2 && cellDims.size() == 2);
    
    float missingValue = grid->HasMissingData() ? grid->GetMissingValue() : NAN;
    size_t maxNodes = grid->GetMaxVertexPerCell();
    size_t coordDim = grid->GetGeometryDim();
    size_t *nodes = (size_t*)alloca(sizeof(size_t) * maxNodes * nodeDim);
    
    printf("nVerts = %li\n", nVerts);
    printf("maxNodes = %li\n", maxNodes);
    printf("coordDim = %li\n", coordDim);
    printf("nodeDim = %li\n", nodeDim);
    
    float *vdata = new float[nVerts];
    auto dataIt = grid->cbegin();
    for (size_t i = 0; i < nVerts; ++i, ++dataIt)
        vdata[i] = *dataIt == missingValue ? NAN : *dataIt;
    
    float *cdata = new float[nVerts*3];
    auto coord = grid->ConstCoordBegin();
    for (size_t i = 0; i < nVerts; ++i, ++coord) {
        cdata[i*3  ] = (*coord)[0];
        cdata[i*3+1] = (*coord)[1];
        cdata[i*3+2] = (*coord)[2];
    }
    
    vector<unsigned int> cellIndices;
    vector<unsigned int> cellStarts;
    vector<unsigned char> cellTypes;
    
    unsigned added[32] = {0};
    unsigned skipped[32] = {0};
    
//    int maxCells = std::min((int)nCells, (int)GetActiveParams()->GetValueLong("osp_max_cells", 1));
    
    auto cellIt = grid->ConstCellBegin();
    for (size_t cellCounter = 0; cellCounter < nCells; ++cellIt, ++cellCounter) {
        const vector<size_t> &cell = *cellIt;
        int numNodes;
        grid->GetCellNodes(cell.data(), nodes, numNodes);
        
        if (numNodes == 4000) {
            cellStarts.push_back(cellIndices.size());
            cellTypes.push_back(OSP_TETRAHEDRON);
            added[numNodes]++;
            
            for (int i = 0; i < 4; i++)
                cellIndices.push_back(nodes[i*nodeDim] + nodes[i*nodeDim+1]*nodeDims[0]);
        }
        else if (numNodes == 6) {
            cellStarts.push_back(cellIndices.size());
            cellTypes.push_back(OSP_WEDGE);
            added[numNodes]++;
            
            for (int i = 0; i < 6; i++)
                cellIndices.push_back(nodes[i*nodeDim] + nodes[i*nodeDim+1]*nodeDims[0]);
        }
        else {
            skipped[numNodes]++;
        }
        
//        if (cellCounter >= maxCells-1) {
//            printf("WARNING BREAKING EARLY\n");
//            break;
//        }
    }
    
    int totalAdded=0, totalSkipped=0;
    for (int i = 0; i < 32; i++) {
        if (added[i] > 0) printf("Added[%i] = %i\n", i, added[i]);
        if (skipped[i] > 0) printf("Skipped[%i] = %i\n", i, skipped[i]);
        totalAdded += added[i];
        totalSkipped += skipped[i];
    }
    printf("Total Added = %i\n", totalAdded);
    printf("Total Skipped = %i\n", totalSkipped);
//    printf("# Coords = %li\n", nVerts);
    
    vec3 *coords = (vec3*)cdata;
    
    
    for (unsigned i = 0; i < cellStarts.size(); i++) {
        unsigned start = cellStarts[i];
        unsigned type = cellTypes[i];
        
        if (type == OSP_WEDGE) {
            if (CW == GetWindingOrderRespectToZ(coords[cellIndices[start+0]], coords[cellIndices[start+1]], coords[cellIndices[start+2]])) {
                swap(cellIndices[start+1], cellIndices[start+2]);
                swap(cellIndices[start+1+3], cellIndices[start+2+3]);
            }
        }
        else if (type == OSP_TETRAHEDRON) {
//            if (CW == GetWindingOrderTetra(coords[cellIndices[start+0]], coords[cellIndices[start+1]], coords[cellIndices[start+2]], coords[cellIndices[start+3]]))
//                swap(cellIndices[start+1], cellIndices[start+2]);
//            assert(CCW == GetWindingOrderTetra(coords[cellIndices[start+0]], coords[cellIndices[start+1]], coords[cellIndices[start+2]], coords[cellIndices[start+3]]));
        }
    }
    
    
    int testCell = std::min(cellStarts.size()-1, (size_t)GetActiveParams()->GetValueLong("osp_test_cells", 0));
    if (testCell >= 0) {
        int testCellNodes = cellTypes[testCell] == OSP_TETRAHEDRON ? 4 : 6;
        printf("Cell[%i].nodes = %i\n", testCell, testCellNodes);
        vec3 testCellCoords[testCellNodes];
        for (int i = 0; i < testCellNodes; i++) {
            int idx = cellIndices[cellStarts[testCell]+i];
            testCellCoords[i] = coords[idx];
            printf("\tCells[%i].vert[%i] = coords[%i] = (%f, %f, %f)\n", testCell, i, idx, coords[idx].x, coords[idx].y, coords[idx].z);
        }
        printf("Winding bottom = %s\n", to_string(GetWindingOrderRespectToZ(testCellCoords[0], testCellCoords[1], testCellCoords[2])));
        if (testCellNodes == 6)
            printf("Winding top = %s\n", to_string(GetWindingOrderRespectToZ(testCellCoords[3], testCellCoords[4], testCellCoords[5])));
    }
    
    
    // Sanity Checks
    for (auto i : cellIndices) assert(i < nVerts);
    for (auto i : cellStarts) assert(i < cellIndices.size());
    for (auto i : cellTypes) assert(i == OSP_WEDGE || i == OSP_TETRAHEDRON);
    assert(cellStarts[cellStarts.size()-1] + (cellTypes[cellTypes.size()-1] == OSP_WEDGE ? 6 : 4) == cellIndices.size());
    assert(cellStarts.size() == cellTypes.size());
    
    OSPVolume volume = ospNewVolume("unstructured");
    OSPData data;
    
    data = VOSP::NewCopiedData(vdata, OSP_FLOAT, nVerts);
    ospCommit(data);
    ospSetObject(volume, "vertex.data", data);
    ospRelease(data);
    
    data = VOSP::NewCopiedData(cdata, OSP_VEC3F, nVerts);
    ospCommit(data);
    ospSetObject(volume, "vertex.position", data);
    ospRelease(data);
    
    data = VOSP::NewCopiedData(cellIndices.data(), OSP_UINT, cellIndices.size());
    ospCommit(data);
    ospSetObject(volume, "index", data);
    ospRelease(data);
    
    data = VOSP::NewCopiedData(cellStarts.data(), OSP_UINT, cellStarts.size());
    ospCommit(data);
    ospSetObject(volume, "cell.index", data);
    ospRelease(data);
    
    data = VOSP::NewCopiedData(cellTypes.data(), OSP_UCHAR, cellTypes.size());
    ospCommit(data);
    ospSetObject(volume, "cell.type", data);
    ospRelease(data);
    
    delete [] vdata;
    delete [] cdata;
    
    ospCommit(volume);
    return volume;
}

OSPVolume VolumeRenderer::_ospLoadVolumeTest(const Grid *grid)
{
    printf("Load Test Volume");
    
    std::vector<double> dataMinExtD, dataMaxExtD;
    grid->GetUserExtents(dataMinExtD, dataMaxExtD);
    vec3 dataMinExt(dataMinExtD[0], dataMinExtD[1], dataMinExtD[2]);
    vec3 dataMaxExt(dataMaxExtD[0], dataMaxExtD[1], dataMaxExtD[2]);
    int nVerts = 8;
    
    float values[nVerts];
    for (int i = 0; i < nVerts; i++)
        values[i] = 0;
    
    vec3 coords[nVerts];
    float s = 1;
    vec3 l = s * dataMinExt;
    vec3 h = s * dataMaxExt;
    coords[0] = vec3(l.x, l.y, l.z);
    coords[1] = vec3(h.x, l.y, l.z);
    coords[2] = vec3(h.x, h.y, l.z);
    coords[3] = vec3(l.x, h.y, l.z);
    coords[4] = vec3(l.x, l.y, h.z);
    coords[5] = vec3(h.x, l.y, h.z);
    coords[6] = vec3(h.x, h.y, h.z);
    coords[7] = vec3(l.x, h.y, h.z);
    
    OSPVolume volume = ospNewVolume("unstructured");
    OSPData data;
    
    data = VOSP::NewCopiedData(values, OSP_FLOAT, nVerts);
    ospCommit(data);
    ospSetObject(volume, "vertex.data", data);
    ospRelease(data);
    
    data = VOSP::NewCopiedData(coords, OSP_VEC3F, nVerts);
    ospCommit(data);
    ospSetObject(volume, "vertex.position", data);
    ospRelease(data);
    
    vector<unsigned int> cellIndices;
    vector<unsigned int> cellStarts;
    vector<unsigned int> cellTypes;
    
    cellStarts.push_back(cellIndices.size());
    cellTypes.push_back(OSP_HEXAHEDRON);
    cellIndices.push_back(0);
    cellIndices.push_back(1);
    cellIndices.push_back(2);
    cellIndices.push_back(3);
    cellIndices.push_back(4);
    cellIndices.push_back(5);
    cellIndices.push_back(6);
    cellIndices.push_back(7);
    
    data = VOSP::NewCopiedData(cellIndices.data(), OSP_UINT, cellIndices.size());
    ospCommit(data);
    ospSetObject(volume, "index", data);
    ospRelease(data);
    
    data = VOSP::NewCopiedData(cellStarts.data(), OSP_UINT, cellStarts.size());
    ospCommit(data);
    ospSetObject(volume, "cell.index", data);
    ospRelease(data);
    
    data = VOSP::NewCopiedData(cellTypes.data(), OSP_UCHAR, cellTypes.size());
    ospCommit(data);
    ospSetObject(volume, "cell.type", data);
    ospRelease(data);
    
    ospCommit(volume);
    return volume;
}

void VolumeRenderer::_ospDelete()
{
    if (_ospRenderer    ) ospRelease(_ospRenderer    );
    if (_ospWorld       ) ospRelease(_ospWorld       );
    if (_ospCamera      ) ospRelease(_ospCamera      );
    if (_ospTF          ) ospRelease(_ospTF          );
    if (_ospInstance    ) ospRelease(_ospInstance    );
    if (_ospVolumeModel ) ospRelease(_ospVolumeModel );
    if (_ospLightDistant) ospRelease(_ospLightDistant);
    
}
