#include <vapor/VolumeGLSL.h>
#include <vector>
#include <vapor/glutil.h>
#include <glm/glm.hpp>
#include <vapor/GLManager.h>
#include <vapor/ViewpointParams.h>
#include <vapor/VolumeParams.h>

using glm::vec3;
using std::vector;

using namespace VAPoR;

// static VolumeAlgorithmRegistrar<VolumeGLSL> registration;

VolumeGLSL::VolumeGLSL(GLManager *gl, VolumeRenderer *renderer) : VolumeAlgorithm(gl, renderer)
{
    _LUTTexture.Generate();
    _LUT2Texture.Generate();
    _depthTexture.Generate();
}

VolumeGLSL::~VolumeGLSL() {}

void VolumeGLSL::SaveDepthBuffer(bool fast) { _depthTexture.CopyDepthBuffer(); }

int VolumeGLSL::Render(bool fast)
{
    _loadTF();

    ShaderProgram *shader = GetShader();
    if (!shader) return -1;
    shader->Bind();
    _setShaderUniforms(shader, fast);

    return 0;
}

int VolumeGLSL::LoadData(const Grid *grid)
{
    grid->GetUserExtents(_minDataExtents, _maxDataExtents);
    return 0;
}

void VolumeGLSL::GetFinalBlendingMode(int *src, int *dst)
{
    *src = GL_ONE;
    *dst = GL_ONE_MINUS_SRC_ALPHA;
}

void VolumeGLSL::_loadTF()
{
    VolumeParams *vp = GetParams();

    vector<float> constantColor = vp->GetConstantColor();
    constantColor.push_back(vp->GetMapperFunc(vp->GetVariableName())->getOpacityScale());
    _constantColor = constantColor;

    _loadTF(&_LUTTexture, vp->GetMapperFunc(vp->GetVariableName()), &_tf);

    if (_usingColorMapData()) { _loadTF(&_LUT2Texture, vp->GetMapperFunc(vp->GetColorMapVariableName()), &_tf2); }
}

void VolumeGLSL::_loadTF(Texture1D *texture, MapperFunction *tf, MapperFunction **cacheTF)
{
    if (*cacheTF) delete *cacheTF;
    *cacheTF = new MapperFunction(*tf);

    vector<float> LUT(4 * 256);
    _getLUTFromTF(tf, LUT.data());
    texture->TexImage(GL_RGBA8, 256, 0, 0, GL_RGBA, GL_FLOAT, LUT.data());
}

void VolumeGLSL::_getLUTFromTF(const MapperFunction *tf, float *LUT) const
{
    // Constant opacity needs to be removed here and applied in the shader
    // because otherwise we run out of precision in the LUT
    MapperFunction tfSansConstantOpacity(*tf);
    tfSansConstantOpacity.setOpacityScale(1);

    tfSansConstantOpacity.makeLut(LUT);
    for (int i = 0; i < 256; i++) { LUT[4 * i + 3] = powf(LUT[4 * i + 3], 2); }
}

void VolumeGLSL::_setShaderUniforms(const ShaderProgram *shader, const bool fast) const
{
    VolumeParams *   vp = GetParams();
    ViewpointParams *viewpointParams = GetViewpointParams();
    Viewpoint *      viewpoint = viewpointParams->getCurrentViewpoint();
    double           m[16];
    double           cameraPos[3], cameraUp[3], cameraDir[3];
    _glManager->matrixManager->GetDoublev(MatrixManager::Mode::ModelView, m);
    viewpoint->ReconstructCamera(m, cameraPos, cameraUp, cameraDir);

    shader->SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());
    shader->SetUniform("cameraPos", vec3(cameraPos[0], cameraPos[1], cameraPos[2]));
    shader->SetUniform("samplingRateMultiplier", (float)vp->GetSamplingMultiplier());
    shader->SetUniform("lightingEnabled", vp->GetLightingEnabled());
    shader->SetUniform("phongAmbient", vp->GetPhongAmbient());
    shader->SetUniform("phongDiffuse", vp->GetPhongDiffuse());
    shader->SetUniform("phongSpecular", vp->GetPhongSpecular());
    shader->SetUniform("phongShininess", powf(vp->GetPhongShininess(), 2) * 100);

    glm::vec3 dataMin, dataMax, userMin, userMax;
    _getExtents(&dataMin, &dataMax, &userMin, &userMax);
    vec3  extLengths = dataMax - dataMin;
    vec3  extScales = _getVolumeScales();
    vec3  extLengthsScaled = extLengths * extScales;
    float smallestDimension = min(extLengthsScaled[0], min(extLengthsScaled[1], extLengthsScaled[2]));
    float largestDimension = max(extLengthsScaled[0], max(extLengthsScaled[1], extLengthsScaled[2]));

    shader->SetUniform("dataBoundsMin", dataMin);
    shader->SetUniform("dataBoundsMax", dataMax);
    shader->SetUniform("userExtsMin", userMin);
    shader->SetUniform("userExtsMax", userMax);
    shader->SetUniform("unitDistance", largestDimension / 100.f);
    shader->SetUniform("unitOpacityScalar", largestDimension / smallestDimension);
    shader->SetUniform("scales", extScales);

    float density = vp->GetValueDouble(VolumeParams::VolumeDensityTag, 1);
    density = powf(density, 4);

    shader->SetUniform("density", (float)density);
    shader->SetUniform("LUTMin", (float)_tf->getMinMapValue());
    shader->SetUniform("LUTMax", (float)_tf->getMaxMapValue());
    shader->SetUniform("mapOrthoMode", viewpointParams->GetProjectionType() == ViewpointParams::MapOrthographic);

    shader->SetSampler("LUT", _LUTTexture);
    shader->SetSampler("sceneDepth", _depthTexture);

    if (_usingColorMapData()) {
        shader->SetUniform("LUTMin2", (float)_tf2->getMinMapValue());
        shader->SetUniform("LUTMax2", (float)_tf2->getMaxMapValue());
        shader->SetSampler("LUT2", _LUT2Texture);
    }

    shader->SetUniform("fast", fast);

    vector<double> isoValuesD = GetParams()->GetIsoValues();
    vector<float>  isoValues(isoValuesD.begin(), isoValuesD.end());
    vector<bool>   enabledIsoValues(4, false);
    for (int i = 0; i < isoValues.size(); i++) enabledIsoValues[i] = true;
    shader->SetUniformArray("isoValue", isoValues.size(), isoValues.data());
    shader->SetUniform("isoEnabled[0]", (bool)enabledIsoValues[0]);
    shader->SetUniform("isoEnabled[1]", (bool)enabledIsoValues[1]);
    shader->SetUniform("isoEnabled[2]", (bool)enabledIsoValues[2]);
    shader->SetUniform("isoEnabled[3]", (bool)enabledIsoValues[3]);
    if (_constantColor.size() == 4) shader->SetUniform("constantColor", *(glm::vec4 *)_constantColor.data());

    SetUniforms(shader);
}

glm::vec3 VolumeGLSL::_getVolumeScales() const
{
    Transform *datasetTransform = GetDatasetTransform();
    Transform *rendererTransform = GetParams()->GetTransform();
    VAssert(datasetTransform && rendererTransform);

    vector<double> datasetScales, rendererScales;
    datasetScales = datasetTransform->GetScales();
    rendererScales = rendererTransform->GetScales();

    return glm::vec3(datasetScales[0] * rendererScales[0], datasetScales[1] * rendererScales[1], datasetScales[2] * rendererScales[2]);
}

void VolumeGLSL::_getExtents(glm::vec3 *dataMin, glm::vec3 *dataMax, glm::vec3 *userMin, glm::vec3 *userMax) const
{
    vector<double> minRendererExtents, maxRendererExtents;
    GetParams()->GetBox()->GetExtents(minRendererExtents, maxRendererExtents);
    *userMin = vec3(minRendererExtents[0], minRendererExtents[1], minRendererExtents[2]);
    *userMax = vec3(maxRendererExtents[0], maxRendererExtents[1], maxRendererExtents[2]);
    *dataMin = vec3(_minDataExtents[0], _minDataExtents[1], _minDataExtents[2]);
    *dataMax = vec3(_maxDataExtents[0], _maxDataExtents[1], _maxDataExtents[2]);

    // Moving domain allows area outside of data to be selected
    *userMin = glm::max(*userMin, *dataMin);
    *userMax = glm::min(*userMax, *dataMax);
}

bool VolumeGLSL::_usingColorMapData() const
{
    // Overriden by VolumeIsoRenderer
    return GetParams()->GetValueLong(VAPoR::VolumeParams::UseColormapVariableTag, false);
}
