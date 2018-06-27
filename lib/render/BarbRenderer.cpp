//*************************************************************
//
// Copyright (C) 2017
// University Corporation for Atmospheric Research
// All Rights Reserved
//
// ************************************************************

// Specify the barbhead width compared with barb diameter:
#define BARB_HEAD_FACTOR 3.0
// Specify how long the barb tube is, in front of where the barbhead is attached:
#define BARB_LENGTH_FACTOR 0.9

#include <vapor/glutil.h>    // Must be included first!!!
#include <cstdlib>
#include <cstdio>
#include <cstring>

#ifndef WIN32
    #include <unistd.h>
#endif

//#include <vapor/Renderer.h>
#include <vapor/DataMgrUtils.h>
#include <vapor/BarbRenderer.h>
#include <vapor/BarbParams.h>
#include <vapor/Visualizer.h>

#include <vapor/regionparams.h>
#include <vapor/ViewpointParams.h>
#include <vapor/MyBase.h>
#include <vapor/errorcodes.h>
#include <vapor/DataMgr.h>

#define X 0
#define Y 1
#define Z 2

using namespace VAPoR;
using namespace Wasp;

static RendererRegistrar<BarbRenderer> registrar(BarbRenderer::GetClassType(), BarbParams::GetClassType());

BarbRenderer::BarbRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataMgr *dataMgr)
: Renderer(pm, winName, dataSetName, BarbParams::GetClassType(), BarbRenderer::GetClassType(), instName, dataMgr)
{
    _drawList = 0;
    _fieldVariables.clear();
    _vectorScaleFactor = 1.0;
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
    BarbParams *p = (BarbParams *)GetActiveParams();
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
    BarbParams *p = (BarbParams *)GetActiveParams();
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
    float          vectorLengthScale;
    string         hname;
    string         colorVar;

    BarbParams *bParams = (BarbParams *)GetActiveParams();
    size_t      ts = bParams->GetCurrentTimestep();

    int            refLevel = bParams->GetRefinementLevel();
    int            lod = bParams->GetCompressionLevel();
    vector<double> minExts, maxExts;
    bParams->GetBox()->GetExtents(minExts, maxExts);

    vector<string> varnames = bParams->GetFieldVariableNames();
    if (!VariableExists(ts, varnames, refLevel, lod, true)) {
        SetErrMsg("One or more selected field variables does not exist");
        rc = -1;
        goto RETURN;
    }

    // Find box extents for ROI
    //
    //	if (varnames != _fieldVariables) {
    cout << "......Calculating _vectorScaleFactor....." << endl;
    _vectorScaleFactor = _calcDefaultScale(ts, varnames, bParams);
    _fieldVariables = varnames;
    //	}

    // Get grids for our vector variables
    //
    rc = DataMgrUtils::GetGrids(_dataMgr, ts, varnames, minExts, maxExts, true, &refLevel, &lod, varData);

    if (rc < 0) goto RETURN;
    varData.push_back(NULL);
    varData.push_back(NULL);

    // Get grids for our height variable
    //
    hname = bParams->GetHeightVariableName();
    if (!hname.empty()) {
        Grid *sg = NULL;
        int   rc = DataMgrUtils::GetGrids(_dataMgr, ts, hname, minExts, maxExts, true, &refLevel, &lod, &sg);
        if (rc < 0) {
            for (int i = 0; i < varData.size(); i++) {
                if (varData[i]) _dataMgr->UnlockGrid(varData[i]);
            }
            goto RETURN;
        }
        varData[3] = sg;
    }

    // Get grids for our auxillary variables
    //
    colorVar = bParams->GetColorMapVariableName();
    if (!bParams->UseSingleColor() && !colorVar.empty()) {
        Grid *sg;
        int   rc = DataMgrUtils::GetGrids(_dataMgr, ts, colorVar, minExts, maxExts, true, &refLevel, &lod, &sg);
        if (rc < 0) {
            for (int i = 0; i < varData.size(); i++) {
                if (varData[i]) _dataMgr->UnlockGrid(varData[i]);
            }
            goto RETURN;
        }
        varData[4] = sg;
    }

    vectorLengthScale = bParams->GetLengthScale() * _vectorScaleFactor;
    cout << "vectorLengthScale (rendering) " << vectorLengthScale << endl;

    //
    // Perform OpenGL rendering of barbs
    //
    rc = performRendering(bParams, refLevel, vectorLengthScale, varData);

    // Release the locks on the data:
    for (int i = 0; i < varData.size(); i++) {
        if (varData[i]) _dataMgr->UnlockGrid(varData[i]);
    }

RETURN:
    glEndList();
    return (rc);
}

// Issue OpenGL calls to draw a cylinder with orthogonal ends from
// one point to another.  Then put an barb head on the end
//
void BarbRenderer::drawBarb(const float startPoint[3], const float endPoint[3], float radius)
{
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

    // Calculate an orthonormal frame, dirVec, uVec, bVec.
    // dirVec is the barb direction
    float dirVec[3], bVec[3], uVec[3];
    vsub(endPoint, startPoint, dirVec);
    float len = vlength(dirVec);
    if (len == 0.f) return;
    vscale(dirVec, 1. / len);

    // Calculate uVec, orthogonal to dirVec:
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

    int i;
    // Calculate nextPoint and vertexPoint, for barbhead

    for (i = 0; i < 3; i++) {
        nextPoint[i] = (1. - BARB_LENGTH_FACTOR) * startPoint[i] + BARB_LENGTH_FACTOR * endPoint[i];
        // Assume a vertex angle of 45 degrees:
        vertexPoint[i] = nextPoint[i] + dirVec[i] * radius;
        headCenter[i] = nextPoint[i] - dirVec[i] * (BARB_HEAD_FACTOR * radius - radius);
    }
    // calculate 6 points in plane orthog to dirVec, in plane of point
    for (i = 0; i < 6; i++) {
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

    glBegin(GL_POLYGON);
    glNormal3fv(dirVec);
    for (int k = 0; k < 6; k++) { glVertex3fv(startVertex + 3 * k); }
    glEnd();

    // calc for endpoints:
    for (i = 0; i < 6; i++) {
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

    // Now make a triangle strip for cylinder sides:
    glBegin(GL_TRIANGLE_STRIP);

    for (i = 0; i < 6; i++) {
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
    // Now draw the barb head.  First calc 6 vertices at back of barbhead
    // Reuse startNormal and startVertex vectors
    // calc for endpoints:
    for (i = 0; i < 6; i++) {
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
    // Create a triangle fan from these 6 vertices.
    glBegin(GL_TRIANGLE_FAN);
    glNormal3fv(dirVec);
    glVertex3fv(vertexPoint);
    for (i = 0; i < 6; i++) {
        glNormal3fv(startNormal + 3 * i);
        glVertex3fv(startVertex + 3 * i);
    }
    // Repeat first point to close fan:
    glNormal3fv(startNormal);
    glVertex3fv(startVertex);
    glEnd();
}

int BarbRenderer::performRendering(BarbParams *bParams, int actualRefLevel, float vectorLengthScale, vector<Grid *> variableData)
{
    assert(variableData.size() == 5);

    size_t timestep = bParams->GetCurrentTimestep();

    vector<double> rMinExtents, rMaxExtents;
    bParams->GetBox()->GetExtents(rMinExtents, rMaxExtents);

    double rakeExts[6];    // rake extents in user coordinates

    rakeExts[0] = rMinExtents[0];
    rakeExts[1] = rMinExtents[1];
    rakeExts[2] = rMinExtents[2];
    rakeExts[3] = rMaxExtents[0];
    rakeExts[4] = rMaxExtents[1];
    rakeExts[5] = rMaxExtents[2];

    string           winName = GetVisualizer();
    ViewpointParams *vpParams = _paramsMgr->GetViewpointParams(winName);

    // Barb thickness is .001*LineThickness*viewDiameter.
    // float thickness = bParams->GetLineThickness();

    size_t         ts = bParams->GetCurrentTimestep();
    vector<string> varnames = bParams->GetFieldVariableNames();
    float          thickness = .2 * _calcDefaultScale(ts, varnames, bParams);

    // float rad =(float)( 0.001*vpParams->GetCurrentViewDiameter()*thickness);
    // float rad = (float) (1000*thickness);
    float rad = (float)(thickness);
    // Set up lighting and color
    int   nLights = vpParams->getNumLights();
    float fcolor[3];
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

    vector<long> longGrid = bParams->GetGrid();
    int          rakeGrid[3];
    rakeGrid[0] = (int)longGrid[0];
    rakeGrid[1] = (int)longGrid[1];
    rakeGrid[2] = (int)longGrid[2];

    renderGrid(rakeGrid, rakeExts, variableData, timestep, vectorLengthScale, rad, bParams);

    return 0;
}

float BarbRenderer::getHeightOffset(Grid *heightVar, float xCoord, float yCoord, bool &missing)
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

void BarbRenderer::renderGrid(int rakeGrid[3], double rakeExts[6], vector<Grid *> variableData, int timestep, float length, float rad, BarbParams *bParams)
{
    assert(variableData.size() == 5);

    string         winName = GetVisualizer();
    vector<double> scales(3, 1.0);

    Grid *heightVar = variableData[3];

    float end[3];
    float xStride = (rakeExts[3] - rakeExts[0]) / ((float)rakeGrid[0] + 1);
    float yStride = (rakeExts[4] - rakeExts[1]) / ((float)rakeGrid[1] + 1);
    float zStride = (rakeExts[5] - rakeExts[2]) / ((float)rakeGrid[2] + 1);

    string          colorVar = bParams->GetColorMapVariableName();
    float           clut[256 * 4];
    bool            doColorMapping = !bParams->UseSingleColor() && !colorVar.empty();
    MapperFunction *tf = 0;
    if (doColorMapping) {
        tf = (MapperFunction *)bParams->GetMapperFunc(colorVar);
        assert(tf);
        tf->makeLut(clut);
    }

    float xCoord, yCoord, zCoordGrid, zCoord;
    for (int k = 0; k <= rakeGrid[2]; k++) {
        zCoordGrid = zStride * k + rakeExts[2] + zStride / 2.0;
        for (int j = 0; j <= rakeGrid[1]; j++) {
            yCoord = yStride * j + rakeExts[1] + yStride / 2.0;
            for (int i = 0; i <= rakeGrid[0]; i++) {
                xCoord = xStride * i + rakeExts[0] + xStride / 2.0;
                zCoord = zCoordGrid;

                bool missing = false;
                if (heightVar) { zCoord += getHeightOffset(heightVar, xCoord, yCoord, missing); }

                float direction[3] = {0.f, 0.f, 0.f};
                for (int dim = 0; dim < 3; dim++) {
                    direction[dim] = 0.f;
                    if (variableData[dim]) {
                        direction[dim] = variableData[dim]->GetValue(xCoord, yCoord, zCoord);

                        float missingVal = variableData[dim]->GetMissingValue();
                        if (direction[dim] == missingVal) { missing = true; }
                    }
                }

                float point[3] = {xCoord, yCoord, zCoord};
                end[0] = point[0] + scales[0] * direction[0] * length;
                end[1] = point[1] + scales[1] * direction[1] * length;
                end[2] = point[2] + scales[2] * direction[2] * length;

                if (doColorMapping) {
                    float val = variableData[4]->GetValue(point[0], point[1], point[2]);
                    if (val == variableData[4]->GetMissingValue())
                        missing = true;
                    else {
                        missing = GetColorMapping(tf, val, clut);
                    }
                }
                if (!missing) {
                    string                  datasetName = GetMyDatasetName();
                    string                  myVisName = GetVisualizer();
                    VAPoR::ViewpointParams *vpp = _paramsMgr->GetViewpointParams(myVisName);

                    Transform *t = vpp->GetTransform(datasetName);
                    assert(t);
                    vector<double> scales = t->GetScales();

                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glScalef(1.f / scales[0], 1.f / scales[1], 1.f / scales[2]);
                    drawBarb(point, end, rad * 10);
                    glPopMatrix();
                }
            }
        }
    }
    return;
}

bool BarbRenderer::GetColorMapping(MapperFunction *tf, float val, float clut[256 * 4])
{
    bool missing = false;

    //	float clut[256*4];
    //	tf->makeLut(clut);

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
    BarbParams *   p = (BarbParams *)GetActiveParams();
    p->GetBox()->GetExtents(minExts, maxExts);

    vector<long> rakeGrid = p->GetGrid();

    // float xStride = (rakeExts[3]-rakeExts[0]) / (rakeGrid[0]+1);
    // float yStride = (rakeExts[4]-rakeExts[1]) / (rakeGrid[1]+1);
    // float zStride = (rakeExts[5]-rakeExts[2]) / (rakeGrid[2]+1);

    float stride[3] = {0.f, 0.f, 0.f};
    for (int i = 0; i < 3; i++) { stride[i] = (maxExts[i] - minExts[i]) / (rakeGrid[i] + 1); }

    double maxValue = 0.0;
    double xCoord, yCoord, zCoord;
    for (int k = 1; k <= rakeGrid[Z]; k++) {
        zCoord = stride[Z] * k + minExts[Z] + stride[Z] / 2.0;
        for (int j = 1; j <= rakeGrid[Y]; j++) {
            yCoord = stride[Y] * j + minExts[Y] + stride[Y] / 2.0;
            for (int i = 1; i <= rakeGrid[X]; i++) {
                xCoord = stride[X] * i + minExts[X] + stride[X] / 2.0;

                double value = grid->GetValue(xCoord, yCoord, zCoord);
                if (value > maxValue) maxValue = value;

                // cout << i << " " << j << " " << k << " " << xCoord << " " << yCoord << " " << zCoord << endl;
            }
        }
    }
    return maxValue;
}

// std::vector<double> BarbRenderer::_getMaxVarValues(
vector<double> BarbRenderer::_getMaxVarValues(size_t ts, const std::vector<string> &varNames) const
{
    std::vector<double> maxVarVals(3, 0.0);
    for (int i = 0; i < varNames.size(); i++) {
        if (varNames[i] == "") {
            //			maxvarvals.push_back(0.);
            continue;
        } else {
            string varName = varNames[i];

            BarbParams *p = (BarbParams *)GetActiveParams();
            int         refLevel = p->GetRefinementLevel();
            int         compLevel = p->GetCompressionLevel();

            VAPoR::Grid *grid;
            grid = _dataMgr->GetVariable(ts, varName, refLevel, compLevel);

            maxVarVals[i] = _getMaxAtBarbLocations(grid);

            //			vector <double> minmax;
            //			_dataMgr->GetDataRange(0,varnames[i], 0, 0, minmax);
            //			maxvarvals.push_back(Max(abs(minmax[0]),abs(minmax[1])));
        }
    }

    cout << "maxes " << maxVarVals[0] << " " << maxVarVals[1] << " " << maxVarVals[2] << endl;

    return maxVarVals;
}

double BarbRenderer::_getDomainHypotenuse(size_t ts, const std::vector<string> varnames) const
{
    vector<int>    axes;
    vector<double> minExts, maxExts;
    bool           status = DataMgrUtils::GetExtents(_dataMgr, ts, varnames, minExts, maxExts, axes);
    assert(status);

    double xLen = maxExts[0] - minExts[0];
    double yLen = maxExts[1] - minExts[1];
    double zLen = maxExts[2] - minExts[2];
    double diag = sqrt(xLen * xLen + yLen * yLen + zLen * zLen);
    return diag;
}

double BarbRenderer::_getMaxDomainLength(size_t ts, const std::vector<string> varnames) const
{
    vector<int>    axes;
    vector<double> minExts, maxExts;
    bool           status = DataMgrUtils::GetExtents(_dataMgr, ts, varnames, minExts, maxExts, axes);
    assert(status);

    double maxVecLength = 0.0;
    for (int i = 0; i < minExts.size(); i++) { maxVecLength = Max(maxVecLength, (maxExts[i] - minExts[i])); }
    maxVecLength *= 0.1;

    cout << "maxVecLength " << maxVecLength << endl;

    return maxVecLength;
}

double BarbRenderer::_calcDefaultScale(size_t ts, const vector<string> &varnames, const BarbParams *bParams)
{
    assert(varnames.size() <= 3);
    if (varnames[0] == "" && varnames[1] == "" && varnames[2] == "") return 1.f;

    Transform *    t = bParams->GetTransform();
    vector<double> scales = t->GetScales();

    vector<double> maxVarVals = _getMaxVarValues(ts, varnames);
    for (int i = 0; i < maxVarVals.size(); i++) maxVarVals[i] *= scales[i];
    double maxVarVal = Max(maxVarVals[0], maxVarVals[1]);
    maxVarVal = Max(maxVarVal, maxVarVals[2]);

    double maxVecLength = _getDomainHypotenuse(ts, varnames);

    _getMaxVarValues(ts, varnames);
    double maxVecVal = 0.0;
    for (int i = 0; i < maxVarVals.size(); i++) { maxVecVal = Max(maxVecLength, maxVarVals[i]); }

    cout << ". " << maxVecLength << " " << maxVarVal << endl;

    double returnValue = maxVecLength * .05;
    returnValue *= 1.0 / maxVarVal;

    //	if (maxVecVal == 0.) returnValue = maxVecLength;
    //	else returnValue = maxVecLength/maxVecVal;

    return returnValue;
    // return 1000.0;
}
