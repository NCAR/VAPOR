#include "vapor/DVRenderer.h"

#define GLERROR -5

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RendererRegistrar<DVRenderer> registrar(DVRenderer::GetClassType(), DVRParams::GetClassType());

DVRenderer::DVRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr)
: RayCaster(pm, winName, dataSetName, DVRParams::GetClassType(), DVRenderer::GetClassType(), instName, dataMgr)
{
}

int DVRenderer::_load3rdPassShaders()
{
    ShaderProgram *shader = nullptr;
    if ((shader = _glManager->shaderManager->GetShader("DVR3rdPassMode1")))
        _3rdPassMode1Shader = shader;
    else
        return GLERROR;

    if ((shader = _glManager->shaderManager->GetShader("DVR3rdPassMode2")))
        _3rdPassMode2Shader = shader;
    else
        return GLERROR;

    return 0;    // Success
}

void DVRenderer::_3rdPassSpecialHandling(bool fast, int castingMode, bool use2ndVar)
{
    // Collect existing depth value of the scene.
    glActiveTexture(GL_TEXTURE0 + _depthTexOffset);
    glBindTexture(GL_TEXTURE_2D, _depthTextureId);
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, _currentViewport[0], _currentViewport[1], _currentViewport[2], _currentViewport[3], 0);
    _3rdPassShader->SetUniform("depthTexture", _depthTexOffset);
    // Note: Don't bind 0 to GL_TEXTURE_2D yet; it'll result in bad rendering,
    //   though I don't know why...
}
