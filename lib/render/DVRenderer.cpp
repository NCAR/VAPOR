#include "vapor/DVRenderer.h"

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RendererRegistrar<DVRenderer> registrar(DVRenderer::GetClassType(), DVRParams::GetClassType());

DVRenderer::DVRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr)
: RayCaster(pm, winName, dataSetName, DVRParams::GetClassType(), DVRenderer::GetClassType(), instName, dataMgr)
{
}

int DVRenderer::_loadShaders()
{
    ShaderProgram *shader = nullptr;
    if ((shader = _glManager->shaderManager->GetShader("DVR1stPass")))
        _1stPassShaderId = shader->GetID();
    else
        return 1;
    if ((shader = _glManager->shaderManager->GetShader("DVR2ndPass")))
        _2ndPassShaderId = shader->GetID();
    else
        return 1;
    if ((shader = _glManager->shaderManager->GetShader("DVR3rdPassMode1")))
        _3rdPassMode1ShaderId = shader->GetID();
    else
        return 1;
    if ((shader = _glManager->shaderManager->GetShader("DVR3rdPassMode2")))
        _3rdPassMode2ShaderId = shader->GetID();
    else
        return 1;

    return 0;    // Success
}
