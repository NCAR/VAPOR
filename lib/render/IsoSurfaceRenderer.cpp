#include "vapor/IsoSurfaceRenderer.h"
#include "vapor/GetAppPath.h"

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RendererRegistrar<IsoSurfaceRenderer> registrar(IsoSurfaceRenderer::GetClassType(), IsoSurfaceParams::GetClassType());

IsoSurfaceRenderer::IsoSurfaceRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr)
: RayCaster(pm, winName, dataSetName, IsoSurfaceParams::GetClassType(), IsoSurfaceRenderer::GetClassType(), instName, dataMgr)
{
}

void IsoSurfaceRenderer::_loadShaders()
{
    std::vector<std::string> extraPath;
    extraPath.push_back("shaders");
    extraPath.push_back("main");
    std::string shaderPath = Wasp::GetAppPath("VAPOR", "share", extraPath);
    std::string VShader1stPass = shaderPath + "/IsoSurface1stPass.vgl";
    std::string FShader1stPass = shaderPath + "/IsoSurface1stPass.fgl";
    std::string VShader2ndPass = shaderPath + "/IsoSurface2ndPass.vgl";
    std::string FShader2ndPass = shaderPath + "/IsoSurface2ndPass.fgl";
    std::string VShader3rdPass = shaderPath + "/IsoSurface3rdPass.vgl";
    std::string FShader3rdPass = shaderPath + "/IsoSurface3rdPass.fgl";

    _1stPassShaderId = _compileShaders(VShader1stPass.data(), FShader1stPass.data());
    _2ndPassShaderId = _compileShaders(VShader2ndPass.data(), FShader2ndPass.data());
    _3rdPassShaderId = _compileShaders(VShader3rdPass.data(), FShader3rdPass.data());
}

void IsoSurfaceRenderer::_3rdPassSpecialHandling()
{
    IsoSurfaceParams *  params = dynamic_cast<IsoSurfaceParams *>(GetActiveParams());
    std::vector<double> isoValues = params->GetIsoValues();
    std::vector<bool>   flags = params->GetEnabledIsoValueFlags();
    std::vector<float>  validValues;
    for (int i = 0; i < flags.size(); i++)
        if (flags[i]) validValues.push_back((float)isoValues[i]);
    int numOfIsoValues = (int)validValues.size();

    GLint uniformLocation = glGetUniformLocation(_3rdPassShaderId, "numOfIsoValues");
    glUniform1i(uniformLocation, numOfIsoValues);

    uniformLocation = glGetUniformLocation(_3rdPassShaderId, "isoValues");
    glUniform1fv(uniformLocation, (GLsizei)numOfIsoValues, validValues.data());
}
