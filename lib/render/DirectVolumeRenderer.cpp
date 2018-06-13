#include <vapor/glutil.h>    // Must be included first!!!
#include <vapor/DirectVolumeRenderer.h>
#include <iostream>

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RendererRegistrar<DirectVolumeRenderer> registrar(DirectVolumeRenderer::GetClassType(), DVRParams::GetClassType());
// Constructor
DirectVolumeRenderer::DirectVolumeRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, DVRParams::GetClassType(), DirectVolumeRenderer::GetClassType(), instName, dataMgr)
{
    _volumeTextureUnit = 0;
    _colormapTextureUnit = 0;
    _volumeCoordinateTextureUnit = 0;
}

// Destructor
DirectVolumeRenderer::~DirectVolumeRenderer()
{
    /* A few useful files from VAPOR2:
         DVRRayCaster.h/cpp
         DVRTexture3d.h/cpp
         TextureBrick.h/cpp
         DVRShader.h/cpp
    */
    if (_volumeTextureUnit) glDeleteTextures(1, &_volumeTextureUnit);
    if (_colormapTextureUnit) glDeleteTextures(1, &_colormapTextureUnit);
    if (_volumeCoordinateTextureUnit) glDeleteTextures(1, &_volumeCoordinateTextureUnit);
}

int DirectVolumeRenderer::_initializeGL()
{
    int maxTextureUnits;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxTextureUnits);
    std::cout << "    **** System Info ****" << std::endl;
    std::cout << "    OpenGL version : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "    GLSL version   : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "    Vendor         : " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "    Renderer       : " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "    Max number of texture units: " << maxTextureUnits << std::endl;
    std::cout << "    **** System Info ****" << std::endl;

    if (!_shaderMgr) {
        std::cerr << "Programmable shading not available" << std::endl;
        SetErrMsg("Programmable shading not available");
        return (-1);
    }

    int rc;
    if (!_shaderMgr->EffectExists(_effectNameStr)) {
        rc = _shaderMgr->DefineEffect(_effectNameStr, "", _effectNameStr);
        if (rc < 0) {
            std::cerr << "DefineEffect() failed" << std::endl;
            SetErrMsg("DefineEffect() failed");
            return -1;
        }
    }

    /* good texture tutorial:
       https://open.gl/textures */
    glGenTextures(1, &_colormapTextureUnit);
    std::cout << "_colormapTextureUnit = " << _colormapTextureUnit << std::endl;

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, _colormapTextureUnit);
    // glTexImage1D(    GL_TEXTURE_1D, 0, 4, _colormapSize, 0, GL_RGBA, GL_FLOAT, _colormap );
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    return 0;
}

int DirectVolumeRenderer::_paintGL()
{
    std::cout << "_paintGL() called" << std::endl;

    float s = 10000;
    glColor3f(1, 1, 1);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(s, 0);
    glVertex2f(s, s);
    glVertex2f(0, s);
    glEnd();

    return 0;
}

void DirectVolumeRenderer::_saveCacheParams()
{
    DVRParams *params = dynamic_cast<DVRParams *>(GetActiveParams());
    _cacheParams.varName = params->GetVariableName();
    _cacheParams.ts = params->GetCurrentTimestep();
    _cacheParams.level = params->GetRefinementLevel();
    _cacheParams.lod = params->GetCompressionLevel();
    params->GetBox()->GetExtents(_cacheParams.boxMin, _cacheParams.boxMax);

    MapperFunction *mapper = params->GetMapperFunc(_cacheParams.varName);
    _cacheParams.colormap.resize(mapper->getNumEntries() * 4, 0.0f);
    // colormap values aren't filled yet!
}

bool DirectVolumeRenderer::_isCacheDirty() const
{
    DVRParams *params = dynamic_cast<DVRParams *>(GetActiveParams());
    if (_cacheParams.varName != params->GetVariableName()) return true;
    if (_cacheParams.ts != params->GetCurrentTimestep()) return true;
    if (_cacheParams.level != params->GetRefinementLevel()) return true;
    if (_cacheParams.lod != params->GetCompressionLevel()) return true;

    vector<double> min, max;
    params->GetBox()->GetExtents(min, max);
    if (_cacheParams.boxMin != min) return true;
    if (_cacheParams.boxMax != max) return true;

    MapperFunction *mapper = params->GetMapperFunc(_cacheParams.varName);
    // colormap values aren't compared yet!!

    return false;
}
