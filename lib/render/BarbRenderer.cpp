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
// hypotenuse of the domain, divided by 100 (the maximum barb thicness param)
// I.E. if the user sets the thickness to 100 (the maximum), then the
// barbs will have radius = 100 * BARB_RADIUS_TO_HYPOTENUSE * hypotenuse
#define BARB_RADIUS_TO_HYPOTENUSE .000125

// Specify the maximum barb length in proportion to the
// hypotenuse of the domain, divided by 100 (the maximum barb length param)
// I.E. if the user sets the length to 100 (the maximum), then the longest
// barb will have length = 100 * BARB_LENGTH_TO_HYPOTENUSE * hypotenuse
#define BARB_LENGTH_TO_HYPOTENUSE .00125

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
#include <vapor/Visualizer.h>

#include <vapor/regionparams.h>
#include <vapor/ViewpointParams.h>
#include <vapor/MyBase.h>
#include <vapor/errorcodes.h>
#include <vapor/DataMgr.h>

#define X    0
#define Y    1
#define Z    2
#define XMIN 0
#define YMIN 1
#define ZMIN 2
#define XMAX 3
#define YMAX 4
#define ZMAX 5

using namespace VAPoR;
using namespace Wasp;

static RendererRegistrar<BarbRenderer> registrar(BarbRenderer::GetClassType(), BarbParams::GetClassType());

BarbRenderer::BarbRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, BarbParams::GetClassType(), BarbRenderer::GetClassType(), instName, dataMgr)
{
    _drawList = 0;
    _fieldVariables.clear();
    _vectorScaleFactor = .2;
    _maxThickness = .2;
    _maxValue = 0.f;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
BarbRenderer::~BarbRenderer() {}

// Totally unnecessary?
//
int BarbRenderer::_initializeGL()
{
    //_initialized = true;
    _drawList = glGenLists(1);
    return (0);
}

void BarbRenderer::_saveCacheParams()
{
    BarbParams *p = dynamic_cast<BarbParams *>(GetActiveParams());
    assert(p);
    _cacheParams.fieldVarNames = p->GetFieldVariableNames();
    _cacheParams.heightVarName = p->GetHeightVariableName();
    _cacheParams.colorVarName = p->GetColorMapVariableName();
    _cacheParams.ts = p->GetCurrentTimestep();
    _cacheParams.level = p->GetRefinementLevel();
    _cacheParams.lod = p->GetCompressionLevel();
    _cacheParams.useSingleColor = p->UseSingleColor();
    _cacheParams.lineThickness = p->GetLineThickness();
    _cacheParams.lengthScale = p->GetLengthScale();
    _cacheParams.grid = p->GetGrid();
    _cacheParams.needToRecalc = p->GetNeedToRecalculateScales();
    p->GetConstantColor(_cacheParams.constantColor);
    p->GetBox()->GetExtents(_cacheParams.boxMin, _cacheParams.boxMax);

    if (_cacheParams.useSingleColor) return;

    MapperFunction *tf = p->GetMapperFunc(_cacheParams.colorVarName);
    _cacheParams.opacity = tf->getOpacityScale();
    _cacheParams.minMapValue = tf->getMinMapValue();
    _cacheParams.maxMapValue = tf->getMaxMapValue();
    for (int i = 0; i < 10; i++) {
        float point = _cacheParams.minMapValue + i / 10.f * (_cacheParams.maxMapValue - _cacheParams.minMapValue);
        tf->rgbValue(point, _cacheParams.colorSamples[i]);
        _cacheParams.alphaSamples[i] = tf->getOpacityValueData(point);
    }
}

bool BarbRenderer::_isCacheDirty() const
{
    BarbParams *p = dynamic_cast<BarbParams *>(GetActiveParams());
    assert(p);
    if (_cacheParams.fieldVarNames != p->GetFieldVariableNames()) return true;
    if (_cacheParams.heightVarName != p->GetHeightVariableName()) return true;
    if (_cacheParams.colorVarName != p->GetColorMapVariableName()) return true;
    if (_cacheParams.ts != p->GetCurrentTimestep()) return true;
    if (_cacheParams.level != p->GetRefinementLevel()) return true;
    if (_cacheParams.lod != p->GetCompressionLevel()) return true;
    if (_cacheParams.useSingleColor != p->UseSingleColor()) return true;
    if (_cacheParams.lineThickness != p->GetLineThickness()) return true;
    if (_cacheParams.lengthScale != p->GetLengthScale()) return true;
    if (_cacheParams.grid != p->GetGrid()) return true;
    if (_cacheParams.needToRecalc != p->GetNeedToRecalculateScales()) return true;

    vector<double> min, max, contourValues;
    p->GetBox()->GetExtents(min, max);

    if (_cacheParams.boxMin != min) return true;
    if (_cacheParams.boxMax != max) return true;

    float constantColor[3];
    p->GetConstantColor(constantColor);
    if (memcmp(_cacheParams.constantColor, constantColor, sizeof(constantColor))) return true;

    if (_cacheParams.useSingleColor) return false;

    MapperFunction *tf = p->GetMapperFunc(_cacheParams.colorVarName);
    if (_cacheParams.opacity != tf->getOpacityScale()) return true;
    if (_cacheParams.minMapValue != tf->getMinMapValue()) return true;
    if (_cacheParams.maxMapValue != tf->getMaxMapValue()) return true;

    for (int i = 0; i < 10; i++) {
        float point = _cacheParams.minMapValue + i / 10.f * (_cacheParams.maxMapValue - _cacheParams.minMapValue);
        float color[3];
        tf->rgbValue(point, color);
        if (_cacheParams.colorSamples[i][0] != color[0]) return true;
        if (_cacheParams.colorSamples[i][1] != color[1]) return true;
        if (_cacheParams.colorSamples[i][2] != color[2]) return true;
        if (_cacheParams.alphaSamples[i] != tf->getOpacityValueData(point)) return true;
    }

    return false;
}

void BarbRenderer::_recalculateScales(
    // std::vector<string> varnames,
    std::vector<VAPoR::Grid *> &varData, int ts)
{
    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    assert(bParams);

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
    assert(bParams);

    vector<string> varnames = bParams->GetFieldVariableNames();
    if (!VariableExists(ts, varnames, refLevel, lod, true)) {
        SetErrMsg("One or more selected field variables does not exist");
        return -1;
        // glEndList();
    }

    // Get grids for our vector variables
    //
    int rc = DataMgrUtils::GetGrids(_dataMgr, ts, varnames, minExts, maxExts, true, &refLevel, &lod, varData);

    return rc;
}

void BarbRenderer::_getGridRequirements(int &ts, int &refLevel, int &lod, std::vector<double> &minExts, std::vector<double> &maxExts) const
{
    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    assert(bParams);

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
            glEndList();
            return (rc);
        }
        varData[varData.size() - 1] = sg;
    }

    return 0;
}

int BarbRenderer::_paintGL()
{
    int rc = 0;
    if (!_isCacheDirty()) {
        glCallList(_drawList);
        return 0;
    }
    _saveCacheParams();
    glNewList(_drawList, GL_COMPILE_AND_EXECUTE);

    // Set up the variable data required, while determining data
    // extents to use in rendering
    //
    vector<Grid *> varData;

    int            ts, refLevel, lod;
    vector<double> minExts, maxExts;
    _getGridRequirements(ts, refLevel, lod, minExts, maxExts);

    // Get vector variables
    _getVectorVarGrids(ts, refLevel, lod, minExts, maxExts, varData);
    if (rc < 0) {
        glEndList();
        return (rc);
    }

    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    assert(bParams);

    // Get height variable
    string heightVar = bParams->GetHeightVariableName();
    _getVarGrid(ts, refLevel, lod, heightVar, minExts, maxExts, varData);
    if (rc < 0) return rc;

    // Get color variable
    string colorVar = bParams->GetColorMapVariableName();
    _getVarGrid(ts, refLevel, lod, colorVar, minExts, maxExts, varData);
    if (rc < 0) return rc;

    _recalculateScales(varData, ts);

    _setUpLightingAndColor();

    // Render the barbs
    _operateOnGrid(varData);

    // Release the locks on the data
    for (int i = 0; i < varData.size(); i++) {
        if (varData[i]) _dataMgr->UnlockGrid(varData[i]);
    }

    glEndList();
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
    glBegin(GL_POLYGON);
    glNormal3fv(dirVec);
    for (int k = 0; k < 6; k++) { glVertex3fv(startVertex + 3 * k); }
    glEnd();
}

void BarbRenderer::_drawCylinderSides(const float nextNormal[3], const float nextVertex[3], const float startNormal[3], const float startVertex[3]) const
{
    glBegin(GL_TRIANGLE_STRIP);

    for (int i = 0; i < 6; i++) {
        glNormal3fv(nextNormal + 3 * i);
        glVertex3fv(nextVertex + 3 * i);

        glNormal3fv(startNormal + 3 * i);
        glVertex3fv(startVertex + 3 * i);
    }
    // repeat first two vertices to close cylinder:

    glNormal3fv(nextNormal);
    glVertex3fv(nextVertex);

    glNormal3fv(startNormal);
    glVertex3fv(startVertex);

    glEnd();
}

void BarbRenderer::_drawBarbHead(const float dirVec[3], const float vertexPoint[3], const float startNormal[3], const float startVertex[3]) const
{
    // Create a triangle fan from these 6 vertices.
    glBegin(GL_TRIANGLE_FAN);
    glNormal3fv(dirVec);
    glVertex3fv(vertexPoint);
    for (int i = 0; i < 6; i++) {
        glNormal3fv(startNormal + 3 * i);
        glVertex3fv(startVertex + 3 * i);
    }
    // Repeat first point to close fan:
    glNormal3fv(startNormal);
    glVertex3fv(startVertex);
    glEnd();
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
void BarbRenderer::_drawBarb(const std::vector<Grid *> variableData, float startPoint[3], bool doColorMapping, float clut[1024])
{
    assert(variableData.size() == 5);

    float endPoint[3];
    bool  missing = _defineBarb(variableData, startPoint, endPoint, doColorMapping, clut);

    if (missing) return;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    vector<double> scales = _getScales();
    glScalef(1.f / scales[0], 1.f / scales[1], 1.f / scales[2]);

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
    assert(bParams);

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

    glPopMatrix();
}

void BarbRenderer::_setUpLightingAndColor()
{
    string           winName = GetVisualizer();    // GetVisualizer is not const :(
    ViewpointParams *vpParams = _paramsMgr->GetViewpointParams(winName);
    int              nLights = vpParams->getNumLights();

    float       fcolor[3];
    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    assert(bParams);
    bParams->GetConstantColor(fcolor);

    if (nLights == 0) {
        glDisable(GL_LIGHTING);
    } else {
        glShadeModel(GL_SMOOTH);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, fcolor);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, vpParams->getExponent());
        // All the geometry will get a white specular color:
        float specColor[4];
        specColor[0] = specColor[1] = specColor[2] = 0.8f;
        specColor[3] = 1.f;
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specColor);
        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
    }
    glColor3fv(fcolor);
}

void BarbRenderer::_reFormatExtents(vector<float> &rakeExts) const
{
    rakeExts.clear();
    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    assert(bParams);
    vector<double> rMinExtents, rMaxExtents;
    bParams->GetBox()->GetExtents(rMinExtents, rMaxExtents);

    rakeExts.push_back(rMinExtents[X]);
    rakeExts.push_back(rMinExtents[Y]);
    rakeExts.push_back(rMinExtents[Z]);
    rakeExts.push_back(rMaxExtents[X]);
    rakeExts.push_back(rMaxExtents[Y]);
    rakeExts.push_back(rMaxExtents[Z]);
}

void BarbRenderer::_makeRakeGrid(vector<int> &rakeGrid) const
{
    rakeGrid.clear();
    BarbParams *bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    assert(bParams);
    vector<long> longGrid = bParams->GetGrid();

    rakeGrid.push_back((int)longGrid[X]);
    rakeGrid.push_back((int)longGrid[Y]);
    rakeGrid.push_back((int)longGrid[Z]);
}

float BarbRenderer::_getHeightOffset(Grid *heightVar, float xCoord, float yCoord, bool &missing) const
{
    assert(heightVar);
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
    assert(bParams);
    string colorVar = bParams->GetColorMapVariableName();
    bool   doColorMapping = !bParams->UseSingleColor() && !colorVar.empty();

    if (doColorMapping) {
        MapperFunction *tf = 0;
        tf = (MapperFunction *)bParams->GetMapperFunc(colorVar);
        assert(tf);
        tf->makeLut(clut);
    }
    return doColorMapping;
}

vector<double> BarbRenderer::_getScales()
{
    string                  myVisName = GetVisualizer();    // Not const :(
    VAPoR::ViewpointParams *vpp = _paramsMgr->GetViewpointParams(myVisName);
    string                  datasetName = GetMyDatasetName();
    Transform *             t = vpp->GetTransform(datasetName);

    vector<double> scales{3, 1.0};
    if (t != NULL) scales = t->GetScales();
    ;

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
    assert(bParams);
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

bool BarbRenderer::_defineBarb(std::vector<Grid *> variableData, float start[3], float end[3], bool doColorMapping, float clut[1024])
{
    bool missing = false;

    Grid *heightVar = variableData[3];
    if (heightVar) { start[Z] += _getHeightOffset(heightVar, start[X], start[Y], missing); }

    float direction[3] = {0.f, 0.f, 0.f};
    _getDirection(direction, variableData, start[X], start[Y], start[Z], missing);

    _makeStartAndEndPoint(start, end, direction);

    if (doColorMapping) {
        float val = variableData[4]->GetValue(start[X], start[Y], start[Z]);

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

    float start[3];    //, end[3];
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

            value = abs(value);

            if (value > maxValue && value < std::numeric_limits<double>::max() && value > std::numeric_limits<double>::lowest() && value != missingValue && !isnan(value)) maxValue = value;
        }
    }
    if (maxValue > _maxValue) _maxValue = maxValue;
}

bool BarbRenderer::_getColorMapping(float val, float clut[256 * 4])
{
    bool missing = false;

    MapperFunction *tf = 0;
    BarbParams *    bParams = dynamic_cast<BarbParams *>(GetActiveParams());
    assert(bParams);
    string colorVar = bParams->GetColorMapVariableName();
    tf = (MapperFunction *)bParams->GetMapperFunc(colorVar);
    assert(tf);

    float mappedColor[4] = {0., 0., 0., 0.};
    // Use the transfer function to map the data:
    int lutIndex = tf->mapFloatToIndex(val);
    for (int i = 0; i < 4; i++) mappedColor[i] = clut[4 * lutIndex + i];
    glColor4fv(mappedColor);
    return missing;
}

double BarbRenderer::_getMaxAtBarbLocations(VAPoR::Grid *grid) const
{
    vector<double> minExts, maxExts;
    BarbParams *   p = dynamic_cast<BarbParams *>(GetActiveParams());
    assert(p);
    p->GetBox()->GetExtents(minExts, maxExts);

    vector<long> rakeGrid = p->GetGrid();

    float stride[3] = {0.f, 0.f, 0.f};
    for (int i = 0; i < 3; i++) { stride[i] = (maxExts[i] - minExts[i]) / (rakeGrid[i] + 1); }

    double maxValue = 0.0;
    double xCoord, yCoord, zCoord;
    for (int k = 1; k <= rakeGrid[Z]; k++) {
        zCoord = stride[Z] * k + minExts[Z];    // + stride[Z]/2.0;
        for (int j = 1; j <= rakeGrid[Y]; j++) {
            yCoord = stride[Y] * j + minExts[Y];    // + stride[Y]/2.0;
            for (int i = 1; i <= rakeGrid[X]; i++) {
                xCoord = stride[X] * i + minExts[X];    // + stride[X]/2.0;

                double value = grid->GetValue(xCoord, yCoord, zCoord);
                double missingValue = grid->GetMissingValue();

                value = abs(value);
                if (value > maxValue && value < std::numeric_limits<double>::max() && value > std::numeric_limits<double>::lowest() && value != missingValue && !isnan(value)) maxValue = value;
            }
        }
    }
    return maxValue;
}

vector<double> BarbRenderer::_getMaximumValues(size_t ts, const std::vector<string> &varNames) const
{
    std::vector<double> maxVarVals(3, 0.0);
    for (int i = 0; i < varNames.size(); i++) {
        if (varNames[i] == "") {
            continue;
        } else {
            string varName = varNames[i];

            BarbParams *p = dynamic_cast<BarbParams *>(GetActiveParams());
            assert(p);
            int refLevel = p->GetRefinementLevel();
            int compLevel = p->GetCompressionLevel();

            VAPoR::Grid *grid;
            grid = _dataMgr->GetVariable(ts, varName, refLevel, compLevel);
            if (!grid) {
                MyBase::SetErrMsg("Failed to retrieve %s at timestep %i", varName.c_str(), ts);
            } else {
                maxVarVals[i] = _getMaxAtBarbLocations(grid);
            }
        }
    }

    return maxVarVals;
}

double BarbRenderer::_getDomainHypotenuse(size_t ts) const
{
    std::vector<int>    axes;
    std::vector<double> minExts, maxExts;
    std::vector<string> varNames;

    BarbParams *p = dynamic_cast<BarbParams *>(GetActiveParams());
    assert(p);
    varNames = p->GetFieldVariableNames();

    bool status = DataMgrUtils::GetExtents(_dataMgr, ts, varNames, minExts, maxExts, axes);
    assert(status);

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
    assert(varData.size() >= 3);

    _maxValue = 0;

    _operateOnGrid(varData, false);

    double hypotenuse = _getDomainHypotenuse(ts);

    if (hypotenuse == 0.f) return;

    _maxThickness = hypotenuse * BARB_RADIUS_TO_HYPOTENUSE;
    _vectorScaleFactor = hypotenuse * BARB_LENGTH_TO_HYPOTENUSE;
    _vectorScaleFactor *= 1.0 / _maxValue;
}
