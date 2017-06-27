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
#include <vapor/BarbRenderer.h>
#include <vapor/BarbParams.h>
#include <vapor/Visualizer.h>

#include <vapor/regionparams.h>
#include <vapor/AnimationParams.h>
#include <vapor/ViewpointParams.h>
//#include <vapor/params.h>
//#include <vapor/arrowparams.h>
#include <vapor/MyBase.h>
#include <vapor/errorcodes.h>
#include <vapor/DataMgr.h>

using namespace VAPoR;
using namespace Wasp;

static RendererRegistrar<BarbRenderer> registrar(BarbRenderer::GetClassType(), BarbParams::GetClassType());

BarbRenderer::BarbRenderer(const ParamsMgr *pm, string winName, string dataSetName, string instName, DataStatus *ds)
: Renderer(pm, winName, dataSetName, BarbParams::GetClassType(), BarbRenderer::GetClassType(), instName, ds)
{
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
    return (0);
}

int BarbRenderer::_paintGL()
{
    // Set up the variable data required, while determining data
    // extents to use in rendering
    //
    StructuredGrid *varData[] = {NULL, NULL, NULL, NULL, NULL};

    AnimationParams *myAnimationParams;
    myAnimationParams = GetAnimationParams();
    size_t ts = myAnimationParams->GetCurrentTimestep();

    BarbParams *   bParams = (BarbParams *)GetActiveParams();
    int            refLevel = bParams->GetRefinementLevel();
    int            lod = bParams->GetCompressionLevel();
    vector<double> minExts, maxExts;
    // bParams->GetBox()->GetExtents(minExts, maxExts);
    _dataStatus->GetExtents(minExts, maxExts);

    // Find box extents for ROI
    //
    vector<string> varnames = bParams->GetFieldVariableNames();
    if (varnames != _fieldVariables) {
        _vectorScaleFactor = _calcDefaultScale(varnames, bParams);
        _fieldVariables = varnames;
    }

    // Get grids for our vector variables
    //
    int rc = _dataStatus->getGrids(ts, varnames, minExts, maxExts, &refLevel, &lod, varData);
    if (rc < 0) return (rc);

    // Get grids for our height variable
    //
    string hname = bParams->GetHeightVariableName();
    if (!hname.empty()) {
        vector<string> varnames;
        varnames.push_back(hname);
        int rc = _dataStatus->getGrids(ts, varnames, minExts, maxExts, &refLevel, &lod, &varData[3]);
        if (rc < 0) return (rc);
    }

    // Get grids for our auxillary variables
    //
    vector<string> auxvars = bParams->GetAuxVariableNames();
    if (!auxvars.empty() && !bParams->UseSingleColor()) {
        vector<string> varnames;
        varnames.push_back(auxvars[0]);
        int rc = _dataStatus->getGrids(ts, varnames, minExts, maxExts, &refLevel, &lod, &varData[4]);
        if (rc < 0) return (rc);
    }

    float vectorLengthScale = bParams->GetVectorScale() * _vectorScaleFactor;

    //
    // Perform OpenGL rendering of barbs
    //
    rc = performRendering(bParams, refLevel, vectorLengthScale, varData);

    // Release the locks on the data:
    DataMgr *dataMgr = _dataStatus->GetDataMgr();
    for (int k = 0; k < 5; k++) {
        if (varData[k]) dataMgr->UnlockGrid(varData[k]);
    }
    return (rc);
}

// Issue OpenGL calls to draw a cylinder with orthogonal ends from one point to another.
// Then put an barb head on the end
//
void BarbRenderer::drawBarb(const float startPoint[3], const float endPoint[3], float radius)
{
    // Constants are needed for cosines and sines, at 60 degree intervals.
    // The barb is really a hexagonal tube, but the shading makes it look round.
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

    // Calculate an orthonormal frame, dirVec, uVec, bVec.  dirVec is the barb direction
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
// Perform the openGL rendering:
int BarbRenderer::performRendering(    // DataMgr* dataMgr,
    const RenderParams *params, int actualRefLevel, float vectorLengthScale, StructuredGrid *variableData[5])
{
    BarbParams *     bParams = (BarbParams *)params;
    AnimationParams *myAnimationParams;
    myAnimationParams = GetAnimationParams();
    size_t timestep = myAnimationParams->GetCurrentTimestep();

    vector<double> rMinExtents, rMaxExtents;
    bParams->GetBox()->GetExtents(rMinExtents, rMaxExtents);
    // Convert to user coordinates:
    vector<double> minExts, maxExts;
    _dataStatus->GetExtents(minExts, maxExts);

    const vector<long> rGrid = bParams->GetGrid();
    int                rakeGrid[3];
    double             rakeExts[6];    // rake extents in user coordinates

    for (int i = 0; i < 3; i++) {
        rakeExts[i] = minExts[i];
        rakeExts[i + 3] = maxExts[i];
    }

    string           winName = GetVisualizer();
    ViewpointParams *vpParams = _paramsMgr->GetViewpointParams(winName);
    // Barb thickness is .001*LineThickness*viewDiameter.
    float thickness = bParams->GetLineThickness();
    // float rad =(float)( 0.001*vpParams->GetCurrentViewDiameter()*thickness);
    float rad = (float)(1000 * thickness);
    cout << "fudging barb line thickness.  Fix me!" << endl;
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

    rakeGrid[0] = rakeGrid[1] = 5;
    rakeGrid[2] = 1;

    // NEED TO PASS RAKE EXTENTS, NOT DOMAIN EXTENTS
    //
    renderScottsGrid(rakeGrid, rakeExts, variableData, timestep, vectorLengthScale, rad, params);

    return 0;
}

float BarbRenderer::getHeightOffset(StructuredGrid *heightVar, float xCoord, float yCoord, bool &missing)
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

void BarbRenderer::renderScottsGrid(int rakeGrid[3], double rakeExts[6], StructuredGrid *variableData[5], int timestep, float vectorLengthScale, float rad, const RenderParams *params)
{
    string           winName = GetVisualizer();
    ViewpointParams *vpParams = _paramsMgr->GetViewpointParams(winName);
    vector<double>   scales = vpParams->GetStretchFactors();

    StructuredGrid *heightVar = variableData[3];

    float end[3];
    float xStride = (rakeExts[3] - rakeExts[0]) / ((float)rakeGrid[0] + 1);
    float yStride = (rakeExts[4] - rakeExts[1]) / ((float)rakeGrid[1] + 1);
    float zStride = (rakeExts[5] - rakeExts[2]) / ((float)rakeGrid[2] + 1);

    float xCoord, yCoord, zCoord;
    for (int k = 1; k <= rakeGrid[2]; k++) {
        zCoord = zStride * k + rakeExts[2];
        // cout << zCoord << " " << zStride << " " << k << " " << minExts[2] << endl;
        for (int j = 1; j <= rakeGrid[1]; j++) {
            yCoord = yStride * j + rakeExts[1];
            for (int i = 1; i <= rakeGrid[0]; i++) {
                xCoord = xStride * i + rakeExts[0];

                bool missing = false;
                if (heightVar) { zCoord += getHeightOffset(heightVar, xCoord, yCoord, missing); }

                float direction[3] = {0.f, 0.f, 0.f};
                for (int dim = 0; dim < 3; dim++) {
                    direction[dim] = 0.f;
                    if (variableData[dim]) { direction[dim] = variableData[dim]->GetValue(xCoord, yCoord, zCoord); }

                    float missingVal = variableData[dim]->GetMissingValue();
                    if (direction[dim] == missingVal) { missing = true; }
                }

                float point[3] = {xCoord, yCoord, zCoord};
                end[0] = point[0] + scales[0] * direction[0];
                end[1] = point[1] + scales[1] * direction[1];
                end[2] = point[2] + scales[2] * direction[2];
                if (!missing) { drawBarb(point, end, rad * 10); }
            }
        }
    }
    return;
}

void BarbRenderer::renderUnaligned(int rakeGrid[3], double rakeExts[6], StructuredGrid *variableData[5], int timestep, float vectorLengthScale, float rad, const RenderParams *params)
{
    double point[3];
    float  dirVec[3], endPoint[3], fltPnt[3];

    string           winName = GetVisualizer();
    ViewpointParams *vpParams = _paramsMgr->GetViewpointParams(winName);
    vector<double>   scales = vpParams->GetStretchFactors();

    BarbParams *   bParams = (BarbParams *)params;
    vector<double> minExts, maxExts;
    bParams->GetBox()->GetExtents(minExts, maxExts);

    bool              doColorMapping = !bParams->UseSingleColor();
    TransferFunction *transFunc = 0;

    float clut[256 * 4];
    if (doColorMapping) {
        vector<string> colorVar = params->GetAuxVariableNames();
        transFunc = bParams->MakeTransferFunc(colorVar[0]);
        assert(transFunc);
        transFunc->makeLut(clut);
    }

    for (int k = 0; k < rakeGrid[2]; k++) {
        float pntz = (rakeExts[2] + (0.5 + (float)k) * ((rakeExts[5] - rakeExts[2]) / (float)rakeGrid[2]));

        for (int j = 0; j < rakeGrid[1]; j++) {
            point[1] = (rakeExts[1] + (0.5 + (float)j) * ((rakeExts[4] - rakeExts[1]) / (float)rakeGrid[1]));

            for (int i = 0; i < rakeGrid[0]; i++) {
                point[0] = (rakeExts[0] + (0.5 + (float)i) * ((rakeExts[3] - rakeExts[0]) / (float)rakeGrid[0]));
                bool  missing = false;
                float offset = 0.;
                if (variableData[3]) {
                    offset = variableData[3]->GetValue(point[0], point[1], 0.);
                    if (offset == variableData[3]->GetMissingValue()) {
                        missing = true;
                        offset = 0.;
                    }
                }
                point[2] = pntz + offset;
                for (int dim = 0; dim < 3; dim++) {
                    dirVec[dim] = 0.f;
                    if (variableData[dim]) {
                        dirVec[dim] = variableData[dim]->GetValue(point[0], point[1], point[2] + offset);

                        if (dirVec[dim] == variableData[dim]->GetMissingValue()) { missing = true; }
                    }
                    endPoint[dim] = scales[dim] * (point[dim] - minExts[dim] + vectorLengthScale * dirVec[dim]);
                    fltPnt[dim] = (float)((point[dim] - minExts[dim]) * scales[dim]);
                }
                if (doColorMapping) {
                    float mappedColor[4] = {0., 0., 0., 0.};
                    float colorval = variableData[4]->GetValue(point[0], point[1], point[2]);
                    if (colorval == variableData[4]->GetMissingValue())
                        missing = true;
                    else {
                        // Use the transfer function to map the data:
                        int lutIndex = transFunc->mapFloatToIndex(colorval);
                        for (int i = 0; i < 4; i++) mappedColor[i] = clut[4 * lutIndex + i];
                    }
                    glColor4fv(mappedColor);
                }

                if (!missing) drawBarb(fltPnt, endPoint, rad);
            }
        }
    }
}
void BarbRenderer::renderAligned(int rakeGrid[3], double rakeExts[6], StructuredGrid *variableData[5], int timestep, float vectorLengthScale, float rad, const RenderParams *params)
{
    double         point[3];
    float          dirVec[3], endPoint[3], fltPnt[3];
    vector<double> minExts, maxExts;

    BarbParams *   bParams = (BarbParams *)params;
    vector<double> scales = params->GetStretchFactors();
    bParams->GetBox()->GetExtents(minExts, maxExts);
    bool              doColorMapping = !bParams->UseSingleColor();
    TransferFunction *transFunc = 0;
    float             clut[256 * 4];

    if (doColorMapping) {
        vector<string> colorVar = params->GetAuxVariableNames();
        transFunc = bParams->MakeTransferFunc(colorVar[0]);
        assert(transFunc);
        transFunc->makeLut(clut);
    }

    for (int k = 0; k < rakeGrid[2]; k++) {
        float pntz = (rakeExts[2] + (0.5 + (float)k) * ((rakeExts[5] - rakeExts[2]) / (float)rakeGrid[2]));

        for (int j = 0; j < rakeGrid[1]; j++) {
            point[1] = (rakeExts[1] + ((double)j) * ((rakeExts[4] - rakeExts[1]) / (double)rakeGrid[1]));

            for (int i = 0; i < rakeGrid[0]; i++) {
                bool  missing = false;
                float offset = 0.;
                point[0] = rakeExts[0] + ((double)i) * ((rakeExts[3] - rakeExts[0]) / (double)rakeGrid[0]);
                if (variableData[3]) {
                    offset = variableData[3]->GetValue(point[0], point[1], 0.);
                    if (offset == variableData[3]->GetMissingValue()) { missing = true; }
                }
                point[2] = pntz + offset;
                for (int dim = 0; dim < 3; dim++) {
                    dirVec[dim] = 0.f;
                    if (variableData[dim]) {
                        dirVec[dim] = variableData[dim]->GetValue(point[0], point[1], point[2]);
                        if (dirVec[dim] == variableData[dim]->GetMissingValue()) { missing = true; }
                    }
                    endPoint[dim] = scales[dim] * (point[dim] - minExts[dim] + vectorLengthScale * dirVec[dim]);
                    fltPnt[dim] = (float)((point[dim] - minExts[dim]) * scales[dim]);
                }
                // For color mapping, determine the mapping of the barb position
                if (doColorMapping) {
                    float mappedColor[4] = {0., 0., 0., 0.};
                    float colorval = variableData[4]->GetValue(point[0], point[1], point[2]);
                    if (colorval == variableData[4]->GetMissingValue())
                        missing = true;
                    else {
                        // Use the transfer function to map the data:
                        int lutIndex = transFunc->mapFloatToIndex(colorval);
                        for (int i = 0; i < 4; i++) mappedColor[i] = clut[4 * lutIndex + i];
                    }
                    glColor4fv(mappedColor);
                }
                if (!missing) drawBarb(fltPnt, endPoint, rad);
            }
        }
    }
}

double BarbRenderer::_calcDefaultScale(const vector<string> &varnames, const BarbParams *bParams)
{
    assert(varnames.size() <= 3);
    double maxvarvals[3] = {1.0, 1.0, 1.0};

    DataMgr *dataMgr = _dataStatus->GetDataMgr();

    vector<double> stretch = bParams->GetStretchFactors();
    for (int i = 0; i < varnames.size(); i++) {
        if (varnames[i] == "") {
            maxvarvals[i] = 0.;
        } else {
            // Obtain the default
            //

            vector<double> minmax;
            dataMgr->GetDataRange(0, varnames[i], 0, 0, minmax);
            maxvarvals[i] = Max(abs(minmax[0]), abs(minmax[1]));
        }
    }

    for (int i = 0; i < 3; i++) maxvarvals[i] *= stretch[i];

    const double *extents = _dataStatus->getLocalExtents();
    double        maxVecLength = (double)Max(extents[3] - extents[0], extents[4] - extents[1]) * 0.1;

    double maxVecVal = Max(maxvarvals[0], Max(maxvarvals[1], maxvarvals[2]));

    if (maxVecVal == 0.)
        return (maxVecLength);
    else
        return (maxVecLength / maxVecVal);
}
