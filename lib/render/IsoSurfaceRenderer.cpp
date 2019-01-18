#include "vapor/IsoSurfaceRenderer.h"

#define GLERROR -5

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RendererRegistrar<IsoSurfaceRenderer> registrar(IsoSurfaceRenderer::GetClassType(),
                                                       IsoSurfaceParams::GetClassType());

IsoSurfaceRenderer::IsoSurfaceRenderer(const ParamsMgr *pm,
                                       std::string &winName,
                                       std::string &dataSetName,
                                       std::string &instName,
                                       DataMgr *dataMgr)
    : RayCaster(pm,
                winName,
                dataSetName,
                IsoSurfaceParams::GetClassType(),
                IsoSurfaceRenderer::GetClassType(),
                instName,
                dataMgr) {}

int IsoSurfaceRenderer::_load3rdPassShaders() {
    ShaderProgram *shader = nullptr;
    if ((shader = _glManager->shaderManager->GetShader("IsoSurface3rdPassMode1")))
        _3rdPassMode1Shader = shader;
    else
        return GLERROR;

    if ((shader = _glManager->shaderManager->GetShader("IsoSurface3rdPassMode2")))
        _3rdPassMode2Shader = shader;
    else
        return GLERROR;

    return 0; // Success
}

void IsoSurfaceRenderer::_3rdPassSpecialHandling(bool fast, int castingMode) const {
    IsoSurfaceParams *params = dynamic_cast<IsoSurfaceParams *>(GetActiveParams());
    std::vector<double> isoValues = params->GetIsoValues();
    std::vector<bool> isoFlags = params->GetEnabledIsoValueFlags();

    // Special handling for IsoSurface: pass in iso values.
    std::vector<float> validValues;
    for (int i = 0; i < isoFlags.size(); i++) {
        if (isoFlags[i])
            validValues.push_back(float(isoValues[i]));
    }
    int numOfIsoValues = (int)validValues.size();
    for (int i = numOfIsoValues; i < 4; i++)
        validValues.push_back(0.0f);

    _3rdPassShader->SetUniform("numOfIsoValues", numOfIsoValues);
    _3rdPassShader->SetUniformArray("isoValues", 4, validValues.data());

    bool use2ndVar = _use2ndVariable(params);
    _3rdPassShader->SetUniform("use2ndVar", int(use2ndVar));

    // Pass in these textures no matter the secondary variable is used or not.
    // Because Apple doesn't like empty uniforms...
    glActiveTexture(GL_TEXTURE0 + _2ndVarDataTexOffset);
    glBindTexture(GL_TEXTURE_3D, _2ndVarDataTexId);
    _3rdPassShader->SetUniform("secondVarDataTexture", _2ndVarDataTexOffset);

    glActiveTexture(GL_TEXTURE0 + _2ndVarMaskTexOffset);
    glBindTexture(GL_TEXTURE_3D, _2ndVarMaskTexId);
    _3rdPassShader->SetUniform("secondVarMaskTexture", _2ndVarMaskTexOffset);
}

void IsoSurfaceRenderer::_colormapSpecialHandling() {
    IsoSurfaceParams *params = dynamic_cast<IsoSurfaceParams *>(GetActiveParams());
    if (_use2ndVariable(params)) {
        VAPoR::MapperFunction *mapperFunc = params->RenderParams::GetMapperFunc(params->GetColorMapVariableName());
        mapperFunc->makeLut(_colorMap);
        assert(_colorMap.size() % 4 == 0);
        std::vector<double> range = mapperFunc->getMinMaxMapValue();
        _colorMapRange[0] = float(range[0]);
        _colorMapRange[1] = float(range[1]);
        _colorMapRange[2] = (_colorMapRange[1] - _colorMapRange[0]) > 1e-5f ? (_colorMapRange[1] - _colorMapRange[0]) : 1e-5f;
    }
    // Note: _colorMapRange[2] keeps the range of a color map, which will later be used in the shader.
    //       However, this range cannot be zero to prevent infinity values being generated.
}

bool IsoSurfaceRenderer::_use2ndVariable(const RayCasterParams *params) const {
    if (params->UseSingleColor())
        return false;
    else
        return true;
}

void IsoSurfaceRenderer::_update2ndVarTextures() {
    const size_t *dims = _userCoordinates.dims;
    IsoSurfaceParams *params = dynamic_cast<IsoSurfaceParams *>(GetActiveParams());
    bool use2ndVar = _use2ndVariable(params);

    glActiveTexture(GL_TEXTURE0 + _2ndVarDataTexOffset);
    glBindTexture(GL_TEXTURE_3D, _2ndVarDataTexId);
    float dummyVolume[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    if (use2ndVar) {
#ifdef Darwin
        //
        // Intel driver on MacOS seems to not able to correctly update the texture content
        //   when the texture is moderately big. This workaround of loading a dummy texture
        //   to force it to update seems to resolve this issue.
        //
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 2, 2, 2, 0, GL_RED, GL_FLOAT, dummyVolume);
#endif
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, dims[0], dims[1], dims[2], 0,
                     GL_RED, GL_FLOAT, _userCoordinates.secondVarData);
    } else {
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, 2, 2, 2, 0, GL_RED, GL_FLOAT, dummyVolume);
    }

    // Now we HAVE TO attach a missing value mask texture, because
    //   Intel driver on Mac doesn't like leaving the texture empty...
    glActiveTexture(GL_TEXTURE0 + _2ndVarMaskTexOffset);
    glBindTexture(GL_TEXTURE_3D, _2ndVarMaskTexId);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Alignment adjustment. Stupid OpenGL thing.
    if (_userCoordinates.secondVarMask) {
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, dims[0], dims[1], dims[2], 0,
                     GL_RED_INTEGER, GL_UNSIGNED_BYTE, _userCoordinates.secondVarMask);
    } else {
        unsigned char dummyMask[8] = {0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u};
        glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, 2, 2, 2, 0,
                     GL_RED_INTEGER, GL_UNSIGNED_BYTE, dummyMask);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // Restore default alignment.
}
