#include "vapor/VolumeRenderer.h"
#include <vapor/VolumeParams.h>

#include <vapor/MatrixManager.h>
#include <vapor/GLManager.h>
#include <vapor/glutil.h>
#include <glm/glm.hpp>
#include <vapor/VolumeRegular.h>
#include <vapor/VolumeCellTraversal.h>
#include <vapor/VolumeOSPRay.h>

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
    
    return 0;
}

int VolumeRenderer::_paintGL(bool fast)
{
    if (fast && _wasTooSlowForFastRender())
        return 0;
    
    auto p = GetActiveParams();
    CheckCache(_cache.ospMaxCells,         p->GetValueLong("osp_max_cells", 1));
    CheckCache(_cache.ospTestCellId,       p->GetValueLong("osp_test_cells", 1));
    CheckCache(_cache.osp_force_regular,   p->GetValueLong("osp_force_regular", 0));
    CheckCache(_cache.osp_test_volume,     p->GetValueLong("osp_test_volume", 0));
    CheckCache(_cache.osp_decompose,       p->GetValueLong("osp_decompose", 0));
    CheckCache(_cache.osp_enable_clipping, p->GetValueLong("osp_enable_clipping", 0));
    
    if (_initializeAlgorithm() < 0) return -1;
    if (_loadData() < 0) return -1;
    if (_loadSecondaryData() < 0) return -1;
    _cache.needsUpdate = false;
    
    _algorithm->SaveDepthBuffer(fast);
    _initializeFramebuffer(fast);
    
    glEnable(GL_BLEND);
    int src, dst;
    _algorithm->GetFinalBlendingMode(&src, &dst);
    glBlendFunc(src, dst);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    
    void *start = GLManager::BeginTimer();
    
    _algorithm->Render(fast);
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

std::string VolumeRenderer::_getColorbarVariableName() const
{
    VolumeParams *vp = dynamic_cast<VolumeParams *>(GetActiveParams());
    if (vp->GetValueLong(VolumeParams::UseColormapVariableTag, 0))
        return vp->GetColorMapVariableName();
    else
        return vp->GetVariableName();
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
    
    if (_cache.algorithmName != vp->GetAlgorithm()) {
        _cache.algorithmName = vp->GetAlgorithm();
        if (_cache.algorithmName == "")
            _cache.algorithmName = "NULL";
        if (_algorithm) delete _algorithm;
        _algorithm = VolumeAlgorithm::NewAlgorithm(_cache.algorithmName, _glManager, this);
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
    CheckCache(_cache.ospMaxCells, RP->GetValueLong("osp_max_cells", 1));
    if (!_cache.needsUpdate)
        return 0;
    
    Grid *grid = _dataMgr->GetVariable(_cache.ts, _cache.var, _cache.refinement, _cache.compression, _cache.minExt, _cache.maxExt);
    if (!grid)
        return -1;
    
    if (dynamic_cast<const UnstructuredGrid *>(grid) && !dynamic_cast<VolumeOSPRay*>(_algorithm)) {
        MyBase::SetErrMsg("Unstructured grids are not supported by the GPU renderer");
        return -1;
    }

	// Actual min and max extents of returned grid, which are in general 
	// larger than requested extents.
	//
	grid->GetUserExtents(_dataMinExt, _dataMaxExt);
    
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

std::string VolumeRenderer::_getDefaultAlgorithmForGrid(const Grid *grid) const
{
    bool intel = GLManager::GetVendor() == GLManager::Vendor::Intel;
    
    if (dynamic_cast<const RegularGrid *>   (grid)) return VolumeRegular      ::GetName();
    if (dynamic_cast<const StructuredGrid *>(grid)) return intel ? VolumeRegular::GetName() : VolumeCellTraversal::GetName();
    if (dynamic_cast<const UnstructuredGrid *>(grid)) return VolumeOSPRay::GetName();
    MyBase::SetErrMsg("Unsupported grid type: %s", grid->GetType().c_str());
    return "";
}

bool VolumeRenderer::_needToSetDefaultAlgorithm() const
{
    return !((VolumeParams*)GetActiveParams())->GetAlgorithmWasManuallySetByUser();
}
