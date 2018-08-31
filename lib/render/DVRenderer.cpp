#include "vapor/DVRenderer.h"
#include "vapor/GetAppPath.h"

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
    std::vector<std::string> extraPath;
    extraPath.push_back("shaders");
    extraPath.push_back("main");
    std::string shaderPath = Wasp::GetAppPath("VAPOR", "share", extraPath);
    std::string VShader1stPass = shaderPath + "/DVR1stPass.vgl";
    std::string FShader1stPass = shaderPath + "/DVR1stPass.fgl";
    std::string VShader2ndPass = shaderPath + "/DVR2ndPass.vgl";
    std::string FShader2ndPass = shaderPath + "/DVR2ndPass.fgl";
    std::string VShader3rdPass = shaderPath + "/DVR3rdPass.vgl";
    std::string FShader3rdPass = shaderPath + "/DVR3rdPass.fgl";

    _1stPassShaderId = _compileShaders(VShader1stPass.data(), FShader1stPass.data());
    _2ndPassShaderId = _compileShaders(VShader2ndPass.data(), FShader2ndPass.data());
    _3rdPassShaderId = _compileShaders(VShader3rdPass.data(), FShader3rdPass.data());
}
