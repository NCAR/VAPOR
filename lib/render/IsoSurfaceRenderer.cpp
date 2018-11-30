#include "vapor/IsoSurfaceRenderer.h"

#include <chrono>
#include <ctime>

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
    ShaderProgram *shader = _glManager->shaderManager->GetShader("IsoSurface1stPass");
    _1stPassShaderId = shader->GetID();
    shader = _glManager->shaderManager->GetShader("IsoSurface2ndPass");
    _2ndPassShaderId = shader->GetID();
    shader = _glManager->shaderManager->GetShader("IsoSurface3rdPassMode1");
    _3rdPassMode1ShaderId = shader->GetID();
    shader = _glManager->shaderManager->GetShader("IsoSurface3rdPassMode2");
    _3rdPassMode2ShaderId = shader->GetID();

    auto timenow = chrono::system_clock::to_time_t(chrono::system_clock::now());
    std::cout << std::endl << "Shaders compiled at: " << ctime(&timenow) << std::endl;
}

void IsoSurfaceRenderer::_3rdPassSpecialHandling(bool fast, long castingMode)
{
    IsoSurfaceParams *  params = dynamic_cast<IsoSurfaceParams *>(GetActiveParams());
    std::vector<double> isoValues = params->GetIsoValues();
    std::vector<bool>   isoFlags = params->GetEnabledIsoValueFlags();

    // Special handling for IsoSurface: pass in *normalized* iso values.
    std::vector<float> validValues;
    for (int i = 0; i < isoFlags.size(); i++) {
        if (isoFlags[i]) validValues.push_back((float(isoValues[i]) - _userCoordinates.valueRange[0]) / (_userCoordinates.valueRange[1] - _userCoordinates.valueRange[0]));
    }
    int numOfIsoValues = (int)validValues.size();

    glUniform1i(glGetUniformLocation(_3rdPassShaderId, "numOfIsoValues"), numOfIsoValues);

    glUniform1fv(glGetUniformLocation(_3rdPassShaderId, "isoValues"), (GLsizei)numOfIsoValues, validValues.data());
}
