//*************************************************************
//
// Copyright (C) 2017
// University Corporation for Atmospheric Research
// All Rights Reserved
//
// ************************************************************

// Specify the barbhead width compared with barb diameter:
#define BARB_HEAD_FACTOR 3.0

// Specify how long the barb tube is, in front of where the
// barbhead is attached:
#define BARB_LENGTH_FACTOR 0.9

// Specify the maximum cylinder radius in proportion to the
// hypotenuse of the domain, divided by 4 (the maximum barb thicness param)
// I.E. if the user sets the thickness to 4 (the maximum), then the
// barbs will have max radius = 4 * BARB_RADIUS_TO_HYPOTENUSE * hypotenuse
//#define BARB_RADIUS_TO_HYPOTENUSE .00625
#define BARB_RADIUS_TO_HYPOTENUSE .001875

// Specify the maximum barb length in proportion to the
// hypotenuse of the domain, divided by 4 (the maximum barb length param)
// I.E. if the user sets the length to 4 (the maximum), then the longest
// barb will have max length = 4 * BARB_LENGTH_TO_HYPOTENUSE * hypotenuse
#define BARB_LENGTH_TO_HYPOTENUSE .0625

#include <vapor/glutil.h>    // Must be included first!!!
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <limits>

#ifndef WIN32
    #include <unistd.h>
#endif

#include <vapor/DataStatus.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/BarbRenderer.h>
#include <vapor/BarbParams.h>

#include <vapor/regionparams.h>
#include <vapor/ViewpointParams.h>
#include <vapor/MyBase.h>
#include <vapor/errorcodes.h>
#include <vapor/DataMgr.h>
#define INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
#include <vapor/LegacyVectorMath.h>
#include "vapor/LegacyGL.h"
#include "vapor/GLManager.h"
#include <glm/gtc/type_ptr.hpp>

#define X    0
#define Y    1
#define Z    2
#define XMIN 0
#define YMIN 1
#define ZMIN 2
#define XMAX 3
#define YMAX 4
#define ZMAX 5

int counter = 0;

using namespace VAPoR;
using namespace Wasp;

static RendererRegistrar<BarbRenderer> registrar(BarbRenderer::GetClassType(), BarbParams::GetClassType());

BarbRenderer::BarbRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, BarbParams::GetClassType(), BarbRenderer::GetClassType(), instName, dataMgr)
{
    _fieldVariables.clear();
    _vectorScaleFactor = .2;
    _maxThickness = .2;
    _maxValue = 0.f;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
BarbRenderer::~BarbRenderer() {}

std::string BarbRenderer::_getColorbarVariableName() const
{
    RenderParams *rParams = GetActiveParams();
    return rParams->GetColorMapVariableName();
}

// Totally unnecessary?
//
int BarbRenderer::_initializeGL()
{
    //_initialized = true;
    return (0);
}

void BarbRenderer::_saveCacheParams()
{
    BarbParams *p = dynamic_cast<BarbParams *>(GetActiveParams());
    VAssert(p);
    _cacheParams.fieldVarNames = p->GetFieldVariableNames();
    _cacheParams.heightVarName = p->GetHeightVariableName();
    _cacheParams.colorVarName = p->GetColorMapVariableName();
    _cacheParams.ts = p->GetCurrentTimestep();
    _cacheParams.level = p->GetRefinementLevel();
    _cacheParams.lod = p->GetCompressionLevel();
    _cacheParams.grid = p->GetGrid();
    _cacheParams.needToRecalc = p->GetNeedToRecalculateScales();
    p->GetBox()->GetExtents(_cacheParams.boxMin, _cacheParams.boxMax);
}

bool BarbRenderer::_isCacheDirty() const
{
    BarbParams *p = dynamic_cast<BarbParams *>(GetActiveParams());
    VAssert(p);
    if (_cacheParams.fieldVarNames != p->GetFieldVariableNames()) return true;
    if (_cacheParams.heightVarName != p->GetHeightVariableName()) return true;
    if (_cacheParams.colorVarName != p->GetColorMapVariableName()) return true;
    if (_cacheParams.ts != p->GetCurrentTimestep()) return true;
    if (_cacheParams.level != p->GetRefinementLevel()) return true;
    if (_cacheParams.lod != p->GetCompressionLevel()) return true;
    if (_cacheParams.grid != p->GetGrid()) return true;
    if (_cacheParams.needToRecalc != p->GetNeedToRecalculateScales()) return true;

    vector<double> min, max, contourValues;
    p->GetBox()->GetExtents(min, max);

    if (_cacheParams.boxMin != min) return true;
    if (_cacheParams.boxMax != max) return true;

    return false;
}

void BarbRenderer::_recalculateScales(
    // std::vector<string> varnames,
    std::vector<VAPoR::Grid *> &varData, int ts)
{
    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    VAssert(bParams);

    vector<string> varnames = bParams->GetFieldVariableNames();
    bool           recalculateScales = bParams->GetNeedToRecalculateScales();

    if (varnames != _fieldVariables || recalculateScales) {
        //_setDefaultLengthAndThicknessScales(ts, varnames, bParams);
        _setDefaultLengthAndThicknessScales(ts, varData, bParams);
        _fieldVariables = varnames;
        bParams->SetNeedToRecalculateScales(false);
    }
}

int BarbRenderer::_getVectorVarGrids(int ts, int refLevel, int lod, std::vector<double> minExts, std::vector<double> maxExts, std::vector<VAPoR::Grid *> &varData)
{
    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    VAssert(bParams);

    vector<string> varnames = bParams->GetFieldVariableNames();
    if (!VariableExists(ts, varnames, refLevel, lod, true)) {
        SetErrMsg("One or more selected field variables does not exist");
        return -1;
    }

    // Get grids for our vector variables
    //
    int rc = DataMgrUtils::GetGrids(_dataMgr, ts, varnames, minExts, maxExts, true, &refLevel, &lod, varData);

    return rc;
}

void BarbRenderer::_getGridRequirements(int &ts, int &refLevel, int &lod, std::vector<double> &minExts, std::vector<double> &maxExts) const
{
    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    VAssert(bParams);

    ts = bParams->GetCurrentTimestep();

    refLevel = bParams->GetRefinementLevel();
    lod = bParams->GetCompressionLevel();
    bParams->GetBox()->GetExtents(minExts, maxExts);
}

int BarbRenderer::_getVarGrid(int ts, int refLevel, int lod, string varName, std::vector<double> minExts, std::vector<double> maxExts, std::vector<VAPoR::Grid *> &varData)
{
    Grid *sg = NULL;
    varData.push_back(sg);

    if (!varName.empty()) {
        int rc = DataMgrUtils::GetGrids(_dataMgr, ts, varName, minExts, maxExts, true, &refLevel, &lod, &sg);
        if (rc < 0) {
            for (int i = 0; i < varData.size(); i++) {
                if (varData[i]) _dataMgr->UnlockGrid(varData[i]);
            }
            return (rc);
        }
        varData[varData.size() - 1] = sg;
    }

    return 0;
}

int BarbRenderer::_paintGL(bool)
{
    if (_isCacheDirty()) {
        if (_generateBarbs() < 0) return -1;
        _saveCacheParams();
    }

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    _setUpLightingAndColor();

    float clut[1024];
    bool  doColorMapping = _makeCLUT(clut);
    auto  crange = GetActiveParams()->GetMapperFunc(GetActiveParams()->GetColorMapVariableName())->getMinMaxMapValue();
    for (auto b : _barbCache) _drawBarb(b, doColorMapping, clut, crange.data());

    _glManager->legacy->DisableLighting();

    return 0;
}

int BarbRenderer::_generateBarbs()
{
    int rc;

    // Set up the variable data required, while determining data
    // extents to use in rendering
    //
    vector<Grid *> varData;

    int            ts, refLevel, lod;
    vector<double> minExts, maxExts;
    _getGridRequirements(ts, refLevel, lod, minExts, maxExts);

    // Get vector variables
    rc = _getVectorVarGrids(ts, refLevel, lod, minExts, maxExts, varData);
    if (rc < 0) {
        SetErrMsg("One or more selected field variables does not exist");
        return -1;
    }

    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    VAssert(bParams);

    // Get height variable
    string heightVar = bParams->GetHeightVariableName();
    rc = _getVarGrid(ts, refLevel, lod, heightVar, minExts, maxExts, varData);
    if (rc < 0) {
        SetErrMsg("Height variable does not exist");
        return -1;
    }

    // Get color variable
    string colorVar = bParams->GetColorMapVariableName();
    rc = _getVarGrid(ts, refLevel, lod, colorVar, minExts, maxExts, varData);
    if (rc < 0) {
        SetErrMsg("Color variable does not exist");
        return -1;
    }

    _recalculateScales(varData, ts);

    _barbCache.clear();
    // Render the barbs
    _operateOnGrid(varData);

    // Release the locks on the data
    for (int i = 0; i < varData.size(); i++) {
        if (varData[i]) _dataMgr->UnlockGrid(varData[i]);
    }

    return (rc);
}

float BarbRenderer::_calculateDirVec(const float start[3], const float end[3], float dirVec[3])
{
    vsub(end, start, dirVec);
    float len = vlength(dirVec);
    if (len != 0.f) vscale(dirVec, 1. / len);
    return len;
}

void BarbRenderer::_drawBackOfBarb(const float dirVec[3], const float startVertex[3]) const
{
    // TODO GL
    /*
    glBegin(GL_POLYGON);
    glNormal3fv(dirVec);
    for (int k = 0; k<6; k++){
        glVertex3fv(startVertex+3*k);
    }
    glEnd();
     */
}

void BarbRenderer::_drawCylinderSides(const float nextNormal[3], const float nextVertex[3], const float startNormal[3], const float startVertex[3]) const
{
    LegacyGL *lgl = _glManager->legacy;

    lgl->Begin(GL_TRIANGLE_STRIP);

    for (int i = 0; i < 6; i++) {
        lgl->Normal3fv(nextNormal + 3 * i);
        lgl->Vertex3fv(nextVertex + 3 * i);

        lgl->Normal3fv(startNormal + 3 * i);
        lgl->Vertex3fv(startVertex + 3 * i);
    }
    // repeat first two vertices to close cylinder:

    lgl->Normal3fv(nextNormal);
    lgl->Vertex3fv(nextVertex);

    lgl->Normal3fv(startNormal);
    lgl->Vertex3fv(startVertex);

    lgl->End();
}

void BarbRenderer::_drawBarbHead(const float dirVec[3], const float vertexPoint[3], const float startNormal[3], const float startVertex[3]) const
{
    LegacyGL *lgl = _glManager->legacy;

    // Create a triangle fan from these 6 vertices.
    lgl->Begin(GL_TRIANGLE_FAN);
    lgl->Normal3fv(dirVec);
    lgl->Vertex3fv(vertexPoint);
    for (int i = 0; i < 6; i++) {
        lgl->Normal3fv(startNormal + 3 * i);
        lgl->Vertex3fv(startVertex + 3 * i);
    }
    // Repeat first point to close fan:
    lgl->Normal3fv(startNormal);
    lgl->Vertex3fv(startVertex);
    lgl->End();
}

#ifdef DEBUG
void BarbRenderer::_printBackDiameter(const float startVertex[18]) const
{
    float pointA[3] = {startVertex[0], startVertex[1], startVertex[2]};
    float pointB[3] = {startVertex[3], startVertex[4], startVertex[5]};
    float pointC[3] = {startVertex[6], startVertex[7], startVertex[8]};
    float pointD[3] = {startVertex[9], startVertex[10], startVertex[11]};
    float pointE[3] = {startVertex[12], startVertex[13], startVertex[14]};
    float pointF[3] = {startVertex[15], startVertex[16], startVertex[17]};
    cout << "   " << pointA[0] << "\t\t" << pointB[0] << "\t\t\t" << pointC[0];
    cout << "\t\t" << pointD[0] << "\t\t" << pointE[0] << "\t\t" << pointF[0] << endl;

    cout << "   " << pointA[1] << "\t\t" << pointB[1] << "\t\t" << pointC[1];
    cout << "\t" << pointD[1] << "\t" << pointE[1] << "\t\t" << pointF[1] << endl;

    cout << "   " << pointA[2] << "\t\t" << pointB[2] << "\t\t" << pointC[2];
    cout << "\t\t" << pointD[2] << "\t\t" << pointE[2] << "\t\t" << pointF[2] << endl;
    cout << "Back Diameter " << _calculateLength(pointA, pointD) << endl;
}
#endif

// Issue OpenGL calls to draw a cylinder with orthogonal ends from
// one point to another.  Then put an barb head on the end
//
void BarbRenderer::_drawBarb(const std::vector<Grid *> variableData, const float startPoint_[3], bool doColorMapping, const float clut[1024])
{
    Barb b;
    memcpy(b.startPoint, startPoint_, sizeof(float) * 3);
    b.lengthScalar = ((BarbParams *)GetActiveParams())->GetLengthScale() * _vectorScaleFactor;

    VAssert(variableData.size() == 5);

    bool missing = _defineBarb(variableData, b.startPoint, b.endPoint, &b.value, doColorMapping, clut);

    if (missing) return;

    _barbCache.push_back(b);
}

void BarbRenderer::_setBarbColor(float value, const float clut[1024], double crange[2]) const
{
    float range = crange[1] - crange[0];
    float n = (value - crange[0]) / range;
    n = glm::clamp(n, 0.f, 1.f);
    int i = 255 * n;

    _glManager->legacy->Color4fv(&clut[i * 4]);
}

void BarbRenderer::_drawBarb(Barb b, bool doColorMapping, const float clut[1024], double crange[2])
{
    float *        startPoint = b.startPoint;
    float *        endPoint = b.endPoint;
    MatrixManager *mm = _glManager->matrixManager;
    if (doColorMapping) _setBarbColor(b.value, clut, crange);
    float     newLengthScalar = ((BarbParams *)GetActiveParams())->GetLengthScale() * _vectorScaleFactor;
    glm::vec3 v = glm::make_vec3(b.endPoint) - glm::make_vec3(b.startPoint);
    float     l = glm::length(v) / b.lengthScalar * newLengthScalar;
    glm::vec3 end = glm::make_vec3(b.startPoint) + glm::normalize(v) * l;
    memcpy(b.endPoint, glm::value_ptr(end), sizeof(float) * 3);

    mm->MatrixModeModelView();
    mm->PushMatrix();

    mm->Translate(startPoint[0], startPoint[1], startPoint[2]);

    endPoint[0] -= startPoint[0];
    endPoint[1] -= startPoint[1];
    endPoint[2] -= startPoint[2];

    startPoint[0] = startPoint[1] = startPoint[2] = 0;

    vector<double> scales = _getScales();
    mm->Scale(1.f / scales[0], 1.f / scales[1], 1.f / scales[2]);

    // Constants are needed for cosines and sines, at
    // 60 degree intervals. The barb is really a hexagonal tube,
    // but the shading makes it look round.
    const float sines[6] = {0.f, (float)(sqrt(3.) / 2.), (float)(sqrt(3.) / 2.), 0.f, (float)(-sqrt(3.) / 2.), (float)(-sqrt(3.) / 2.)};
    const float coses[6] = {1.f, 0.5, -0.5, -1., -.5, 0.5};

    float nextPoint[3];
    float vertexPoint[3];
    float headCenter[3];
    float startNormal[18];
    float nextNormal[18];
    float startVertex[18];
    float nextVertex[18];
    float testVec[3];
    float testVec2[3];

    // Calculate an orthonormal frame

    // dirVec is the barb direction
    float dirVec[3];
    float len = _calculateDirVec(startPoint, endPoint, dirVec);

    // Calculate uVec, orthogonal to dirVec
    // Not sure what bVec is.  Up direction?
    float uVec[3], bVec[3];
    vset(testVec, 1., 0., 0.);
    vcross(dirVec, testVec, uVec);
    len = vdot(uVec, uVec);
    if (len == 0.f) {
        vset(testVec, 0., 1., 0.);
        vcross(dirVec, testVec, uVec);
        len = vdot(uVec, uVec);
    }
    vscale(uVec, 1.f / sqrt(len));
    vcross(uVec, dirVec, bVec);

    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    VAssert(bParams);

    float radius = bParams->GetLineThickness() * _maxThickness;

    // calculate 6 points in plane orthog to dirVec, in plane of point
    for (int i = 0; i < 6; i++) {
        // testVec and testVec2 are components of point in plane
        vmult(uVec, coses[i], testVec);
        vmult(bVec, sines[i], testVec2);

        // Calc outward normal as a sideEffect..
        // It is the vector sum of x,y components (norm 1)
        vadd(testVec, testVec2, startNormal + 3 * i);

        // stretch by radius to get current displacement
        vmult(startNormal + 3 * i, radius, startVertex + 3 * i);

        // add to current point
        vadd(startVertex + 3 * i, startPoint, startVertex + 3 * i);
    }

#ifdef DEBUG
    _printBackDiameter(startVertex);
#endif

    _drawBackOfBarb(dirVec, startVertex);

    // The variables are located as follows:
    //		- - - - >
    //	   ^       ^ ^
    // start    next end

    // Calculate nextPoint and vertexPoint, for barbhead
    for (int i = 0; i < 3; i++) {
        nextPoint[i] = (1. - BARB_LENGTH_FACTOR) * startPoint[i] + BARB_LENGTH_FACTOR * endPoint[i];
        // Assume a vertex angle of 45 degrees:
        vertexPoint[i] = nextPoint[i] + dirVec[i] * radius;
        headCenter[i] = nextPoint[i] - dirVec[i] * (BARB_HEAD_FACTOR * radius - radius);
    }

    // calc for endpoints:
    for (int i = 0; i < 6; i++) {
        // testVec and testVec2 are components of point in plane
        vmult(uVec, coses[i], testVec);
        vmult(bVec, sines[i], testVec2);

        // Calc outward normal as a sideEffect..
        // It is the vector sum of x,y components (norm 1)
        vadd(testVec, testVec2, nextNormal + 3 * i);

        // stretch by radius to get current displacement
        vmult(nextNormal + 3 * i, radius, nextVertex + 3 * i);

        // add to current point
        vadd(nextVertex + 3 * i, nextPoint, nextVertex + 3 * i);
    }

    _drawCylinderSides(nextNormal, nextVertex, startNormal, startVertex);

    // Now draw the barb head.  First calc 6 vertices at back of barbhead
    // Reuse startNormal and startVertex vectors
    // calc for endpoints:
    for (int i = 0; i < 6; i++) {
        // testVec and testVec2 are components of point in plane
        // Can reuse them from previous (cylinder end) calculation
        vmult(uVec, coses[i], testVec);
        vmult(bVec, sines[i], testVec2);

        // Calc outward normal as a sideEffect..
        // It is the vector sum of x,y components (norm 1)
        vadd(testVec, testVec2, startNormal + 3 * i);

        // stretch by radius to get current displacement
        vmult(startNormal + 3 * i, BARB_HEAD_FACTOR * radius, startVertex + 3 * i);

        // add to current point
        vadd(startVertex + 3 * i, headCenter, startVertex + 3 * i);

        // Now tilt normals in direction of barb:
        for (int k = 0; k < 3; k++) { startNormal[3 * i + k] = 0.5 * startNormal[3 * i + k] + 0.5 * dirVec[k]; }
    }

    _drawBarbHead(dirVec, vertexPoint, startNormal, startVertex);

    mm->PopMatrix();
}

void BarbRenderer::_setUpLightingAndColor()
{
    LegacyGL *       lgl = _glManager->legacy;
    string           winName = GetVisualizer();    // GetVisualizer is not const :(
    ViewpointParams *vpParams = _paramsMgr->GetViewpointParams(winName);
    int              nLights = vpParams->getNumLights();

    float       fcolor[3];
    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    VAssert(bParams);
    bParams->GetConstantColor(fcolor);
    if (nLights == 0) {
        lgl->DisableLighting();
    } else {
        // All the geometry will get a white specular color:
        float specColor[4];
        specColor[0] = specColor[1] = specColor[2] = 0.8f;
        specColor[3] = 1.f;
        lgl->EnableLighting();
    }
    lgl->Color3fv(fcolor);
}

void BarbRenderer::_reFormatExtents(vector<float> &rakeExts) const
{
    rakeExts.clear();
    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    VAssert(bParams);
    vector<double> rMinExtents, rMaxExtents;

    bParams->GetBox()->GetExtents(rMinExtents, rMaxExtents);

    bool planar = bParams->GetBox()->IsPlanar();

    rakeExts.push_back(rMinExtents[X]);
    rakeExts.push_back(rMinExtents[Y]);
    rakeExts.push_back(planar ? GetDefaultZ(_dataMgr, bParams->GetCurrentTimestep()) : rMinExtents[Z]);
    rakeExts.push_back(rMaxExtents[X]);
    rakeExts.push_back(rMaxExtents[Y]);
    rakeExts.push_back(planar ? GetDefaultZ(_dataMgr, bParams->GetCurrentTimestep()) : rMaxExtents[Z]);
}

void BarbRenderer::_makeRakeGrid(vector<int> &rakeGrid) const
{
    rakeGrid.clear();
    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    VAssert(bParams);
    vector<long> longGrid = bParams->GetGrid();

    rakeGrid.push_back((int)longGrid[X]);
    rakeGrid.push_back((int)longGrid[Y]);
    rakeGrid.push_back((int)longGrid[Z]);
}

float BarbRenderer::_getHeightOffset(Grid *heightVar, float xCoord, float yCoord, bool &missing) const
{
    VAssert(heightVar);
    float missingVal = heightVar->GetMissingValue();
    float offset = heightVar->GetValue(xCoord, yCoord, 0.f);
    if (offset == missingVal) {
        missing = true;
        offset = 0.f;
    }
    return offset;
}

void BarbRenderer::_getDirection(float direction[3], vector<Grid *> variableData, float xCoord, float yCoord, float zCoord, bool &missing) const
{
    for (int dim = 0; dim < 3; dim++) {
        direction[dim] = 0.f;
        if (variableData[dim]) {
            direction[dim] = variableData[dim]->GetValue(xCoord, yCoord, zCoord);

            float missingVal = variableData[dim]->GetMissingValue();
            if (direction[dim] == missingVal) { missing = true; }
        }
    }
}

bool BarbRenderer::_makeCLUT(float clut[1024]) const
{
    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    VAssert(bParams);
    string colorVar = bParams->GetColorMapVariableName();
    bool   doColorMapping = !bParams->UseSingleColor() && !colorVar.empty();

    if (doColorMapping) {
        MapperFunction *tf = 0;
        tf = (MapperFunction *)bParams->GetMapperFunc(colorVar);
        VAssert(tf);
        tf->makeLut(clut);
    }
    return doColorMapping;
}

vector<double> BarbRenderer::_getScales()
{
    string                  myVisName = GetVisualizer();    // Not const :(
    VAPoR::ViewpointParams *vpp = _paramsMgr->GetViewpointParams(myVisName);
    string                  datasetName = GetMyDatasetName();
    Transform *             tDataset = vpp->GetTransform(datasetName);
    Transform *             tRenderer = GetActiveParams()->GetTransform();

    vector<double> scales = tDataset->GetScales();
    vector<double> rendererScales = tRenderer->GetScales();

    scales[0] *= rendererScales[0];
    scales[1] *= rendererScales[1];
    scales[2] *= rendererScales[2];

    return scales;
}

float BarbRenderer::_calculateLength(float start[3], float end[3]) const
{
    float xDist = start[X] - end[X];
    float yDist = start[Y] - end[Y];
    float zDist = start[Z] - end[Z];
    float length = sqrt(xDist * xDist + yDist * yDist + zDist * zDist);
    return length;
}

void BarbRenderer::_makeStartAndEndPoint(float start[3], float end[3], float direction[3])
{
    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    VAssert(bParams);
    float length = bParams->GetLengthScale() * _vectorScaleFactor;

    vector<double> scales = _getScales();

    end[X] = start[X] + scales[X] * direction[X] * length;
    end[Y] = start[Y] + scales[Y] * direction[Y] * length;
    end[Z] = start[Z] + scales[Z] * direction[Z] * length;
}

void BarbRenderer::_getStrides(vector<float> &strides, vector<int> &rakeGrid, vector<float> &rakeExts) const
{
    strides.clear();

    float xStride = (rakeExts[XMAX] - rakeExts[XMIN]) / ((float)rakeGrid[X] + 1);
    strides.push_back(xStride);

    float yStride = (rakeExts[YMAX] - rakeExts[YMIN]) / ((float)rakeGrid[Y] + 1);
    strides.push_back(yStride);

    float zStride = (rakeExts[ZMAX] - rakeExts[ZMIN]) / ((float)rakeGrid[Z] + 1);
    strides.push_back(zStride);
}

bool BarbRenderer::_defineBarb(const std::vector<Grid *> variableData, float start[3], float end[3], float *value, bool doColorMapping, const float clut[1024])
{
    bool missing = false;

    Grid *heightVar = variableData[3];
    if (heightVar) { start[Z] += _getHeightOffset(heightVar, start[X], start[Y], missing); }

    float direction[3] = {0.f, 0.f, 0.f};
    _getDirection(direction, variableData, start[X], start[Y], start[Z], missing);

    _makeStartAndEndPoint(start, end, direction);

    if (doColorMapping) {
        float val = variableData[4]->GetValue(start[X], start[Y], start[Z]);
        *value = val;

        if (val == variableData[4]->GetMissingValue())
            missing = true;
        else {
            missing = _getColorMapping(val, clut);
        }
    }
    return missing;
}

void BarbRenderer::_operateOnGrid(vector<Grid *> variableData, bool drawBarb)
{
    vector<int> rakeGrid;
    _makeRakeGrid(rakeGrid);

    vector<float> rakeExts;
    _reFormatExtents(rakeExts);

    vector<float> strides;
    _getStrides(strides, rakeGrid, rakeExts);

    float clut[1024];
    bool  doColorMapping = _makeCLUT(clut);

    float start[3];
    for (int i = 1; i <= rakeGrid[X]; i++) {
        start[X] = strides[X] * i + rakeExts[X];    // + xStride/2.0;
        for (int j = 1; j <= rakeGrid[Y]; j++) {
            start[Y] = strides[Y] * j + rakeExts[Y];    // + yStride/2.0;
            for (int k = 1; k <= rakeGrid[Z]; k++) {
                start[Z] = strides[Z] * k + rakeExts[Z];    //+ zStride/2.0;

                if (drawBarb) {
                    _drawBarb(variableData, start, doColorMapping, clut);
                } else {
                    _getMagnitudeAtPoint(variableData, start);
                }
            }
        }
    }
    return;
}

void BarbRenderer::_getMagnitudeAtPoint(std::vector<VAPoR::Grid *> variables, float point[3])
{
    VAPoR::Grid *grid;
    double       maxValue = 0.f;
    for (int i = 0; i < 3; i++) {
        grid = variables[i];
        if (grid == NULL)
            continue;
        else {
            double value = grid->GetValue(point[X], point[Y], point[Z]);
            double missingValue = grid->GetMissingValue();

            if (value == missingValue) { continue; }
            value = abs(value);

            if (value > maxValue && value < std::numeric_limits<double>::max() && value > std::numeric_limits<double>::lowest() && !std::isnan(value)) maxValue = value;
        }
    }
    if (maxValue > _maxValue) { _maxValue = maxValue; }
}

bool BarbRenderer::_getColorMapping(float val, const float clut[256 * 4])
{
    bool missing = false;

    MapperFunction *tf = 0;
    BarbParams *    bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    VAssert(bParams);
    string colorVar = bParams->GetColorMapVariableName();
    tf = (MapperFunction *)bParams->GetMapperFunc(colorVar);
    VAssert(tf);

    float mappedColor[4] = {0., 0., 0., 0.};
    // Use the transfer function to map the data:
    int lutIndex = tf->mapFloatToIndex(val);
    for (int i = 0; i < 4; i++) mappedColor[i] = clut[4 * lutIndex + i];
    _glManager->legacy->Color4fv(mappedColor);
    return missing;
}

double BarbRenderer::_getDomainHypotenuse(size_t ts) const
{
    std::vector<int>    axes;
    std::vector<double> minExts, maxExts;
    std::vector<string> varNames;

    BarbParams *p = dynamic_cast<BarbParams *>(GetActiveParams());
    VAssert(p);
    varNames = p->GetFieldVariableNames();

    bool status = DataMgrUtils::GetExtents(_dataMgr, ts, varNames, 0, 0, minExts, maxExts, axes);
    VAssert(status);

    if (varNames[0] == "" && varNames[1] == "" && varNames[2] == "") return 0.0;

    double xLen = maxExts[0] - minExts[0];

    double yLen = maxExts[1] - minExts[1];

    double zLen = 0.0;
    if (minExts.size() > 2) zLen = maxExts[2] - minExts[2];

    double diag = sqrt(xLen * xLen + yLen * yLen + zLen * zLen);
    return diag;
}

void BarbRenderer::_setDefaultLengthAndThicknessScales(size_t ts, const std::vector<VAPoR::Grid *> &varData, const BarbParams *bParams)
{
    VAssert(varData.size() >= 3);

    _maxValue = 0;

    _operateOnGrid(varData, false);

    double hypotenuse = _getDomainHypotenuse(ts);

    if (hypotenuse == 0.f) return;

    _maxThickness = hypotenuse * BARB_RADIUS_TO_HYPOTENUSE;
    _vectorScaleFactor = hypotenuse * BARB_LENGTH_TO_HYPOTENUSE;
    _vectorScaleFactor *= 1.0 / _maxValue;
}
