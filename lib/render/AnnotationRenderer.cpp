//-- AnnotationRenderer.cpp ----------------------------------------------------------
//
//				   Copyright (C)  2015
//	 University Corporation for Atmospheric Research
//				   All Rights Reserved
//
//----------------------------------------------------------------------------
//
//	  File:		   AnnotationRenderer.cpp
//
//	  Author:		 Alan Norton
//
//	  Description:  Implementation of AnnotationRenderer class
//
//----------------------------------------------------------------------------

#include <vapor/glutil.h>    // Must be included first!!!
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cfloat>
#include <numeric>
#include <sstream>
#include <iomanip>

#ifndef WIN32
    #include <unistd.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vapor/DataStatus.h>
#include <vapor/AnnotationRenderer.h>
#include <vapor/ResourcePath.h>
#include "vapor/LegacyGL.h"
#include "vapor/TextLabel.h"
#include "vapor/AnnotationParams.h"
#define INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
#include <vapor/LegacyVectorMath.h>

#define X 0
#define Y 1
#define Z 2

#define ARROW_SCALE_FACTOR .25

using namespace VAPoR;
using namespace Wasp;

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
AnnotationRenderer::AnnotationRenderer(const ParamsMgr *pm, const DataStatus *dataStatus, string winName)
{
    m_paramsMgr = pm;
    m_dataStatus = dataStatus;
    m_winName = winName;
    _glManager = nullptr;

    _currentTimestep = 0;

    _fontName = "arimo";
    _fontFile = GetSharePath("fonts/" + _fontName + ".ttf");
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
AnnotationRenderer::~AnnotationRenderer()
{
#ifdef VAPOR3_0_0_ALPHA
    if (_textObjectsValid) invalidateCache();
#endif
}

void AnnotationRenderer::InitializeGL(GLManager *glManager) { _glManager = glManager; }

void AnnotationRenderer::drawDomainFrame(std::vector<double> corners) const
{
    assert(corners.size() == 6);

    AnnotationParams *vfParams = m_paramsMgr->GetAnnotationParams(m_winName);

    std::vector<double> minExts = {corners[X], corners[Y], corners[Z]};
    std::vector<double> maxExts = {corners[X + 3], corners[Y + 3], corners[Z + 3]};

    int    i;
    int    numLines[3];
    double fullSize[3], modMin[3], modMax[3];

    // Instead:  either have 2 or 1 lines in each dimension.  2 if the size is < 1/3
    for (i = 0; i < minExts.size(); i++) {
        double regionSize = maxExts[i] - minExts[i];

        // Stretch size by 1%
        fullSize[i] = (maxExts[i] - minExts[i]) * 1.01;
        double mid = 0.5f * (maxExts[i] + minExts[i]);
        modMin[i] = mid - 0.5f * fullSize[i];
        modMax[i] = mid + 0.5f * fullSize[i];
        if (regionSize < fullSize[i] * .3)
            numLines[i] = 2;
        else
            numLines[i] = 1;
    }

    double clr[3];
    vfParams->GetDomainColor(clr);
    // glLineWidth(1);
    // Now draw the lines.  Divide each dimension into numLines[dim] sections.

    int x, y, z;
    // Do the lines in each z-plane
    // Turn on writing to the z-buffer
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    LegacyGL *lgl = _glManager->legacy;
    lgl->Color3f(clr[0], clr[1], clr[2]);
    lgl->Begin(GL_LINES);
    for (z = 0; z <= numLines[2]; z++) {
        float zCrd = modMin[2] + ((float)z / (float)numLines[2]) * fullSize[2];
        // Draw lines in x-direction for each y
        for (y = 0; y <= numLines[1]; y++) {
            float yCrd = modMin[1] + ((float)y / (float)numLines[1]) * fullSize[1];

            lgl->Vertex3f(modMin[0], yCrd, zCrd);
            lgl->Vertex3f(modMax[0], yCrd, zCrd);
        }
        // Draw lines in y-direction for each x
        for (x = 0; x <= numLines[0]; x++) {
            float xCrd = modMin[0] + ((float)x / (float)numLines[0]) * fullSize[0];

            lgl->Vertex3f(xCrd, modMin[1], zCrd);
            lgl->Vertex3f(xCrd, modMax[1], zCrd);
        }
    }
    // Do the lines in each y-plane

    for (y = 0; y <= numLines[1]; y++) {
        float yCrd = modMin[1] + ((float)y / (float)numLines[1]) * fullSize[1];
        // Draw lines in x direction for each z
        for (z = 0; z <= numLines[2]; z++) {
            float zCrd = modMin[2] + ((float)z / (float)numLines[2]) * fullSize[2];

            lgl->Vertex3f(modMin[0], yCrd, zCrd);
            lgl->Vertex3f(modMax[0], yCrd, zCrd);
        }
        // Draw lines in z direction for each x
        for (x = 0; x <= numLines[0]; x++) {
            float xCrd = modMin[0] + ((float)x / (float)numLines[0]) * fullSize[0];

            lgl->Vertex3f(xCrd, yCrd, modMin[2]);
            lgl->Vertex3f(xCrd, yCrd, modMax[2]);
        }
    }

    // Do the lines in each x-plane
    for (x = 0; x <= numLines[0]; x++) {
        float xCrd = modMin[0] + ((float)x / (float)numLines[0]) * fullSize[0];
        // Draw lines in y direction for each z
        for (z = 0; z <= numLines[2]; z++) {
            float zCrd = modMin[2] + ((float)z / (float)numLines[2]) * fullSize[2];

            lgl->Vertex3f(xCrd, modMin[1], zCrd);
            lgl->Vertex3f(xCrd, modMax[1], zCrd);
        }
        // Draw lines in z direction for each y
        // Draw lines in z direction for each y
        for (y = 0; y <= numLines[1]; y++) {
            float yCrd = modMin[1] + ((float)y / (float)numLines[1]) * fullSize[1];

            lgl->Vertex3f(xCrd, yCrd, modMin[2]);
            lgl->Vertex3f(xCrd, yCrd, modMax[2]);
        }
    }
    lgl->End();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
}

void AnnotationRenderer::DrawText()
{
    _glManager->PixelCoordinateSystemPush();

    DrawText(_miscAnnot);
    DrawText(_timeAnnot);
    DrawText(_axisAnnot);

    _glManager->PixelCoordinateSystemPop();
}

void AnnotationRenderer::DrawText(vector<billboard> billboards)
{
    float txtColor[] = {1.f, 1.f, 1.f, 1.f};
    float bgColor[] = {0.f, 0.f, 0.f, 0.f};
    float coords[] = {67.5f, 31.6f, 0.f};

    auto size = _glManager->GetViewportSize();

    for (int i = 0; i < billboards.size(); i++) {
        string text = billboards[i].text;

        if (billboards[i].x == 0 && billboards[i].y == 0) {
            coords[0] = billboards[i].xn * size.x;
            coords[1] = billboards[i].yn * size.y;
        } else {
            coords[0] = billboards[i].x;
            coords[1] = billboards[i].y;
        }
        int size = billboards[i].size;
        txtColor[0] = billboards[i].color[0];
        txtColor[1] = billboards[i].color[1];
        txtColor[2] = billboards[i].color[2];

        TextLabel label(_glManager, _fontName, size);
        label.ForegroundColor = glm::make_vec4(txtColor);
        label.BackgroundColor = glm::make_vec4(bgColor);
        label.DrawText(glm::make_vec2(coords), text);
    }
}

void AnnotationRenderer::AddText(string text, int x, int y, int size, float color[3], int type)
{
    //_billboards.clear();  // Temporary hack.  We eventually need separate
    // billboard groups for time annotations, axis
    // labels, etc.  Grouping them all in the same
    // vector makes it hard if not impossible to
    // make changes to any of the labels (color, size,
    // etc)
    billboard myBoard;
    myBoard.text = text;
    myBoard.x = x;
    myBoard.y = y;
    myBoard.xn = 0;
    myBoard.yn = 0;
    myBoard.size = size;
    myBoard.color[0] = color[0];
    myBoard.color[1] = color[1];
    myBoard.color[2] = color[2];

    if (type == 0) {    // Miscellaneous annotation
        _miscAnnot.push_back(myBoard);
    } else if (type == 1) {    // Time annotation
        _timeAnnot.push_back(myBoard);
    } else if (type == 2) {
        _axisAnnot.push_back(myBoard);
    }
}

void AnnotationRenderer::AddTextNormalizedCoords(string text, float x, float y, int size, float color[3], int type)
{
    billboard myBoard;
    myBoard.text = text;
    myBoard.x = 0;
    myBoard.y = 0;
    myBoard.xn = x;
    myBoard.yn = y;
    myBoard.size = size;
    myBoard.color[0] = color[0];
    myBoard.color[1] = color[1];
    myBoard.color[2] = color[2];

    if (type == 0) {    // Miscellaneous annotation
        _miscAnnot.push_back(myBoard);
    } else if (type == 1) {    // Time annotation
        _timeAnnot.push_back(myBoard);
    } else if (type == 2) {
        _axisAnnot.push_back(myBoard);
    }
}

void AnnotationRenderer::ClearText(int type)
{
    if (type == -1) {
        _miscAnnot.clear();
        _timeAnnot.clear();
        _axisAnnot.clear();
    }
    if (type == 0) {
        _miscAnnot.clear();
    } else if (type == 1) {
        _timeAnnot.clear();
    } else if (type == 2) {
        _axisAnnot.clear();
    }
}

void AnnotationRenderer::applyTransform(Transform *t)
{
    vector<double> scale = t->GetScales();
    vector<double> origin = t->GetOrigin();
    vector<double> translate = t->GetTranslations();
    vector<double> rotate = t->GetRotations();
    VAssert(translate.size() == 3);
    VAssert(rotate.size() == 3);
    VAssert(scale.size() == 3);
    VAssert(origin.size() == 3);

    _glManager->matrixManager->Translate(translate[0], translate[1], translate[2]);

    _glManager->matrixManager->Translate(origin[0], origin[1], origin[2]);

    _glManager->matrixManager->Rotate(glm::radians(rotate[0]), 1, 0, 0);
    _glManager->matrixManager->Rotate(glm::radians(rotate[1]), 0, 1, 0);
    _glManager->matrixManager->Rotate(glm::radians(rotate[2]), 0, 0, 1);
    _glManager->matrixManager->Scale(scale[0], scale[1], scale[2]);

    _glManager->matrixManager->Translate(-origin[0], -origin[1], -origin[2]);
}

void AnnotationRenderer::_makeTransformMatrix(const Transform *transform, glm::mat4 &transformMatrix) const
{
    vector<double> scales = transform->GetScales();
    vector<double> origins = transform->GetOrigin();
    vector<double> translations = transform->GetTranslations();
    vector<double> rotations = transform->GetRotations();

    glm::mat4 m;

    m = glm::translate(glm::mat4(1.f), glm::vec3(translations[X], translations[Y], translations[Z]));

    m = glm::translate(m, glm::vec3(origins[X], origins[Y], origins[Z]));

    m = glm::rotate(m, glm::radians((float)rotations[X]), glm::vec3(1.f, 0.f, 0.f));
    m = glm::rotate(m, glm::radians((float)rotations[Y]), glm::vec3(0.f, 1.f, 0.f));
    m = glm::rotate(m, glm::radians((float)rotations[Z]), glm::vec3(0.f, 0.f, 1.f));

    m = glm::scale(m, glm::vec3(scales[X], scales[Y], scales[Z]));

    m = glm::translate(m, glm::vec3(-origins[X], -origins[Y], -origins[Z]));

    transformMatrix = m;
}

void AnnotationRenderer::_applyDataMgrCornerToDomain(std::vector<double> &domainExtents, const glm::vec4 &dataMgrCorner, const glm::mat4 &transformMatrix) const
{
    assert(domainExtents.size() == 6);

    // transform our corner
    glm::vec4 transformedCorner;
    transformedCorner = transformMatrix * dataMgrCorner;

    // See if the minimum and maximum extents of our corner exceed the
    // the currently defined domain.
    for (int i = 0; i < 6; i++) {
        int transformedCornerIndex = i % 3;
        // use this corner to define all domain corners that are uninitialized
        if (std::isnan(domainExtents[i])) domainExtents[i] = transformedCorner[transformedCornerIndex];
        // otherwise see if the corner exceeds our currently defined domain minima
        else if (i < 3) {
            if (transformedCorner[transformedCornerIndex] < domainExtents[i]) { domainExtents[i] = transformedCorner[transformedCornerIndex]; }
        }
        // otherwise see if the corner exceeds our currently defined domain maxima
        else {
            if (transformedCorner[transformedCornerIndex] > domainExtents[i]) { domainExtents[i] = transformedCorner[transformedCornerIndex]; }
        }
    }
}

// The lookup table below designates how the eight corners of the dataMgr are
// defined.
//
// For example, corner# 3 will be comprised of the following dataMgr
// extent values: { minX, maxY, maxZ}.  Corner#6 is { maxX, maxY, minZ}
//
//  X Y Z corner#
//  0 0 0       0
//  0 0 1       1
//  0 1 0       2
//  0 1 1       3
//  1 0 0       4
//  1 0 1       5
//  1 1 0       6
//  1 1 1       7
void AnnotationRenderer::_getDataMgrCorner(const int cornerNumber, glm::vec4 &dataMgrCorner, const std::vector<double> &minDataMgrExtents, const std::vector<double> &maxDataMgrExtents) const
{
    assert(minDataMgrExtents.size() == 3);
    assert(maxDataMgrExtents.size() == 3);
    assert(cornerNumber >= 0 && cornerNumber <= 7);

    double xCoord, yCoord, zCoord;
    if (cornerNumber & 0b100)
        xCoord = maxDataMgrExtents[X];
    else
        xCoord = minDataMgrExtents[X];

    if (cornerNumber & 0b010)
        yCoord = maxDataMgrExtents[Y];
    else
        yCoord = minDataMgrExtents[Y];

    if (cornerNumber & 0b001)
        zCoord = maxDataMgrExtents[Z];
    else
        zCoord = minDataMgrExtents[Z];

    dataMgrCorner = glm::vec4(xCoord, yCoord, zCoord, 1.f);
}

void AnnotationRenderer::_applyDataMgrToDomainExtents(std::vector<double> &domainExtents, const std::vector<double> &dataMgrMinExts, const std::vector<double> &dataMgrMaxExts,
                                                      const Transform *transform) const
{
    assert(domainExtents.size() == 6);
    assert(dataMgrMinExts.size() == 3);
    assert(dataMgrMaxExts.size() == 3);

    glm::mat4 transformMatrix;
    _makeTransformMatrix(transform, transformMatrix);

    glm::vec4 dataMgrCorner;
    for (int i = 0; i < 8; i++) {
        _getDataMgrCorner(i, dataMgrCorner, dataMgrMinExts, dataMgrMaxExts);

        _applyDataMgrCornerToDomain(domainExtents, dataMgrCorner, transformMatrix);
    }
}

void AnnotationRenderer::_calculateDomainExtents(std::vector<double> &domainExtents) const
{
    domainExtents = {NAN, NAN, NAN, NAN, NAN, NAN};

    vector<string> names = m_dataStatus->GetDataMgrNames();
    for (int i = 0; i < names.size(); i++) {
        std::vector<double> dataMgrMinExts, dataMgrMaxExts;

        m_dataStatus->GetActiveExtents(m_paramsMgr, m_winName, names[i], _currentTimestep, dataMgrMinExts, dataMgrMaxExts);
        VAssert(dataMgrMinExts.size() == 3);
        VAssert(dataMgrMaxExts.size() == 3);

        ViewpointParams *vpParams = m_paramsMgr->GetViewpointParams(m_winName);
        Transform *      transform = vpParams->GetTransform(names[i]);
        _applyDataMgrToDomainExtents(domainExtents, dataMgrMinExts, dataMgrMaxExts, transform);
    }

    for (int i = 0; i < 6; i++) {
        if (std::isnan(domainExtents[i])) domainExtents[i] = 0.f;
    }
}

void AnnotationRenderer::InScenePaint(size_t ts)
{
    AnnotationParams *vfParams = m_paramsMgr->GetAnnotationParams(m_winName);
    MatrixManager *   mm = _glManager->matrixManager;

    _currentTimestep = ts;

    ViewpointParams *vpParams = m_paramsMgr->GetViewpointParams(m_winName);

    std::vector<double> domainExtents;
    _calculateDomainExtents(domainExtents);

    _currentTimestep = ts;

    mm->MatrixModeModelView();
    mm->PushMatrix();

    if (vfParams->GetUseDomainFrame()) drawDomainFrame(domainExtents);

    vector<string> names = m_dataStatus->GetDataMgrNames();
    Transform *    t = vpParams->GetTransform(names[0]);
    applyTransform(t);

    double mvMatrix[16];
    mm->GetDoublev(MatrixManager::Mode::ModelView, mvMatrix);
    vpParams->SetModelViewMatrix(mvMatrix);

    AxisAnnotation *aa = vfParams->GetAxisAnnotation();
    if (aa->GetAxisAnnotationEnabled()) { drawAxisTics(aa); }

    mm->MatrixModeModelView();
    mm->PopMatrix();

    mm->GetDoublev(MatrixManager::Mode::ModelView, mvMatrix);
    vpParams->SetModelViewMatrix(mvMatrix);

    CheckGLErrorMsg(m_winName.c_str());
}

void AnnotationRenderer::scaleNormalizedCoordinatesToWorld(std::vector<double> &coords, string dataMgrName)
{
    std::vector<double> extents = getDomainExtents();
    int                 dims = extents.size() / 2;
    for (int i = 0; i < dims; i++) {
        double offset = coords[i] * (extents[i + dims] - extents[i]);
        double minimum = extents[i];
        coords[i] = offset + minimum;
    }
}

void AnnotationRenderer::drawAxisTics(AxisAnnotation *aa) { drawAxisTics(aa, std::vector<double>(), std::vector<double>()); }

void AnnotationRenderer::drawAxisTics(AxisAnnotation *aa, std::vector<double> minTic, std::vector<double> maxTic)
{
    if (aa == NULL) aa = getCurrentAxisAnnotation();

    if (minTic.empty()) minTic = aa->GetMinTics();
    if (maxTic.empty()) maxTic = aa->GetMaxTics();

    std::vector<double> origin = aa->GetAxisOrigin();

    string dmName = aa->GetDataMgrName();
    scaleNormalizedCoordinatesToWorld(origin, dmName);
    scaleNormalizedCoordinatesToWorld(minTic, dmName);
    scaleNormalizedCoordinatesToWorld(maxTic, dmName);

    vector<double> ticLength = aa->GetTicSize();
    vector<double> ticDir = aa->GetTicDirs();
    vector<double> numTics = aa->GetNumTics();
    vector<double> axisColor = aa->GetAxisColor();
    double         width = aa->GetTicWidth();
    bool           latLon = aa->GetLatLonAxesEnabled();

    _drawAxes(minTic, maxTic, origin, axisColor, width);

    double         pointOnAxis[3];
    double         ticVec[3];
    double         startPosn[3], endPosn[3];
    vector<double> extents = getDomainExtents();

    // Now draw tic marks for x:
    pointOnAxis[1] = origin[1];
    pointOnAxis[2] = origin[2];
    ticVec[0] = 0.f;
    ticVec[1] = 0.f;
    ticVec[2] = 0.f;
    double scaleFactor;
    if (ticDir[0] == 1) {    // Y orientation
        scaleFactor = extents[4] - extents[1];
        ticVec[1] = ticLength[0] * scaleFactor;
    } else {    // Z orientation
        scaleFactor = extents[5] - extents[2];
        ticVec[2] = ticLength[0] * scaleFactor;
    }
    // ticVec[1] = ticLength[1]*scaleFactor;
    // ticVec[2] = ticLength[2]*scaleFactor;
    for (int i = 0; i < numTics[0]; i++) {
        pointOnAxis[0] = minTic[0] + (float)i * (maxTic[0] - minTic[0]) / (float)(numTics[0] - 1);
        vsub(pointOnAxis, ticVec, startPosn);
        vadd(pointOnAxis, ticVec, endPosn);

        //_drawTic(startPosn, endPosn, length, width, axisColor);
        _drawTic(startPosn, endPosn, width, axisColor);

        double xValue = pointOnAxis[0];
        double yValue = pointOnAxis[1];
        if (latLon) convertPointToLonLat(xValue, yValue);
        renderText(xValue, startPosn, aa);
    }

    // Now draw tic marks for y:
    pointOnAxis[0] = origin[0];
    pointOnAxis[2] = origin[2];
    ticVec[0] = 0.f;
    ticVec[1] = 0.f;
    ticVec[2] = 0.f;
    if (ticDir[1] == 2) {    // Z orientation
        scaleFactor = extents[5] - extents[2];
        ticVec[2] = ticLength[1] * scaleFactor;
    } else {    // X orientation
        scaleFactor = extents[4] - extents[1];
        ticVec[0] = ticLength[1] * scaleFactor;
    }
    for (int i = 0; i < numTics[1]; i++) {
        pointOnAxis[1] = minTic[1] + (float)i * (maxTic[1] - minTic[1]) / (float)(numTics[1] - 1);
        vsub(pointOnAxis, ticVec, startPosn);
        vadd(pointOnAxis, ticVec, endPosn);

        _drawTic(startPosn, endPosn, width, axisColor);

        double xValue = pointOnAxis[0];
        double yValue = pointOnAxis[1];
        if (latLon) convertPointToLonLat(xValue, yValue);
        renderText(yValue, startPosn, aa);
    }

    // Now draw tic marks for z:
    pointOnAxis[0] = origin[0];
    pointOnAxis[1] = origin[1];
    ticVec[0] = 0.f;
    ticVec[1] = 0.f;
    ticVec[2] = 0.f;
    if (ticDir[2] == 1) {    // Y orientation
        scaleFactor = extents[4] - extents[1];
        ticVec[1] = ticLength[2] * scaleFactor;
    } else {    // X orientation
        scaleFactor = extents[3] - extents[0];
        ticVec[0] = ticLength[2] * scaleFactor;
    }
    for (int i = 0; i < numTics[2]; i++) {
        pointOnAxis[2] = minTic[2] + (float)i * (maxTic[2] - minTic[2]) / (float)(numTics[2] - 1);
        vsub(pointOnAxis, ticVec, startPosn);
        vadd(pointOnAxis, ticVec, endPosn);
        _drawTic(startPosn, endPosn, width, axisColor);
        renderText(pointOnAxis[2], startPosn, aa);
    }
}

void AnnotationRenderer::_drawAxes(std::vector<double> min, std::vector<double> max, std::vector<double> origin, std::vector<double> color, double width)
{
    LegacyGL *lgl = _glManager->legacy;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    lgl->Color4f(color[0], color[1], color[2], color[3]);
    // glLineWidth(width);
    glEnable(GL_LINE_SMOOTH);
    lgl->Begin(GL_LINES);
    lgl->Vertex3f(min[0], origin[1], origin[2]);
    lgl->Vertex3f(max[0], origin[1], origin[2]);
    lgl->Vertex3f(origin[0], min[1], origin[2]);
    lgl->Vertex3f(origin[0], max[1], origin[2]);
    lgl->Vertex3f(origin[0], origin[1], min[2]);
    lgl->Vertex3f(origin[0], origin[1], max[2]);
    lgl->End();
    glDisable(GL_LINE_SMOOTH);
}

void AnnotationRenderer::_drawTic(double startPosn[], double endPosn[], double width, std::vector<double> color)
{
    LegacyGL *lgl = _glManager->legacy;
    lgl->Color4f(color[0], color[1], color[2], color[3]);
    // glLineWidth(width);
    glEnable(GL_LINE_SMOOTH);
    lgl->Begin(GL_LINES);
    lgl->Vertex3dv(startPosn);
    lgl->Vertex3dv(endPosn);
    lgl->End();
    glDisable(GL_LINE_SMOOTH);
}

void AnnotationRenderer::convertPointToLonLat(double &xCoord, double &yCoord)
{
    double coords[2] = {xCoord, yCoord};
    double coordsForError[2] = {coords[0], coords[1]};

    string projString = m_dataStatus->GetMapProjection();
    int    rc = DataMgrUtils::ConvertPCSToLonLat(projString, coords, 1);
    if (!rc) { MyBase::SetErrMsg("Could not convert point %f, %f to Lon/Lat", coordsForError[0], coordsForError[1]); }

    xCoord = coords[0];
    yCoord = coords[1];
}

Transform *AnnotationRenderer::getTransform(string dataMgrName)
{
    if (dataMgrName == "") dataMgrName = getCurrentDataMgrName();

    ViewpointParams *vpParams = m_paramsMgr->GetViewpointParams(m_winName);
    vector<string>   names = m_paramsMgr->GetDataMgrNames();
    Transform *      t = vpParams->GetTransform(names[0]);
    return t;
}

AxisAnnotation *AnnotationRenderer::getCurrentAxisAnnotation()
{
    AnnotationParams *vfParams = m_paramsMgr->GetAnnotationParams(m_winName);
    string            currentAxisDataMgr = vfParams->GetCurrentAxisDataMgrName();
    AxisAnnotation *  aa = vfParams->GetAxisAnnotation();
    return aa;
}

string AnnotationRenderer::getCurrentDataMgrName() const
{
    AnnotationParams *vfParams = m_paramsMgr->GetAnnotationParams(m_winName);
    string            currentAxisDataMgr = vfParams->GetCurrentAxisDataMgrName();
    return currentAxisDataMgr;
}

std::vector<double> AnnotationRenderer::getDomainExtents() const
{
    int            ts = _currentTimestep;
    vector<double> minExts, maxExts;

    m_dataStatus->GetActiveExtents(m_paramsMgr, ts, minExts, maxExts);

    std::vector<double> extents;
    for (int i = 0; i < minExts.size(); i++) { extents.push_back(minExts[i]); }
    for (int i = 0; i < maxExts.size(); i++) { extents.push_back(maxExts[i]); }

    return extents;
}

void AnnotationRenderer::renderText(double text, double coord[], AxisAnnotation *aa)
{
    if (aa == NULL) aa = getCurrentAxisAnnotation();

    std::vector<double> axisColor = aa->GetAxisColor();
    std::vector<double> backgroundColor = aa->GetAxisBackgroundColor();
    int                 fontSize = aa->GetAxisFontSize();

    int               precision = (int)aa->GetAxisDigits();
    std::stringstream ss;
    ss << fixed << setprecision(precision) << text;
    string textString = ss.str();

    TextLabel label(_glManager, _fontName, fontSize);
    label.HorizontalAlignment = TextLabel::Center;
    label.VerticalAlignment = TextLabel::Top;
    label.Padding = fontSize / 4.f;
    label.ForegroundColor = glm::vec4(axisColor[0], axisColor[1], axisColor[2], axisColor[3]);
    label.BackgroundColor = glm::vec4(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
    label.DrawText(glm::vec3(coord[0], coord[1], coord[2]), textString);
}

// Find the world corrdinates of the user-selected screen coordinates, and translate the current matrix to that point
//
void AnnotationRenderer::_configureMatrixForArrows(MatrixManager *matrixManager)
{
    matrixManager->MatrixModeModelView();
    matrixManager->PushMatrix();


    // Calculate the pixel location on the screen from the user's value, which is between 0 and 1
    //
    AnnotationParams *vfParams = m_paramsMgr->GetAnnotationParams(m_winName);
    double            winX = vfParams->GetAxisArrowXPos();    // X position of arrows, between 0 and 1
    double            winY = vfParams->GetAxisArrowYPos();    // Y position of arrows, between 0 and 1
    // Scale the Params values by the window width/height
    std::vector<int> viewport = _glManager->GetViewport();
    winX = winX * viewport[2];    // viewport[2] is window width
    winY = winY * viewport[3];    // viewport[3] is window hight

    // Gather parameters for glm::unProject
    //
    // glm::unProject requires glm::mat4 for the modelview and projection matrices, so we need to
    // convert from vapor's array representation to glm::mat4
    double modelview[16], projection[16];
    _glManager->matrixManager->GetDoublev(MatrixManager::Mode::ModelView, modelview);
    _glManager->matrixManager->GetDoublev(MatrixManager::Mode::Projection, projection);
    glm::mat4 mat4modelview = glm::make_mat4(modelview);
    glm::mat4 mat4projection = glm::make_mat4(projection);

    // glm::unProject requres a glm::vec4 for the viewport, so we need to convert it from its std::vector<int>
    glm::vec4 vec4viewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    // Now un-project to find the world coordinates of the selected pixel, and and translate to it for drawing
    //
    glm::vec3 win = {winX, winY, .5};
    glm::vec3 coords = glm::unProject(win, mat4modelview, mat4projection, vec4viewport);
    _glManager->matrixManager->Translate(coords[0], coords[1], coords[2]);

    // Finally, apply an scale factor that cancels the scaling done when zooming in and out, so that the arrows
    // retain a constant size on the screen.  This code was derived from
    // https://gamedev.stackexchange.com/questions/24968/constant-size-geometries with the exception of using
    // glm::unproject instead of the deprecated gluUnProject function.
    //
    const double fov = m_paramsMgr->GetViewpointParams(m_winName)->GetFOV();
    double       cameraPosD[3], cameraUpD[3], cameraDirD[3];
    m_paramsMgr->GetViewpointParams(m_winName)->ReconstructCamera(modelview, cameraPosD, cameraUpD, cameraDirD);
    glm::vec3 cameraPos = glm::vec3(cameraPosD[0], cameraPosD[1], cameraPosD[2]);
    float     cameraObjectDistance = sqrt(pow(cameraPos[0] - coords[0], 2) + pow(cameraPos[1] - coords[1], 2) + pow(cameraPos[2] - coords[2], 2));
    float     worldSize = (2 * tan(fov / 2.0)) * cameraObjectDistance;
    float     size = vfParams->GetAxisArrowSize() * worldSize * ARROW_SCALE_FACTOR;
    matrixManager->Scale(size, size, size);
}

void AnnotationRenderer::DrawAxisArrows()
{
    AnnotationParams *vfParams = m_paramsMgr->GetAnnotationParams(m_winName);
    if (!vfParams->GetAxisArrowEnabled()) { return; }

    LegacyGL *     lgl = _glManager->legacy;
    MatrixManager *mm = _glManager->matrixManager;

    _configureMatrixForArrows(mm);

    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    // Begin drawing
    //
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    lgl->Color3f(1, 0, 0);
    glEnable(GL_LINE_SMOOTH);

    lgl->Begin(GL_LINES);
    lgl->Vertex3f(0, 0, 0);
    lgl->Vertex3f(1, 0, 0);
    lgl->End();

    lgl->Begin(GL_TRIANGLES);
    lgl->Vertex3f(1, 0, 0);
    lgl->Vertex3f(.8, .1, 0);
    lgl->Vertex3f(.8, 0, .1);

    lgl->Vertex3f(1, 0, 0);
    lgl->Vertex3f(.8, 0, .1);
    lgl->Vertex3f(.8, -.1, 0);

    lgl->Vertex3f(1, 0, 0);
    lgl->Vertex3f(.8, -.1, 0);
    lgl->Vertex3f(.8, 0, -.1);

    lgl->Vertex3f(1, 0, 0);
    lgl->Vertex3f(.8, 0, -.1);
    lgl->Vertex3f(.8, .1, 0);
    lgl->End();

    lgl->Color3f(0, 1, 0);
    lgl->Begin(GL_LINES);
    lgl->Vertex3f(0, 0, 0);
    lgl->Vertex3f(0, 1, 0);
    lgl->End();

    lgl->Begin(GL_TRIANGLES);
    lgl->Vertex3f(0, 1, 0);
    lgl->Vertex3f(.1, .8, 0);
    lgl->Vertex3f(0, .8, .1);

    lgl->Vertex3f(0, 1, 0);
    lgl->Vertex3f(0, .8, .1);
    lgl->Vertex3f(-.1, .8, 0);

    lgl->Vertex3f(0, 1, 0);
    lgl->Vertex3f(-.1, .8, 0);
    lgl->Vertex3f(0, .8, -.1);

    lgl->Vertex3f(0, 1, 0);
    lgl->Vertex3f(0, .8, -.1);
    lgl->Vertex3f(.1, .8, 0);
    lgl->End();

    lgl->Color3f(0, 0.3, 1);
    lgl->Begin(GL_LINES);
    lgl->Vertex3f(0, 0, 0);
    lgl->Vertex3f(0, 0, 1);
    lgl->End();

    lgl->Begin(GL_TRIANGLES);
    lgl->Vertex3f(0, 0, 1);
    lgl->Vertex3f(.1, 0, .8);
    lgl->Vertex3f(0, .1, .8);

    lgl->Vertex3f(0, 0, 1);
    lgl->Vertex3f(0, .1, .8);
    lgl->Vertex3f(-.1, 0, .8);

    lgl->Vertex3f(0, 0, 1);
    lgl->Vertex3f(-.1, 0, .8);
    lgl->Vertex3f(0, -.1, .8);

    lgl->Vertex3f(0, 0, 1);
    lgl->Vertex3f(0, -.1, .8);
    lgl->Vertex3f(.1, 0, .8);
    lgl->End();

    glDepthRange(0, 1.0);

    mm->PopMatrix();
    glDisable(GL_LINE_SMOOTH);
}
