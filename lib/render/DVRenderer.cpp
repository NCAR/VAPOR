#include "vapor/DVRenderer.h"
#include "vapor/ResourcePath.h"

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
#warning This needs to use the ShaderManager
    std::vector<std::string> extraPath;
    std::string              shaderPath = Wasp::GetSharePath("shaders/main");
    std::string              VShader1stPass = shaderPath + "/DVR1stPass.vgl";
    std::string              FShader1stPass = shaderPath + "/DVR1stPass.fgl";
    std::string              VShader2ndPass = shaderPath + "/DVR2ndPass.vgl";
    std::string              FShader2ndPass = shaderPath + "/DVR2ndPass.fgl";
    std::string              VShader3rdPassMode1 = shaderPath + "/DVR3rdPassMode1.vgl";
    std::string              FShader3rdPassMode1 = shaderPath + "/DVR3rdPassMode1.fgl";
    std::string              VShader3rdPassMode2 = shaderPath + "/DVR3rdPassMode2.vgl";
    std::string              FShader3rdPassMode2 = shaderPath + "/DVR3rdPassMode2.fgl";

    _1stPassShaderId = _compileShaders(VShader1stPass.data(), FShader1stPass.data());
    _2ndPassShaderId = _compileShaders(VShader2ndPass.data(), FShader2ndPass.data());
    _3rdPassMode1ShaderId = _compileShaders(VShader3rdPassMode1.data(), FShader3rdPassMode1.data());
    _3rdPassMode2ShaderId = _compileShaders(VShader3rdPassMode2.data(), FShader3rdPassMode2.data());

    auto timenow = chrono::system_clock::to_time_t(chrono::system_clock::now());
    std::cout << std::endl << "Shaders compiled at: " << ctime(&timenow) << std::endl;
}
