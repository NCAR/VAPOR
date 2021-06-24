#include <vapor/VolumeOSPRay.h>
#include <vector>
#include <vapor/glutil.h>
#include <glm/glm.hpp>
#include <vapor/GLManager.h>
#include <vapor/RegularGrid.h>
#include <vapor/StructuredGrid.h>
#include <vapor/UnstructuredGrid.h>
#include <vapor/VolumeParams.h>
#include <vapor/ViewpointParams.h>
#include <vapor/AnnotationParams.h>
#include <vapor/Progress.h>

using glm::mat4;
using glm::vec3;
using glm::vec4;
using std::vector;

static vec3 D2V(const vector<double> &dv)
{
    VAssert(dv.size() == 3);
    return vec3(dv[0], dv[1], dv[2]);
}

#define PrintVec3(v) printf("%s = (%f, %f, %f)\n", #v, (v).x, (v).y, (v).z)

using namespace VAPoR;

static VolumeAlgorithmRegistrar<VolumeOSPRay> registration;

std::string VolumeOSPRay::GetName() { return VolumeParams::OSPVolmeAlgorithmName; }

VolumeOSPRay::VolumeOSPRay(GLManager *gl, VolumeRenderer *renderer) : VolumeAlgorithm(gl, renderer)
{
    _ospCamera = ospNewCamera("perspective");
    _ospWorld = ospNewWorld();

    OSPLight lightAmbient = ospNewLight("ambient");
    ospSetFloat(lightAmbient, "intensity", 0.8);
    ospCommit(lightAmbient);
    //    ospSetObjectAsData(_ospWorld, "light", OSP_LIGHT, lightAmbient);
    //    ospRelease(lightAmbient);

    OSPLight lightDistant = ospNewLight("distant");
    ospSetVec3f(lightDistant, "direction", 0, 0, -1);
    ospSetFloat(lightDistant, "angularDiameter", 1);
    ospSetFloat(lightDistant, "intensity", 4);
    ospCommit(lightDistant);

    vector<OSPLight> lights = {lightAmbient, lightDistant};
    OSPData          lightsData = VOSP::NewCopiedData(lights.data(), OSP_LIGHT, lights.size());
    ospCommit(lightsData);

    // Although these are already pointers, unlike every other function,
    // in the case of this function you need to reference here otherwise
    // it will crash.
    ospSetParam(_ospWorld, "light", OSP_LIGHT, &lightsData);

    ospRelease(lightsData);
    _ospLightDistant = lightDistant;    // Not released therefore retained
    _ospLightAmbient = lightAmbient;

    _ospRenderTexture.Generate();
    _ospWriteDepthTexture.Generate();
}

VolumeOSPRay::~VolumeOSPRay()
{
    if (_ospRenderer) ospRelease(_ospRenderer);
    if (_ospWorld) ospRelease(_ospWorld);
    if (_ospCamera) ospRelease(_ospCamera);
    if (_ospTF) ospRelease(_ospTF);
    if (_ospInstance) ospRelease(_ospInstance);
    if (_ospVolumeModel) ospRelease(_ospVolumeModel);
    if (_ospLightAmbient) ospRelease(_ospLightAmbient);
    if (_ospLightDistant) ospRelease(_ospLightDistant);
    if (_ospIso) ospRelease(_ospIso);
    if (_ospIsoModel) ospRelease(_ospIsoModel);
}

void VolumeOSPRay::SaveDepthBuffer(bool fast)
{
    if (!fast) {
        _copyDepth();

        if (GetParams()->GetValueLong("osp_useBackplate", false)) _copyBackplate();
    }
}

int VolumeOSPRay::Render(bool fast)
{
    auto           viewport = GLManager::GetViewport();
    glm::ivec2     fbSize(viewport[2], viewport[3]);
    OSPFrameBuffer framebuffer = ospNewFrameBuffer(fbSize.x, fbSize.y, OSP_FB_SRGBA, OSP_FB_COLOR | OSP_FB_DEPTH | OSP_FB_ACCUM);
    ospResetAccumulation(framebuffer);

    _setupCamera();
    _loadTF();
    _applyTransform();
    _setupRenderer(fast);
    GetExtents(nullptr, nullptr, &_clipBox.min, &_clipBox.max);
    if (_isIso()) _setupIso();

    ospSetFloat(_ospVolumeModel, "densityScale", GetParams()->GetValueDouble(VolumeParams::OSPDensity, 1));
    ospCommit(_ospVolumeModel);

    ospRenderFrameBlocking(framebuffer, _ospRenderer, _ospCamera, _ospWorld);

    const uint32_t *fb = (uint32_t *)ospMapFrameBuffer(framebuffer, OSP_FB_COLOR);
    //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowSize.x, windowSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, fb);
    _ospRenderTexture.TexImage(GL_RGBA, fbSize.x, fbSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, fb);
    ospUnmapFrameBuffer(fb, framebuffer);

    const float *db = (float *)ospMapFrameBuffer(framebuffer, OSP_FB_DEPTH);
    _ospWriteDepthTexture.TexImage(GL_R32F, fbSize.x, fbSize.y, 0, GL_RED, GL_FLOAT, db);
    ospUnmapFrameBuffer(db, framebuffer);

    ospRelease(framebuffer);

    ShaderProgram *framebufferShader = _glManager->shaderManager->GetShader("FramebufferND");
    framebufferShader->Bind();
    if (!framebufferShader) return -1;
    framebufferShader->SetSampler("colorBuffer", _ospRenderTexture);
    framebufferShader->SetSampler("depthBuffer", _ospWriteDepthTexture);
    framebufferShader->SetUniform("MVP", _glManager->matrixManager->GetModelViewProjectionMatrix());

    auto   vp = GetViewpointParams();
    double matrix[16], dpos[3], dup[3], ddir[3];
    vp->GetModelViewMatrix(matrix);
    vp->ReconstructCamera(matrix, dpos, dup, ddir);
    vec3 pos(dpos[0], dpos[1], dpos[2]);
    vec3 dir(ddir[0], ddir[1], ddir[2]);
    framebufferShader->SetUniform("camPos", pos);
    framebufferShader->SetUniform("camDir", dir);

    return 0;
}

int VolumeOSPRay::LoadData(const Grid *grid)
{
    auto      p = GetParams();
    OSPVolume volume;

    if (p->GetValueLong("osp_test_volume", false))
        volume = _loadVolumeTest(grid);
    else if (p->GetValueLong("osp_force_regular", false))
        volume = _loadVolumeRegular(grid);
    else if (dynamic_cast<const RegularGrid *>(grid))
        volume = _loadVolumeRegular(grid);
    else if (dynamic_cast<const StructuredGrid *>(grid))
        volume = _loadVolumeStructured(grid);
    else if (dynamic_cast<const UnstructuredGrid *>(grid))
        volume = _loadVolumeUnstructured(grid);
    else
        volume = 0, VAssert(!"Unknown grid type");

    if (volume == nullptr) return -1;

    ospRelease(_ospVolumeModel);
    _ospVolumeModel = ospNewVolumetricModel(volume);
    ospRelease(volume);

    ospRelease(_ospTF);
    _ospTF = ospNewTransferFunction("piecewiseLinear");
    float   temp[] = {0, 0, 0, 1, 1, 1};
    OSPData data = VOSP::NewCopiedData(temp, OSP_VEC3F, 2);
    ospCommit(data);
    ospSetObject(_ospTF, "color", data);
    ospRelease(data);
    float temp2[] = {0, 1};
    data = VOSP::NewCopiedData(temp2, OSP_FLOAT, 2);
    ospCommit(data);
    ospSetObject(_ospTF, "opacity", data);
    ospRelease(data);
    ospSetVec2f(_ospTF, "valueRange", -FLT_MAX, 0);    // These initial values are necessary to work around bugs.
    ospCommit(_ospTF);
    ospSetObject(_ospVolumeModel, "transferFunction", _ospTF);
    ospCommit(_ospVolumeModel);

    if (_isIso()) {
        ospRelease(_ospIso);
        ospRelease(_ospIsoModel);

        _ospIso = ospNewGeometry("isosurface");
        ospSetObject(_ospIso, "volume", _ospVolumeModel);
        ospSetFloat(_ospIso, "isovalue", 0);
        ospCommit(_ospIso);

        _ospIsoModel = ospNewGeometricModel(_ospIso);
        ospSetVec4f(_ospIsoModel, "color", 1, 1, 1, 1);
        ospCommit(_ospIsoModel);
    }

    OSPGeometry clip = ospNewGeometry("box");
    data = ospNewSharedData(&_clipBox, OSP_BOX3F, 1);
    ospCommit(data);
    ospSetObject(clip, "box", data);
    ospRelease(data);
    ospCommit(clip);

    OSPGeometricModel clipModel = ospNewGeometricModel(clip);
    ospSetBool(clipModel, "invertNormals", true);
    ospRelease(clip);
    ospCommit(clipModel);

    OSPGroup group = ospNewGroup();
    if (_isIso())
        ospSetObjectAsData(group, "geometry", OSP_GEOMETRIC_MODEL, _ospIsoModel);
    else
        ospSetObjectAsData(group, "volume", OSP_VOLUMETRIC_MODEL, _ospVolumeModel);
    if (p->GetValueLong("osp_enable_clipping", false)) ospSetObjectAsData(group, "clippingGeometry", OSP_GEOMETRIC_MODEL, clipModel);
    //    ospSetObjectAsData(group, "geometry", OSP_GEOMETRIC_MODEL, clipModel);
    ospCommit(group);
    ospRelease(clipModel);

    ospRelease(_ospInstance);
    _ospInstance = ospNewInstance(group);
    ospCommit(_ospInstance);
    ospRelease(group);

    ospSetObjectAsData(_ospWorld, "instance", OSP_INSTANCE, _ospInstance);
    ospCommit(_ospWorld);

    if (dynamic_cast<const RegularGrid *>(grid))
        _ospSampleRateScalar = 1.f;
    else if (VOSP::IsVersionAtLeast(2, 5))
        _ospSampleRateScalar = 1.f;
    else
        _ospSampleRateScalar = _guessSamplingRateScalar(grid);

    return 0;
}

ShaderProgram *VolumeOSPRay::GetShader() const { return _glManager->shaderManager->GetShader("VolumeDVR"); }

void VolumeOSPRay::SetUniforms(const ShaderProgram *s) const {}

float VolumeOSPRay::GuestimateFastModeSpeedupFactor() const { return 5; }

void VolumeOSPRay::GetFinalBlendingMode(int *src, int *dst)
{
    *src = GL_SRC_ALPHA;
    *dst = GL_ONE_MINUS_SRC_ALPHA;
}

void VolumeOSPRay::_setupRenderer(bool fast)
{
    auto p = GetParams();
    bool usePT = p->GetValueLong("osp_usePT", 0);

    if (!_ospRenderer || _cache.usePT != usePT) {
        ospRelease(_ospRenderer);
        _ospRenderer = ospNewRenderer(usePT ? "pathtracer" : "scivis");
        _cache.usePT = usePT;
    }

    vector<double> bgColor;
    int            spp = p->GetValueLong("osp_spp", 1);
    bool           opaqueBG = p->GetValueLong("osp_opaque_bg", false);
    GetAnnotationParams()->GetBackgroundColor(bgColor);
    ospSetVec4f(_ospRenderer, "backgroundColor", bgColor[0], bgColor[1], bgColor[2], opaqueBG ? 1 : 0);
    ospSetFloat(_ospRenderer, "volumeSamplingRate", _ospSampleRateScalar * (fast ? 0.1 : 1) * p->GetValueDouble(VolumeParams::OSPSampleRateScalar, 1));
    ospSetInt(_ospRenderer, "pixelSamples", fast ? 1 : spp);
    ospSetInt(_ospRenderer, "aoSamples", 0);

    ViewpointParams *vp = GetViewpointParams();
    auto             viewport = GLManager::GetViewport();
    glm::ivec2       fbSize(viewport[2], viewport[3]);

    if (fast) {
        ospRemoveParam(_ospRenderer, "map_maxDepth");
    } else {
        auto   mm = _glManager->matrixManager;
        double matrix[16], dpos[3], dup[3], ddir[3];
        vp->GetModelViewMatrix(matrix);
        vp->ReconstructCamera(matrix, dpos, dup, ddir);
        vec3       pos(dpos[0], dpos[1], dpos[2]);
        vec3       up(dup[0], dup[1], dup[2]);
        vec3       dir(ddir[0], ddir[1], ddir[2]);
        OSPTexture depthMap = VOSP::OSPDepthFromGLPerspective(glm::degrees(mm->FOV), mm->Aspect, mm->Near, mm->Far, dir, up, _depthData.data(), fbSize.x, fbSize.y);
        ospSetObject(_ospRenderer, "map_maxDepth", depthMap);
        ospRelease(depthMap);
    }

    if (fast || !p->GetValueLong("osp_useBackplate", false)) {
        ospRemoveParam(_ospRenderer, "map_backplate");
    } else {
        OSPTexture backplate = ospNewTexture("texture2d");
        ospSetInt(backplate, "format", OSP_TEXTURE_R32F);
        ospSetInt(backplate, "filter", OSP_TEXTURE_FILTER_NEAREST);

        OSPData data = VOSP::NewCopiedData(_backplateData.data(), OSP_VEC3UC, fbSize.x, fbSize.y);
        ospCommit(data);
        ospSetObject(backplate, "data", data);
        ospRelease(data);

        ospSetObject(_ospRenderer, "map_backplate", backplate);
        ospRelease(backplate);
    }

    ospCommit(_ospRenderer);
}

void VolumeOSPRay::_setupCamera()
{
    ViewpointParams *vp = GetViewpointParams();
    auto             viewport = GLManager::GetViewport();
    glm::ivec2       fbSize(viewport[2], viewport[3]);

    double matrix[16], dpos[3], dup[3], ddir[3];
    vp->GetModelViewMatrix(matrix);
    vp->ReconstructCamera(matrix, dpos, dup, ddir);
    vec3 pos(dpos[0], dpos[1], dpos[2]);
    vec3 up(dup[0], dup[1], dup[2]);
    vec3 dir(ddir[0], ddir[1], ddir[2]);

    //    printf("Camera Pos = (%f, %f, %f)\n", pos.x, pos.y, pos.z);
    //    printf("Camera Up  = (%f, %f, %f)\n", up.x,  up.y,  up.z);
    //    printf("Camera Dir = (%f, %f, %f)\n", dir.x, dir.y, dir.z);

    ospSetFloat(_ospCamera, "aspect", fbSize.x / (float)fbSize.y);
    ospSetFloat(_ospCamera, "fovy", vp->GetFOV());
    ospSetParam(_ospCamera, "position", OSP_VEC3F, &pos);
    ospSetParam(_ospCamera, "direction", OSP_VEC3F, &dir);
    ospSetParam(_ospCamera, "up", OSP_VEC3F, &up);
    ospCommit(_ospCamera);

    float ambientIntensity = GetParams()->GetValueDouble(VolumeParams::OSPAmbientLightIntensity, 0.2);
    ospSetFloat(_ospLightAmbient, "intensity", ambientIntensity);
    ospCommit(_ospLightAmbient);

    float dirIntensity = GetParams()->GetValueDouble(VolumeParams::OSPDirectionalLightIntensity, 1);
    ospSetFloat(_ospLightDistant, "intensity", dirIntensity);
    ospSetVec3f(_ospLightDistant, "direction", dir.x, dir.y, dir.z);
    ospSetVec3f(_ospLightDistant, "color", 1, 1, 1);
    ospCommit(_ospLightDistant);
}

void VolumeOSPRay::_setupIso()
{
    auto *         p = GetParams();
    vector<double> isoValuesD = p->GetIsoValues();
    vector<float>  isoValues(isoValuesD.begin(), isoValuesD.end());

    OSPData data = VOSP::NewCopiedData(isoValues.data(), OSP_FLOAT, isoValues.size());
    ospSetObject(_ospIso, "isovalue", data);
    ospRelease(data);
    ospCommit(_ospIso);

    vector<float> c = p->GetConstantColor();
    c.push_back(p->GetMapperFunc(p->GetVariableName())->getOpacityScale());
    ospSetVec4f(_ospIsoModel, "color", c[0], c[1], c[2], c[3]);
    ospCommit(_ospIsoModel);
}

void VolumeOSPRay::_loadTF()
{
    if (!_ospTF) {
        fprintf(stderr, "Warning: _ospTF = NULL\n");
        return;
    }
    auto p = GetParams();
    auto vtf = p->GetMapperFunc(p->GetVariableName());
    auto range = vtf->getMinMaxMapValue();

    float *LUT = new float[4 * 256];
    vtf->makeLut(LUT);
    for (int i = 0; i < 256; i++) { LUT[4 * i + 3] = powf(LUT[4 * i + 3], 2); }

    vec3 * cLUT = new vec3[256];
    float *oLUT = new float[256];
    for (int i = 0; i < 256; i++) {
        cLUT[i].r = LUT[4 * i + 0];
        cLUT[i].g = LUT[4 * i + 1];
        cLUT[i].b = LUT[4 * i + 2];
        oLUT[i] = LUT[4 * i + 3];
    }

    OSPData data = VOSP::NewCopiedData(oLUT, OSP_FLOAT, 256);
    ospCommit(data);
    ospSetObject(_ospTF, "opacity", data);
    ospRelease(data);
    data = VOSP::NewCopiedData(cLUT, OSP_VEC3F, 256);
    ospCommit(data);
    ospSetObject(_ospTF, "color", data);
    ospRelease(data);
    ospSetVec2f(_ospTF, "valueRange", range[0], range[1]);
    ospCommit(_ospTF);

    delete[] LUT;
    delete[] cLUT;
    delete[] oLUT;
}

void VolumeOSPRay::_applyTransform()
{
    auto       p = GetParams();
    vec3       translate = D2V(p->GetTransform()->GetTranslations());
    vec3       rotate = D2V(p->GetTransform()->GetRotations());
    vec3       scale = D2V(p->GetTransform()->GetScales());
    vec3       origin = D2V(p->GetTransform()->GetOrigin());
    Transform *datasetTransform = GetDatasetTransform();
    vec3       datasetScales = D2V(datasetTransform->GetScales());
    vec3       datasetRotation = D2V(datasetTransform->GetRotations());
    vec3       datasetTranslation = D2V(datasetTransform->GetTranslations());
    vec3       datasetOrigin = D2V(datasetTransform->GetOrigin());

    mat4 m(1.f);

    m = glm::translate(m, datasetTranslation);
    m = glm::translate(m, datasetOrigin);
    m = glm::rotate(m, glm::radians(datasetRotation.x), vec3(1, 0, 0));
    m = glm::rotate(m, glm::radians(datasetRotation.y), vec3(0, 1, 0));
    m = glm::rotate(m, glm::radians(datasetRotation.z), vec3(0, 0, 1));
    m = glm::scale(m, datasetScales);
    m = glm::translate(m, -datasetOrigin);

    m = glm::scale(m, 1.f / datasetScales);
    m = glm::translate(m, translate);
    m = glm::translate(m, origin);
    m = glm::rotate(m, glm::radians((float)rotate[0]), vec3(1, 0, 0));
    m = glm::rotate(m, glm::radians((float)rotate[1]), vec3(0, 1, 0));
    m = glm::rotate(m, glm::radians((float)rotate[2]), vec3(0, 0, 1));
    m = glm::scale(m, scale);
    m = glm::translate(m, -origin);
    m = glm::scale(m, datasetScales);

    struct {
        vec3 x, y, z, o;
    } affine;

    affine.x = m * vec4(vec3(1, 0, 0), 0);
    affine.y = m * vec4(vec3(0, 1, 0), 0);
    affine.z = m * vec4(vec3(0, 0, 1), 0);
    affine.o = m * vec4(vec3(0, 0, 0), 1);

    ospSetParam(_ospInstance, "xfm", OSP_AFFINE3F, &affine);
    ospCommit(_ospInstance);
    ospCommit(_ospWorld);
}

void VolumeOSPRay::_copyDepth()
{
    GLint viewport[4] = {0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    int width = viewport[2];
    int height = viewport[3];

    _depthData.resize(width * height);
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, (GLvoid *)_depthData.data());
}

void VolumeOSPRay::_copyBackplate()
{
    GLint viewport[4] = {0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    int width = viewport[2];
    int height = viewport[3];

    _backplateData.resize(width * height * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid *)_backplateData.data());
}

float VolumeOSPRay::_guessSamplingRateScalar(const Grid *grid) const
{
    std::vector<double> dataMinExtD, dataMaxExtD;
    grid->GetUserExtents(dataMinExtD, dataMaxExtD);
    vec3  dataMinExt(dataMinExtD[0], dataMinExtD[1], dataMinExtD[2]);
    vec3  dataMaxExt(dataMaxExtD[0], dataMaxExtD[1], dataMaxExtD[2]);
    vec3  lens = dataMaxExt - dataMinExt;
    float longest = max(lens.x, max(lens.y, lens.z));

    // I was going to try to come up with a continuous equation but I'm
    // not sure how the original sample rate is determined so I'm just
    // going off the following two samples of resonable performance vs
    // quality:
    //
    // 4E7 = 0.001
    // 3E6 = 0.1

    return longest < 3E6f ? glm::mix(1.f, 0.1f, longest / 3E6f) : glm::mix(0.1f, 0.001f, (longest - 3E6f) / (4.05E7f - 3E6f));
}

OSPVolume VolumeOSPRay::_loadVolumeRegular(const Grid *grid)
{
    const auto          dims = grid->GetDimensions();
    const size_t        nVerts = dims[0] * dims[1] * dims[2];
    std::vector<double> dataMinExtD, dataMaxExtD;
    grid->GetUserExtents(dataMinExtD, dataMaxExtD);
    vec3  dataMinExt(dataMinExtD[0], dataMinExtD[1], dataMinExtD[2]);
    vec3  dataMaxExt(dataMaxExtD[0], dataMaxExtD[1], dataMaxExtD[2]);
    vec3  dimsf(dims[0], dims[1], dims[2]);
    vec3  gridSpacing = (dataMaxExt - dataMinExt) / (dimsf - 1.f);
    float missingValue = grid->HasMissingData() ? grid->GetMissingValue() : NAN;

    float *fdata = new float[nVerts];
    if (!fdata) {
        Wasp::MyBase::SetErrMsg("Could not allocate enough RAM to load data");
        return nullptr;
    }
    auto dataIt = grid->cbegin();
    for (size_t i = 0; i < nVerts; ++i, ++dataIt) { fdata[i] = *dataIt == missingValue ? NAN : *dataIt; }

    OSPData data = VOSP::NewCopiedData(fdata, OSP_FLOAT, dims[0], dims[1], dims[2]);
    ospCommit(data);
    delete[] fdata;

    OSPVolume volume = ospNewVolume("structuredRegular");

    ospSetObject(volume, "data", data);
    ospRelease(data);

    ospSetVec3f(volume, "gridOrigin", dataMinExt.x, dataMinExt.y, dataMinExt.z);
    ospSetVec3f(volume, "gridSpacing", gridSpacing.x, gridSpacing.y, gridSpacing.z);

    ospCommit(volume);
    return volume;
}

VolumeOSPRay::WindingOrder VolumeOSPRay::getWindingOrderRespectToZ(const vec3 &a, const vec3 &b, const vec3 &c) { return glm::cross(b - a, c - b).z > 0 ? CCW : CW; }

VolumeOSPRay::WindingOrder VolumeOSPRay::getWindingOrderTetra(const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d)
{
    vec3  n = glm::cross(b - a, c - a);
    float s = glm::dot(d - a, n);
    if (fabsf(s) < FLT_EPSILON) return INVALID;
    return s < 0 ? CCW : CW;
}

const char *VolumeOSPRay::windingOrderToString(WindingOrder o) { return o == CCW ? "CCW" : o == CW ? "CW" : "INVALID"; }

bool VolumeOSPRay::isQuadCoPlanar(const vec3 &a, const vec3 &b, const vec3 &c, const vec3 &d)
{
    vec3  n = glm::normalize(glm::cross(b - a, c - a));
    float s = glm::dot(d - a, n);
    return fabsf(s) <= FLT_EPSILON;
}

OSPVolume VolumeOSPRay::_loadVolumeStructured(const Grid *grid)
{
    const auto   dims = grid->GetDimensions();
    const size_t nVerts = dims[0] * dims[1] * dims[2];
    float        missingValue = grid->HasMissingData() ? grid->GetMissingValue() : NAN;

    Progress::Start("Loading Grid", 2, false);
    float *vdata = new float[nVerts];
    auto   dataIt = grid->cbegin();
    for (size_t i = 0; i < nVerts; ++i, ++dataIt) vdata[i] = *dataIt == missingValue ? NAN : *dataIt;
    Progress::Update(1);

    float *cdata = new float[nVerts * 3];
    auto   coord = grid->ConstCoordBegin();
    for (size_t i = 0; i < nVerts; ++i, ++coord) {
        cdata[i * 3] = (*coord)[0];
        cdata[i * 3 + 1] = (*coord)[1];
        cdata[i * 3 + 2] = (*coord)[2];
    }
    Progress::Finish();

    int xd = dims[0];
    int yd = dims[1];
    int zd = dims[2];
    int cxd = xd - 1;
    int cyd = yd - 1;
    int czd = zd - 1;

    if (cxd * cyd * czd == 0) {
        MyBase::SetErrMsg("Volume rendering a flat grid not supported with this method");
        return nullptr;
    }

    // "indexPrefixed" is broken

    typedef struct {
        union {
            struct {
                unsigned int i0, i1, i2, i3, i4, i5, i6, i7;
            };
            unsigned int i[8];
        };
    } Cell;
    vector<Cell> indices(cxd * cyd * czd);

#define I(x, y, z) (unsigned int)((z)*yd * xd + (y)*xd + (x))

    Progress::Start("Convert Grid", czd, true);
    for (int z = 0; z < czd; z++) {
        Progress::Update(z);
        if (Progress::Cancelled()) {
            delete[] vdata;
            delete[] cdata;
            return nullptr;
        }
        for (int y = 0; y < cyd; y++) {
            for (int x = 0; x < cxd; x++) {
                indices[z * cyd * cxd + y * cxd + x] = {{{
                    I(x, y, z),
                    I(x + 1, y, z),
                    I(x + 1, y + 1, z),
                    I(x, y + 1, z),
                    I(x, y, z + 1),
                    I(x + 1, y, z + 1),
                    I(x + 1, y + 1, z + 1),
                    I(x, y + 1, z + 1),
                }}};
            }
        }
    }
    Progress::Finish();
#undef I

    const int maxCells = INT_MAX;
    const int nCells = min(maxCells, czd * cyd * cxd);
    vec3 *    coords = (vec3 *)cdata;

    //    for (int i = 0; i < 8; i++) {
    //        int ci = indices[nCells-1].i[i];
    //        printf("cells[%i][%i] = (%f, %f, %f)\n", nCells-1, i, coords[ci].x, coords[ci].y, coords[ci].z);
    //    }

    //    for (int i = 0; i < 4; i++) {
    //        int bi = indices[nCells-1].i[i];
    //        int ti = indices[nCells-1].i[i+4];
    //        auto &b = coords[bi];
    //        auto &t = coords[ti];
    //        float zdiff = (t-b).z;
    //        printf("ZDiff[%i] = %f%s\n", i, zdiff, zdiff < FLT_EPSILON ? " (< EPSILON)" : "");
    //    }

    //    bool decompose = GetActiveParams()->GetValueLong("osp_decompose", false);
    const bool decompose = true;
    unsigned   nDecomposed = 0;
    unsigned   nDiscarded = 0;

    Progress::Start("Preprocess Grid", nCells, true);
    vector<unsigned int>  startIndex;
    vector<unsigned char> cellType;
    for (int i = 0; i < nCells; i++) {
        Progress::Update(i);
        if (Progress::Cancelled()) {
            delete[] vdata;
            delete[] cdata;
            return nullptr;
        }
        bool discard = false;
        for (int j = 0; j < 4; j++) {
            if (fabsf((coords[indices[i].i[j + 4]] - coords[indices[i].i[j]]).z) < FLT_EPSILON) {
                discard = true;
                break;
            }
        }
        if (discard) {
            nDiscarded++;
            continue;
        }

        if (decompose
            && (!isQuadCoPlanar(coords[indices[i].i0], coords[indices[i].i1], coords[indices[i].i2], coords[indices[i].i3])
                || !isQuadCoPlanar(coords[indices[i].i4], coords[indices[i].i5], coords[indices[i].i6], coords[indices[i].i7]))) {
            Cell c = indices[i];
            Cell w1 = {{{c.i0, c.i1, c.i3, c.i4, c.i5, c.i7, 0, 0}}};
            Cell w2 = {{{c.i1, c.i2, c.i3, c.i5, c.i6, c.i7, 0, 0}}};

            if (CW == getWindingOrderRespectToZ(coords[w1.i0], coords[w1.i1], coords[w1.i2])) {
                std::swap(w1.i1, w1.i2);
                std::swap(w1.i4, w1.i5);
            }
            if (CW == getWindingOrderRespectToZ(coords[w2.i0], coords[w2.i1], coords[w2.i2])) {
                std::swap(w2.i1, w2.i2);
                std::swap(w2.i4, w2.i5);
            }

            startIndex.push_back(indices.size() * 8);
            cellType.push_back(OSP_WEDGE);
            indices.push_back(w1);
            startIndex.push_back(indices.size() * 8);
            cellType.push_back(OSP_WEDGE);
            indices.push_back(w2);
            nDecomposed++;
            continue;
        }

        startIndex.push_back(i * 8);
        cellType.push_back(OSP_HEXAHEDRON);
    }
    Progress::Finish();

    //    printf("Discarded %i cells\n", nDiscarded);
    //    printf("Decomposed %i cells\n", nDecomposed);
    VAssert(cellType.size() == startIndex.size());

    OSPVolume volume = ospNewVolume("unstructured");
    OSPData   data;
    Progress::Start("Copy data to OSPRay", 5, false);

    data = VOSP::NewCopiedData(vdata, OSP_FLOAT, nVerts);
    ospCommit(data);
    ospSetObject(volume, "vertex.data", data);
    ospRelease(data);
    delete[] vdata;
    Progress::Update(1);

    data = VOSP::NewCopiedData(cdata, OSP_VEC3F, nVerts);
    ospCommit(data);
    ospSetObject(volume, "vertex.position", data);
    ospRelease(data);
    delete[] cdata;
    Progress::Update(2);

    // Was cxd*cyd*czd*8
    // need to potentially modify when replacing non-parallelpiped cells.
    data = VOSP::NewCopiedData(indices.data(), OSP_UINT, indices.size() * 8);
    ospCommit(data);
    ospSetObject(volume, "index", data);
    ospRelease(data);
    indices.clear();
    Progress::Update(3);

    data = VOSP::NewCopiedData(startIndex.data(), OSP_UINT, startIndex.size());
    ospCommit(data);
    ospSetObject(volume, "cell.index", data);
    ospRelease(data);
    startIndex.clear();
    Progress::Update(4);

    data = VOSP::NewCopiedData(cellType.data(), OSP_UCHAR, cellType.size());
    ospCommit(data);
    ospSetObject(volume, "cell.type", data);
    ospRelease(data);
    cellType.clear();
    Progress::Update(5);
    Progress::Finish();

    Progress::StartIndefinite("Commit OSPRay");
    ospSetBool(volume, "hexIterative", true);
    ospCommit(volume);
    Progress::Finish();

    return volume;
}

OSPVolume VolumeOSPRay::_loadVolumeUnstructured(const Grid *grid)
{
    const auto           nodeDims = grid->GetDimensions();
    size_t               nodeDim = std::count_if(nodeDims.begin(), nodeDims.end(), [](size_t v) { return v != 1; });
    const size_t         nVerts = nodeDims[0] * nodeDims[1];
    const vector<size_t> cellDims = grid->GetCellDimensions();
    const size_t         nCells = cellDims[0] * cellDims[1];
    VAssert(nodeDim == 2 && cellDims.size() == 2);

    float  missingValue = grid->HasMissingData() ? grid->GetMissingValue() : NAN;
    size_t maxNodes = grid->GetMaxVertexPerCell();
    //    size_t *nodes = (size_t*)alloca(sizeof(size_t) * maxNodes * nodeDim);
    std::vector<Size_tArr3> nodes(maxNodes * nodeDim);

    Progress::Start("Loading Grid Data", 2, false);
    float *vdata = new float[nVerts];
    auto   dataIt = grid->cbegin();
    for (size_t i = 0; i < nVerts; ++i, ++dataIt) vdata[i] = *dataIt == missingValue ? NAN : *dataIt;
    Progress::Update(1);

    float *cdata = new float[nVerts * 3];
    auto   coord = grid->ConstCoordBegin();
    for (size_t i = 0; i < nVerts; ++i, ++coord) {
        cdata[i * 3] = (*coord)[0];
        cdata[i * 3 + 1] = (*coord)[1];
        cdata[i * 3 + 2] = (*coord)[2];
    }
    Progress::Update(2);
    Progress::Finish();

    vector<unsigned int>  cellIndices;
    vector<unsigned int>  cellStarts;
    vector<unsigned char> cellTypes;

    long added[32] = {0};
    long skipped[32] = {0};

    //    int maxCells = std::min((int)nCells, (int)GetActiveParams()->GetValueLong("osp_max_cells", 1));

    auto cellIt = grid->ConstCellBegin();
    Progress::Start("Loading Grid", nCells, true);
    for (size_t cellCounter = 0; cellCounter < nCells; ++cellIt, ++cellCounter) {
        Progress::Update(cellCounter);
        if (Progress::Cancelled()) {
            delete[] vdata;
            delete[] cdata;
            return nullptr;
        }
        const vector<size_t> &cell = *cellIt;
        grid->GetCellNodes(cell.data(), nodes);
        int numNodes = nodes.size();

        if (numNodes == 4) {
            cellStarts.push_back(cellIndices.size());
            cellTypes.push_back(OSP_TETRAHEDRON);
            added[numNodes]++;

            for (int i = 0; i < 4; i++) cellIndices.push_back(nodes[i][0] + nodes[i][1] * nodeDims[0]);
        } else if (numNodes == 6) {
            cellStarts.push_back(cellIndices.size());
            cellTypes.push_back(OSP_WEDGE);
            added[numNodes]++;

            for (int i = 0; i < 6; i++) cellIndices.push_back(nodes[i][0] + nodes[i][1] * nodeDims[0]);
        } else if (numNodes == 12) {    // Hexagonal Prism
#define add(i) cellIndices.push_back(nodes[i][0] + nodes[i][1] * nodeDims[0]);
            for (int i = 0; i < 4; i++) {
                cellStarts.push_back(cellIndices.size());
                cellTypes.push_back(OSP_WEDGE);
                add(0);
                add(i + 1);
                add(i + 2);
                add(6);
                add(6 + i + 1);
                add(6 + i + 2);
            }
#undef add
            added[numNodes]++;
        } else {
            skipped[numNodes]++;
        }

        //        if (cellCounter >= maxCells-1) {
        //            printf("WARNING BREAKING EARLY\n");
        //            break;
        //        }
    }
    Progress::Finish();
    //    printf("done\n");

    //    long totalAdded=0, totalSkipped=0;
    //    for (int i = 0; i < 32; i++) {
    //        if (added[i] > 0) printf("\tAdded[%i] = %li\n", i, added[i]);
    //        if (skipped[i] > 0) printf("\tSkipped[%i] = %li\n", i, skipped[i]);
    //        totalAdded += added[i];
    //        totalSkipped += skipped[i];
    //    }
    //    printf("\tTotal Added = %li\n", totalAdded);
    //    printf("\tTotal Skipped = %li\n", totalSkipped);
    //    printf("# Coords = %li\n", nVerts);

    vec3 *       coords = (vec3 *)cdata;
    vector<bool> erase(cellStarts.size(), false);

    Progress::Start("Preprocessing Cells", cellStarts.size(), true);
    for (unsigned i = 0; i < cellStarts.size(); i++) {
        Progress::Update(i);
        if (Progress::Cancelled()) {
            delete[] vdata;
            delete[] cdata;
            return nullptr;
        }
        unsigned start = cellStarts[i];
        unsigned type = cellTypes[i];

        if (type == OSP_WEDGE) {
            if (CW == getWindingOrderRespectToZ(coords[cellIndices[start + 0]], coords[cellIndices[start + 1]], coords[cellIndices[start + 2]])) {
                std::swap(cellIndices[start + 1], cellIndices[start + 2]);
                std::swap(cellIndices[start + 1 + 3], cellIndices[start + 2 + 3]);
            }
            WindingOrder w1 = getWindingOrderTetra(coords[cellIndices[start + 0]], coords[cellIndices[start + 1]], coords[cellIndices[start + 2]], coords[cellIndices[start + 3]]);
            WindingOrder w2 = getWindingOrderTetra(coords[cellIndices[start + 0]], coords[cellIndices[start + 1]], coords[cellIndices[start + 2]], coords[cellIndices[start + 4]]);
            WindingOrder w3 = getWindingOrderTetra(coords[cellIndices[start + 0]], coords[cellIndices[start + 1]], coords[cellIndices[start + 2]], coords[cellIndices[start + 5]]);
            if (w1 == INVALID || w2 == INVALID || w3 == INVALID) {
                erase[i] = true;
                continue;
            }
        } else if (type == OSP_TETRAHEDRON) {
            WindingOrder w = getWindingOrderTetra(coords[cellIndices[start + 0]], coords[cellIndices[start + 1]], coords[cellIndices[start + 2]], coords[cellIndices[start + 3]]);
            if (w == CW) std::swap(cellIndices[start + 1], cellIndices[start + 2]);
            if (w == INVALID) {
                erase[i] = true;
                continue;
            }
            VAssert(CCW == getWindingOrderTetra(coords[cellIndices[start + 0]], coords[cellIndices[start + 1]], coords[cellIndices[start + 2]], coords[cellIndices[start + 3]]));
            //            if (CCW != GetWindingOrderTetra(coords[cellIndices[start+0]], coords[cellIndices[start+1]], coords[cellIndices[start+2]], coords[cellIndices[start+3]])) {
            //                PrintVec3(coords[cellIndices[start+0]]);
            //                PrintVec3(coords[cellIndices[start+1]]);
            //                PrintVec3(coords[cellIndices[start+2]]);
            //                PrintVec3(coords[cellIndices[start+3]]);
            //                printf("Winding = %s\n", to_string(GetWindingOrderTetra(coords[cellIndices[start+0]], coords[cellIndices[start+1]], coords[cellIndices[start+2]],
            //                coords[cellIndices[start+3]]))); assert(0);
            //            }
        }
    }
    Progress::Finish();
    {
        vector<unsigned int>  cellStartsNew;
        vector<unsigned char> cellTypesNew;

        for (int i = 0; i < cellStarts.size(); i++) {
            if (!erase[i]) {
                cellStartsNew.push_back(cellStarts[i]);
                cellTypesNew.push_back(cellTypes[i]);
            }
        }

        cellStarts = cellStartsNew;
        cellTypes = cellTypesNew;
    }

    //    Progress::Finish();
    //    printf("done\n");

    //    int testCell = std::min(cellStarts.size()-1, (size_t)GetActiveParams()->GetValueLong("osp_test_cells", 0));
    //    int testCell = 0;
    //    if (testCell >= 0) {
    //        int testCellNodes = cellTypes[testCell] == OSP_TETRAHEDRON ? 4 : 6;
    //        printf("Test Cell[%i].nodes = %i\n", testCell, testCellNodes);
    //        vec3 testCellCoords[testCellNodes];
    //        for (int i = 0; i < testCellNodes; i++) {
    //            int idx = cellIndices[cellStarts[testCell]+i];
    //            testCellCoords[i] = coords[idx];
    //            printf("\tCells[%i].vert[%i] = coords[%i] = (%f, %f, %f)\n", testCell, i, idx, coords[idx].x, coords[idx].y, coords[idx].z);
    //        }
    //        printf("\tWinding bottom = %s\n", to_string(GetWindingOrderRespectToZ(testCellCoords[0], testCellCoords[1], testCellCoords[2])));
    //        if (testCellNodes == 6)
    //            printf("\tWinding top = %s\n", to_string(GetWindingOrderRespectToZ(testCellCoords[3], testCellCoords[4], testCellCoords[5])));
    //    }

    Progress::Start("Sanity Checks", 5, false);
    for (auto i : cellIndices) VAssert(i < nVerts);
    Progress::Update(1);
    for (auto i : cellStarts) VAssert(i < cellIndices.size());
    Progress::Update(2);
    for (auto i : cellTypes) VAssert(i == OSP_WEDGE || i == OSP_TETRAHEDRON);
    Progress::Update(3);
    VAssert(cellStarts[cellStarts.size() - 1] + (cellTypes[cellTypes.size() - 1] == OSP_WEDGE ? 6 : 4) <= cellIndices.size());
    Progress::Update(4);
    VAssert(cellStarts.size() == cellTypes.size());
    Progress::Update(5);
    Progress::Finish();

    Progress::Start("Copy data to OSPRay", 5, false);
    OSPVolume volume = ospNewVolume("unstructured");
    OSPData   data;

    data = VOSP::NewCopiedData(vdata, OSP_FLOAT, nVerts);
    ospCommit(data);
    ospSetObject(volume, "vertex.data", data);
    ospRelease(data);
    delete[] vdata;
    Progress::Update(1);

    data = VOSP::NewCopiedData(cdata, OSP_VEC3F, nVerts);
    ospCommit(data);
    ospSetObject(volume, "vertex.position", data);
    ospRelease(data);
    delete[] cdata;
    Progress::Update(2);

    data = VOSP::NewCopiedData(cellIndices.data(), OSP_UINT, cellIndices.size());
    ospCommit(data);
    ospSetObject(volume, "index", data);
    ospRelease(data);
    cellIndices.clear();
    Progress::Update(3);

    data = VOSP::NewCopiedData(cellStarts.data(), OSP_UINT, cellStarts.size());
    ospCommit(data);
    ospSetObject(volume, "cell.index", data);
    ospRelease(data);
    cellStarts.clear();
    Progress::Update(4);

    data = VOSP::NewCopiedData(cellTypes.data(), OSP_UCHAR, cellTypes.size());
    ospCommit(data);
    ospSetObject(volume, "cell.type", data);
    ospRelease(data);
    cellTypes.clear();
    Progress::Update(5);
    Progress::Finish();

    Progress::Start("Commit OSPRay", 1, false);
    ospCommit(volume);
    Progress::Finish();

    return volume;
}

OSPVolume VolumeOSPRay::_loadVolumeTest(const Grid *grid)
{
    std::vector<double> dataMinExtD, dataMaxExtD;
    grid->GetUserExtents(dataMinExtD, dataMaxExtD);
    vec3 dataMinExt(dataMinExtD[0], dataMinExtD[1], dataMinExtD[2]);
    vec3 dataMaxExt(dataMaxExtD[0], dataMaxExtD[1], dataMaxExtD[2]);
    int  nVerts = 8;

    vector<float> values(nVerts);
    for (int i = 0; i < nVerts; i++) values[i] = 0;

    vector<vec3> coords(nVerts);
    float        s = 1;
    vec3         l = s * dataMinExt;
    vec3         h = s * dataMaxExt;
    coords[0] = vec3(0, 0, 0);
    coords[1] = vec3(1, 0, 0);
    coords[2] = vec3(1, 1, 0);
    coords[3] = vec3(0, 1, 0);
    coords[4] = vec3(0, 0, 1);
    coords[5] = vec3(1, 0, 1);
    coords[6] = vec3(1, 1, 1);
    coords[7] = vec3(0, 1, 1);

    for (int i = 0; i < 8; i++) coords[i] = coords[i] * (h - l) + l;

    OSPVolume volume = ospNewVolume("unstructured");
    OSPData   data;

    data = VOSP::NewCopiedData(values.data(), OSP_FLOAT, nVerts);
    ospCommit(data);
    ospSetObject(volume, "vertex.data", data);
    ospRelease(data);

    data = VOSP::NewCopiedData(coords.data(), OSP_VEC3F, nVerts);
    ospCommit(data);
    ospSetObject(volume, "vertex.position", data);
    ospRelease(data);

    vector<unsigned int>  cellIndices;
    vector<unsigned int>  cellStarts;
    vector<unsigned char> cellTypes;

#if 1
    cellStarts.push_back(cellIndices.size());
    cellTypes.push_back(OSP_HEXAHEDRON);
    cellIndices.push_back(0);
    cellIndices.push_back(1);
    cellIndices.push_back(2);
    cellIndices.push_back(3);
    cellIndices.push_back(4);
    cellIndices.push_back(5);
    cellIndices.push_back(6);
    cellIndices.push_back(7);
#else
    cellStarts.push_back(cellIndices.size());
    cellTypes.push_back(OSP_WEDGE);
    cellIndices.push_back(0);
    cellIndices.push_back(1);
    cellIndices.push_back(3);
    cellIndices.push_back(4);
    cellIndices.push_back(5);
    cellIndices.push_back(7);
    cellStarts.push_back(cellIndices.size());
    cellTypes.push_back(OSP_WEDGE);
    cellIndices.push_back(1);
    cellIndices.push_back(2);
    cellIndices.push_back(3);
    cellIndices.push_back(5);
    cellIndices.push_back(6);
    cellIndices.push_back(7);
    cellStarts.push_back(cellIndices.size());
    cellTypes.push_back(OSP_TETRAHEDRON);
    cellIndices.push_back(0);
    cellIndices.push_back(1);
    cellIndices.push_back(2);
    cellIndices.push_back(3);
#endif

    data = VOSP::NewCopiedData(cellIndices.data(), OSP_UINT, cellIndices.size());
    ospCommit(data);
    ospSetObject(volume, "index", data);
    ospRelease(data);

    data = VOSP::NewCopiedData(cellStarts.data(), OSP_UINT, cellStarts.size());
    ospCommit(data);
    ospSetObject(volume, "cell.index", data);
    ospRelease(data);

    data = VOSP::NewCopiedData(cellTypes.data(), OSP_UCHAR, cellTypes.size());
    ospCommit(data);
    ospSetObject(volume, "cell.type", data);
    ospRelease(data);

    ospSetBool(volume, "hexIterative", true);
    ospCommit(volume);
    return volume;
}

/*
#include <vapor/LegacyGL.h>
void VolumeRenderer::drawTestCellOutline()
{
    if (!GetActiveParams()->GetValueLong("osp_test_volume", false))
    return;

    LegacyGL *lgl = _glManager->legacy;
    lgl->Begin(GL_LINE_STRIP);
    lgl->Color3f(1, 0, 0);
    lgl->Vertex(testCell[0]);
    lgl->Color3f(0, 1, 0);
    lgl->Vertex(testCell[1]);
    lgl->Color3f(0, 0, 1);
    lgl->Vertex(testCell[2]);
    lgl->Color3f(0, 1, 1);
    lgl->Vertex(testCell[3]);
    lgl->Color3f(1, 0, 0);
    lgl->Vertex(testCell[0]);
    lgl->Color3f(1, 1, 1);
    lgl->Vertex(testCell[4]);
//    lgl->Color3f(0, 1, 0);
    lgl->Vertex(testCell[5]);
//    lgl->Color3f(0, 0, 1);
    lgl->Vertex(testCell[6]);
//    lgl->Color3f(0, 1, 1);
    lgl->Vertex(testCell[7]);
//    lgl->Color3f(1, 0, 0);
    lgl->Vertex(testCell[4]);
    lgl->End();
    lgl->Begin(GL_LINES);
    lgl->Color3f(0, 1, 0);
    lgl->Vertex(testCell[1]);
    lgl->Vertex(testCell[5]);
    lgl->Color3f(0, 0, 1);
    lgl->Vertex(testCell[2]);
    lgl->Vertex(testCell[6]);
    lgl->Color3f(0, 1, 1);
    lgl->Vertex(testCell[3]);
    lgl->Vertex(testCell[7]);
    lgl->End();

    if (!GetActiveParams()->GetValueLong("osp_draw_test_cell_faces", false))
        return;

    lgl->Color3f(1, 1, 1);
    lgl->EnableLighting();
    ViewpointParams* vp = _paramsMgr->GetViewpointParams(_winName);
    double matrix[16], dpos[3], dup[3], ddir[3];
    vp->GetModelViewMatrix(matrix);
    vp->ReconstructCamera(matrix, dpos, dup, ddir);
    vec3 dir(ddir[0], ddir[1], ddir[2]);
    lgl->LightDirectionfv((float*)&dir);
    glDepthFunc(GL_LESS);

#define FACE(a, b, c) { vec3 n = glm::normalize(glm::cross(testCell[b]-testCell[a], testCell[c]-testCell[a])); lgl->Normal3fv((float*)&n); lgl->Vertex(testCell[a]); lgl->Vertex(testCell[b]);
lgl->Vertex(testCell[c]); } #define QUAD(a, b, c, d) { FACE(a, b, d); FACE(d, b, c); } lgl->Begin(GL_TRIANGLES); QUAD(0, 1, 2, 3); QUAD(4, 5, 6, 7); QUAD(0, 1, 5, 4); QUAD(1, 2, 6, 5); QUAD(2, 3, 7,
6); QUAD(3, 0, 4, 7); lgl->End(); #undef FACE

    glDepthFunc(GL_ALWAYS);
    lgl->DisableLighting();
}
*/

// static VolumeAlgorithmRegistrar<VolumeOSPRayIso> registrationIso;
