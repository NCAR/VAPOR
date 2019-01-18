#include "vapor/DVRenderer.h"

#define GLERROR -5

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RendererRegistrar<DVRenderer> registrar(DVRenderer::GetClassType(),
                                               DVRParams::GetClassType());

DVRenderer::DVRenderer(const ParamsMgr *pm,
                       std::string &winName,
                       std::string &dataSetName,
                       std::string &instName,
                       DataMgr *dataMgr)
    : RayCaster(pm,
                winName,
                dataSetName,
                DVRParams::GetClassType(),
                DVRenderer::GetClassType(),
                instName,
                dataMgr) {}

int DVRenderer::_load3rdPassShaders() {
    ShaderProgram *shader = nullptr;
    if ((shader = _glManager->shaderManager->GetShader("DVR3rdPassMode1")))
        _3rdPassMode1Shader = shader;
    else
        return GLERROR;

    if ((shader = _glManager->shaderManager->GetShader("DVR3rdPassMode2")))
        _3rdPassMode2Shader = shader;
    else
        return GLERROR;

    return 0; // Success
}

void DVRenderer::_3rdPassSpecialHandling(bool fast, int castingMode) const {
    // Collect existing depth value of the scene.
    glActiveTexture(GL_TEXTURE0 + _depthTexOffset);
    glBindTexture(GL_TEXTURE_2D, _depthTextureId);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, _currentViewport[0],
                     _currentViewport[1], _currentViewport[2], _currentViewport[3], 0);
    _3rdPassShader->SetUniform("depthTexture", _depthTexOffset);
}

void DVRenderer::_colormapSpecialHandling() {
    // Get colormap for the primary variable
    DVRParams *params = dynamic_cast<DVRParams *>(GetActiveParams());
    VAPoR::MapperFunction *mapperFunc = params->GetMapperFunc();
    mapperFunc->makeLut(_colorMap);
    assert(_colorMap.size() % 4 == 0);
    std::vector<double> range = mapperFunc->getMinMaxMapValue();
    _colorMapRange[0] = float(range[0]);
    _colorMapRange[1] = float(range[1]);
    _colorMapRange[2] = (_colorMapRange[1] - _colorMapRange[0]) > 1e-5f ? (_colorMapRange[1] - _colorMapRange[0]) : 1e-5f;
    // Note: _colorMapRange[2] keeps the range of a color map, which will later be used in the shader.
    //       However, this range cannot be zero to prevent infinity values being generated.
}
