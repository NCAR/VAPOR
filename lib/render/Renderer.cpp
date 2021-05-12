//************************************************************************
//									*
//			 Copyright (C)  2004				*
//	 University Corporation for Atmospheric Research			*
//			 All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		Renderer.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		September 2004
//
//	Description:	Implements the Renderer class.
//		A pure virtual class that is implemented for each renderer.
//		Methods are called by the Visualizer class as needed.
//
#include <cfloat>
#include <climits>
#include <limits>
#include <iomanip>
#include <sstream>

#include <vapor/glutil.h>    // Must be included first!!!
#include <vapor/Renderer.h>
#include <vapor/DataMgrUtils.h>
#include "vapor/GLManager.h"
#include "vapor/FontManager.h"
#include "vapor/LegacyGL.h"
#include "vapor/TextLabel.h"
#include <glm/gtc/type_ptr.hpp>
#include <vapor/ColorbarRenderer.h>
#include <vapor/VolumeIsoParams.h>

#include <vapor/ViewpointParams.h>

using namespace VAPoR;
const int Renderer::_imgWid = 256;
const int Renderer::_imgHgt = 256;

Renderer::Renderer(const ParamsMgr *pm, string winName, string dataSetName, string paramsType, string classType, string instName, DataMgr *dataMgr)
: RendererBase(pm, winName, dataSetName, paramsType, classType, instName, dataMgr)
{
    // Establish the data sources for the rendering:
    //

    _colorbarTexture = 0;
    _timestep = 0;

    _fontName = "arimo";
}

RendererBase::RendererBase(const ParamsMgr *pm, string winName, string dataSetName, string paramsType, string classType, string instName, DataMgr *dataMgr)
{
    // Establish the data sources for the rendering:
    //
    _paramsMgr = pm;
    _winName = winName;
    _dataSetName = dataSetName;
    _paramsType = paramsType;
    _classType = classType;
    _instName = instName;
    _dataMgr = dataMgr;

    _glManager = nullptr;
    _glInitialized = false;
    _flaggedForDeletion = false;
}
// Destructor
RendererBase::~RendererBase() {}

void RendererBase::FlagForDeletion() { _flaggedForDeletion = true; }

bool RendererBase::IsFlaggedForDeletion() const { return _flaggedForDeletion; }

// Destructor
Renderer::~Renderer()
{
    if (_colorbarTexture) delete _colorbarTexture;
}

int RendererBase::initializeGL(GLManager *glManager)
{
    _glManager = glManager;
    int rc = _initializeGL();
    if (rc < 0) { return (rc); }

    vector<int> status;
    bool        ok = oglStatusOK(status);
    if (!ok) {
        SetErrMsg("OpenGL error : %s", oglGetErrMsg(status).c_str());
        return (-1);
    }

    _glInitialized = true;

    return (0);
}

double Renderer::GetDefaultZ(DataMgr *dataMgr, size_t ts) const
{
    const RenderParams *rParams = GetActiveParams();
    int                 refLevel = rParams->GetRefinementLevel();
    int                 lod = rParams->GetCompressionLevel();

    return DataMgrUtils::Get2DRendererDefaultZ(dataMgr, ts, refLevel, lod);
}

int Renderer::paintGL(bool fast)
{
    const RenderParams *rParams = GetActiveParams();
    MatrixManager *     mm = _glManager->matrixManager;

    if (!rParams->IsEnabled()) return (0);

    _timestep = rParams->GetCurrentTimestep();

    vector<double> translate = rParams->GetTransform()->GetTranslations();
    vector<double> rotate = rParams->GetTransform()->GetRotations();
    vector<double> scale = rParams->GetTransform()->GetScales();
    vector<double> origin = rParams->GetTransform()->GetOrigin();
    VAssert(translate.size() == 3);
    VAssert(rotate.size() == 3);
    VAssert(scale.size() == 3);
    VAssert(origin.size() == 3);

    Transform *    datasetTransform = _paramsMgr->GetViewpointParams(_winName)->GetTransform(_dataSetName);
    vector<double> datasetScales = datasetTransform->GetScales();

    mm->MatrixModeModelView();
    mm->PushMatrix();

    mm->Translate(translate[0], translate[1], translate[2]);

    mm->Scale(1 / datasetScales[0], 1 / datasetScales[1], 1 / datasetScales[2]);

    mm->Translate(origin[0], origin[1], origin[2]);
    mm->Rotate(glm::radians(rotate[0]), 1, 0, 0);
    mm->Rotate(glm::radians(rotate[1]), 0, 1, 0);
    mm->Rotate(glm::radians(rotate[2]), 0, 0, 1);
    mm->Scale(scale[0], scale[1], scale[2]);
    mm->Translate(-origin[0], -origin[1], -origin[2]);

    mm->Scale(datasetScales[0], datasetScales[1], datasetScales[2]);

    int rc = _paintGL(fast);

    mm->PopMatrix();

    if (rc < 0) { return (-1); }

    vector<int> status;
    bool        ok = oglStatusOK(status);
    if (!ok) {
        SetErrMsg("OpenGL error : %s", oglGetErrMsg(status).c_str());
        return (-1);
    }
    return (0);
}

void Renderer::EnableClipToBox(ShaderProgram *shader, float haloFrac) const
{
    shader->Bind();
    VAssert(shader->HasUniform("clippingPlanes"));

    float x0Plane[] = {1.0, 0.0, 0.0, 0.0};
    float x1Plane[] = {-1.0, 0.0, 0.0, 0.0};
    float y0Plane[] = {0.0, 1.0, 0.0, 0.0};
    float y1Plane[] = {0.0, -1.0, 0.0, 0.0};
    float z0Plane[] = {0.0, 0.0, 1.0, 0.0};
    float z1Plane[] = {0.0, 0.0, -1.0, 0.0};    // z largest

    const RenderParams *rParams = GetActiveParams();
    vector<double>      minExts, maxExts;
    rParams->GetBox()->GetExtents(minExts, maxExts);
    VAssert(minExts.size() == maxExts.size());
    VAssert(minExts.size() > 0 && minExts.size() < 4);

    int orientation = rParams->GetBox()->GetOrientation();

    for (int i = 0; i < minExts.size(); i++) {
        double range = maxExts[i] - minExts[i];
        if (fabs(range) <= FLT_EPSILON) range = 1.0;
        float halo = range * haloFrac;
        minExts[i] -= halo;
        maxExts[i] += halo;
    }

    if (minExts.size() == 3 || orientation != Box::YZ) {
        x0Plane[3] = -minExts[0];
        x1Plane[3] = maxExts[0];
        glEnable(GL_CLIP_DISTANCE0);
        glEnable(GL_CLIP_DISTANCE1);
        shader->SetUniform("clippingPlanes[0]", glm::make_vec4(x0Plane));
        shader->SetUniform("clippingPlanes[1]", glm::make_vec4(x1Plane));
    }

    if (minExts.size() == 3 || orientation != Box::XZ) {
        y0Plane[3] = -minExts[1];
        y1Plane[3] = maxExts[1];
        glEnable(GL_CLIP_DISTANCE2);
        glEnable(GL_CLIP_DISTANCE3);
        shader->SetUniform("clippingPlanes[2]", glm::make_vec4(y0Plane));
        shader->SetUniform("clippingPlanes[3]", glm::make_vec4(y1Plane));
    }

    if (minExts.size() == 3 || orientation != Box::XY) {
        z0Plane[3] = -minExts[2];
        z1Plane[3] = maxExts[2];
        glEnable(GL_CLIP_DISTANCE4);
        glEnable(GL_CLIP_DISTANCE5);
        shader->SetUniform("clippingPlanes[4]", glm::make_vec4(z0Plane));
        shader->SetUniform("clippingPlanes[5]", glm::make_vec4(z1Plane));
    }
}

void Renderer::DisableClippingPlanes()
{
    glDisable(GL_CLIP_DISTANCE0);
    glDisable(GL_CLIP_DISTANCE1);
    glDisable(GL_CLIP_DISTANCE2);
    glDisable(GL_CLIP_DISTANCE3);
    glDisable(GL_CLIP_DISTANCE4);
    glDisable(GL_CLIP_DISTANCE5);
}

bool Renderer::VariableExists(size_t ts, std::vector<string> &varnames, int level, int lod, bool zeroOK) const
{
    for (int i = 0; i < varnames.size(); i++) {
        if (zeroOK && (varnames[i] == "<no-variable>" || varnames[i] == "")) { continue; }

        if (!_dataMgr->VariableExists(ts, varnames[i], level, lod)) { return (false); }
    }
    return (true);
}

#ifdef VAPOR3_0_0_ALPHA
void Renderer::buildLocal2DTransform(int dataOrientation, float a[2], float b[2], float *constVal, int mappedDims[3])
{
    mappedDims[2] = dataOrientation;
    mappedDims[0] = (dataOrientation == 0) ? 1 : 0;    // x or y
    mappedDims[1] = (dataOrientation < 2) ? 2 : 1;     // z or y
    const RenderParams *rParams = GetActiveParams();

    const vector<double> &exts = rParams->GetBox()->GetLocalExtents();
    *constVal = exts[dataOrientation];
    // constant terms go to middle
    b[0] = 0.5 * (exts[mappedDims[0]] + exts[3 + mappedDims[0]]);
    b[1] = 0.5 * (exts[mappedDims[1]] + exts[3 + mappedDims[1]]);
    // linear terms send -1,1 to box min,max
    a[0] = b[0] - exts[mappedDims[0]];
    a[1] = b[1] - exts[mappedDims[1]];
}

void Renderer::getLocalContainingRegion(float regMin[3], float regMax[3])
{
    // Determine the smallest axis-aligned cube that contains the rotated box local coordinates.  This is
    // obtained by mapping all 8 corners into the space.

    double transformMatrix[12];
    // Set up to transform from probe (coords [-1,1]) into volume:
    GetActiveParams()->GetBox()->buildLocalCoordTransform(transformMatrix, 0.f, -1);
    const double *sizes = _dataStatus->getFullSizes();

    // Calculate the normal vector to the probe plane:
    double zdir[3] = {0.f, 0.f, 1.f};
    double normEnd[3];    // This will be the unit normal
    double normBeg[3];
    double zeroVec[3] = {0.f, 0.f, 0.f};
    vtransform(zdir, transformMatrix, normEnd);
    vtransform(zeroVec, transformMatrix, normBeg);
    vsub(normEnd, normBeg, normEnd);
    vnormal(normEnd);

    // Start by initializing extents, and variables that will be min,max
    for (int i = 0; i < 3; i++) {
        regMin[i] = FLT_MAX;
        regMax[i] = -FLT_MAX;
    }

    for (int corner = 0; corner < 8; corner++) {
        int    intCoord[3];
        double startVec[3], resultVec[3];
        intCoord[0] = corner % 2;
        intCoord[1] = (corner / 2) % 2;
        intCoord[2] = (corner / 4) % 2;
        for (int i = 0; i < 3; i++) startVec[i] = -1.f + (float)(2.f * intCoord[i]);
        // calculate the mapping of this corner,
        vtransform(startVec, transformMatrix, resultVec);
        // force mapped corner to lie in the local extents
        // and then force box to contain the corner:
        for (int i = 0; i < 3; i++) {
            // force to lie in domain
            if (resultVec[i] < 0.) resultVec[i] = 0.;
            if (resultVec[i] > sizes[i]) resultVec[i] = sizes[i];

            if (resultVec[i] < regMin[i]) regMin[i] = resultVec[i];
            if (resultVec[i] > regMax[i]) regMax[i] = resultVec[i];
        }
    }
    return;
}
#endif

std::string Renderer::_getColorbarVariableName() const
{
    RenderParams *rParams = GetActiveParams();
    return rParams->GetVariableName();
}

void Renderer::renderColorbar()
{
    RenderParams *rParams = GetActiveParams();
    if (!rParams->IsEnabled()) return;

    // If constant color, no valid colorbar
    VolumeIsoParams *vip;
    if ((vip = dynamic_cast<VolumeIsoParams *>(rParams)))
        if (!vip->GetValueLong(VolumeParams::UseColormapVariableTag, 0)) return;

    ColorbarRenderer::Render(_glManager, rParams);
}

//////////////////////////////////////////////////////////////////////////
//
// RendererFactory Class
//
/////////////////////////////////////////////////////////////////////////

RendererFactory *RendererFactory::Instance()
{
    static RendererFactory instance;
    return &instance;
}

void RendererFactory::RegisterFactoryFunction(string myName, string myParamsName, function<Renderer *(const ParamsMgr *, string, string, string, string, DataMgr *)> classFactoryFunction)
{
    // register the class factory function
    _factoryFunctionRegistry[myName] = classFactoryFunction;
    _factoryMapRegistry[myName] = myParamsName;
}

Renderer *RendererFactory::CreateInstance(const ParamsMgr *pm, string winName, string dataSetName, string classType, string instName, DataMgr *dataMgr)
{
    Renderer *instance = NULL;

    // find classType in the registry and call factory method.
    //
    auto it = _factoryFunctionRegistry.find(classType);
    if (it != _factoryFunctionRegistry.end()) { instance = it->second(pm, winName, dataSetName, classType, instName, dataMgr); }

    if (instance != NULL)
        return instance;
    else
        return NULL;
}

string RendererFactory::GetRenderClassFromParamsClass(string paramsClass) const
{
    map<string, string>::const_iterator itr;
    for (itr = _factoryMapRegistry.begin(); itr != _factoryMapRegistry.end(); ++itr) {
        if (itr->second == paramsClass) return (itr->first);
    }
    return ("");
}

string RendererFactory::GetParamsClassFromRenderClass(string renderClass) const
{
    map<string, string>::const_iterator itr;
    for (itr = _factoryMapRegistry.begin(); itr != _factoryMapRegistry.end(); ++itr) {
        if (itr->first == renderClass) return (itr->second);
    }
    return ("");
}

vector<string> RendererFactory::GetFactoryNames() const
{
    vector<string> names;

    map<string, function<Renderer *(const ParamsMgr *, string, string, string, string, DataMgr *)>>::const_iterator itr;

    for (itr = _factoryFunctionRegistry.begin(); itr != _factoryFunctionRegistry.end(); ++itr) { names.push_back(itr->first); }
    return (names);
}

void Renderer::GetClippingPlanes(float planes[24]) const
{
    float x0Plane[4] = {1.0, 0.0, 0.0, 0.0};
    float x1Plane[4] = {-1.0, 0.0, 0.0, 0.0};
    float y0Plane[4] = {0.0, 1.0, 0.0, 0.0};
    float y1Plane[4] = {0.0, -1.0, 0.0, 0.0};
    float z0Plane[4] = {0.0, 0.0, 1.0, 0.0};
    float z1Plane[4] = {0.0, 0.0, -1.0, 0.0};

    const RenderParams *rParams = GetActiveParams();
    std::vector<double> minExts, maxExts;
    rParams->GetBox()->GetExtents(minExts, maxExts);
    VAssert(minExts.size() == maxExts.size());
    VAssert(minExts.size() == 2 || minExts.size() == 3);

    x0Plane[3] = float(-minExts[0]);
    x1Plane[3] = float(maxExts[0]);
    y0Plane[3] = float(-minExts[1]);
    y1Plane[3] = float(maxExts[1]);
    if (minExts.size() == 3)    // Fill normal Z extents
    {
        z0Plane[3] = float(-minExts[2]);
        z1Plane[3] = float(maxExts[2]);
    } else    // Fill a thin layer around DefaultZ
    {
        const auto dfz = this->GetDefaultZ(_dataMgr, rParams->GetCurrentTimestep());
        const auto z1 = dfz * 1.0001;
        const auto z2 = dfz * 0.9999;
        z0Plane[3] = -std::min(z1, z2);
        z1Plane[3] = std::max(z1, z2);
    }

    size_t planeSize = sizeof(x0Plane);
    std::memcpy(planes, x0Plane, planeSize);
    std::memcpy(planes + 4, x1Plane, planeSize);
    std::memcpy(planes + 8, y0Plane, planeSize);
    std::memcpy(planes + 12, y1Plane, planeSize);
    std::memcpy(planes + 16, z0Plane, planeSize);
    std::memcpy(planes + 20, z1Plane, planeSize);
}

RendererFactory::RendererFactory() {}
RendererFactory::RendererFactory(const RendererFactory &) {}
RendererFactory &RendererFactory::operator=(const RendererFactory &) { return *this; }
