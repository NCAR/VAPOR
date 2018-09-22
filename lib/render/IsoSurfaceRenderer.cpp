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
    std::string VShader3rdPassMode1 = shaderPath + "/IsoSurface3rdPassMode1.vgl";
    std::string FShader3rdPassMode1 = shaderPath + "/IsoSurface3rdPassMode1.fgl";
    std::string VShader3rdPassMode2 = shaderPath + "/IsoSurface3rdPassMode2.vgl";
    std::string FShader3rdPassMode2 = shaderPath + "/IsoSurface3rdPassMode2.fgl";

    _1stPassShaderId = _compileShaders(VShader1stPass.data(), FShader1stPass.data());
    _2ndPassShaderId = _compileShaders(VShader2ndPass.data(), FShader2ndPass.data());
    _3rdPassMode1ShaderId = _compileShaders(VShader3rdPassMode1.data(), FShader3rdPassMode1.data());
    _3rdPassMode2ShaderId = _compileShaders(VShader3rdPassMode2.data(), FShader3rdPassMode2.data());
}

void IsoSurfaceRenderer::_3rdPassSpecialHandling(bool fast, long castingMode)
{
    IsoSurfaceParams *  params = dynamic_cast<IsoSurfaceParams *>(GetActiveParams());
    bool                lighting = params->GetLighting();
    std::vector<double> isoValues = params->GetIsoValues();
    std::vector<bool>   isoFlags = params->GetEnabledIsoValueFlags();

    // Special handling for IsoSurface #1:
    //   honor GUI lighting selection even in fast rendering mode.
    glUniform1i(glGetUniformLocation(_3rdPassShaderId, "lighting"), int(lighting));
    if (lighting) {
        std::vector<double> coeffsD = params->GetLightingCoeffs();
        float               coeffsF[4] = {(float)coeffsD[0], (float)coeffsD[1], (float)coeffsD[2], (float)coeffsD[3]};
        glUniform1fv(glGetUniformLocation(_3rdPassShaderId, "lightingCoeffs"), (GLsizei)4, coeffsF);
    }

    // Special handling for IsoSurface #2:
    //   pass in *normalized* iso values.
    std::vector<float> validValues;
    for (int i = 0; i < isoFlags.size(); i++)
        if (isoFlags[i]) validValues.push_back((float(isoValues[i]) - _userCoordinates.valueRange[0]) / (_userCoordinates.valueRange[1] - _userCoordinates.valueRange[0]));
    int numOfIsoValues = (int)validValues.size();

    glUniform1i(glGetUniformLocation(_3rdPassShaderId, "numOfIsoValues"), numOfIsoValues);

    glUniform1fv(glGetUniformLocation(_3rdPassShaderId, "isoValues"), (GLsizei)numOfIsoValues, validValues.data());
}
