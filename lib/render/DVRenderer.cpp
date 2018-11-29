#include "vapor/DVRenderer.h"
#include "vapor/ResourcePath.h"
#include "vapor/ShaderManager.h"
#include "vapor/ShaderProgram.h"
#include "vapor/GLManager.h"

#include <chrono>
#include <ctime>

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RendererRegistrar<DVRenderer> registrar(DVRenderer::GetClassType(), DVRParams::GetClassType());

DVRenderer::DVRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr)
: RayCaster(pm, winName, dataSetName, DVRParams::GetClassType(), DVRenderer::GetClassType(), instName, dataMgr)
{
}

void DVRenderer::_loadShaders()
{
    ShaderProgram *shader = _glManager->shaderManager->GetShader("DVR1stPass");
    _1stPassShaderId = shader->GetID();
    shader = _glManager->shaderManager->GetShader("DVR2ndPass");
    _2ndPassShaderId = shader->GetID();
    shader = _glManager->shaderManager->GetShader("DVR3rdPassMode1");
    _3rdPassMode1ShaderId = shader->GetID();
    shader = _glManager->shaderManager->GetShader("DVR3rdPassMode2");
    _3rdPassMode2ShaderId = shader->GetID();

    auto timenow = chrono::system_clock::to_time_t(chrono::system_clock::now());
    std::cout << std::endl << "Shaders compiled at: " << ctime(&timenow) << std::endl;
}
