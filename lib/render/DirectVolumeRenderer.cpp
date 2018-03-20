#include <vapor/glutil.h>    // Must be included first!!!
#include <vapor/DirectVolumeRenderer.h>
#include <iostream>

using namespace VAPoR;

//
// Register class with object factory!!!
//
static RendererRegistrar<DirectVolumeRenderer> registrar(DirectVolumeRenderer::GetClassType(), DVRParams::GetClassType());

DirectVolumeRenderer::DirectVolumeRenderer(const ParamsMgr *pm, std::string &winName, std::string &dataSetName, std::string &instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, DVRParams::GetClassType(), DirectVolumeRenderer::GetClassType(), instName, dataMgr)
{
}

DirectVolumeRenderer::~DirectVolumeRenderer() {}

int DirectVolumeRenderer::_initializeGL()
{
    std::cout << "    **** OpenGL Info ****" << std::endl;
    std::cout << "    OpenGL version : " << glGetString(GL_VERSION) << std::endl;
    std::cout << "    GLSL version   : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    std::cout << "    Vendor         : " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "    Renderer       : " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "    **** OpenGL Info ****" << std::endl;

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
            return -1;
        }
    }

    rc = (int)_shaderMgr->AttributeLocation(_effectNameAttrStr, _vertexDataAttrStr);
    if (rc < 0) {
        std::cerr << "AttributeLocation() failed" << std::endl;
        return -1;
    }
    _vertexDataAttr = rc;

    return 0;
}

int DirectVolumeRenderer::_paintGL()
{
    std::cout << "_paintGL() called" << std::endl;
    return 0;
}
