/************************************************************************/
//									*
//			 Copyright (C)  2005				*
//	 University Corporation for Atmospheric Research			*
//			 All Rights Reserved				*
//									*
//************************************************************************/
//
//	File:		Manip.cpp
//
//	Author:		Alan Norton, Scott Pearse
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		June, 2018
//
//	Description:	Implements the Manip class and some of its subclasses

#include <iterator>
#include <iomanip>
#include <iostream>
#include <cassert>
#include <cmath>

#include "Manip.h"
#include "vapor/GLManager.h"
#include "vapor/LegacyGL.h"

using namespace VAPoR;

const float Manip::_faceSelectionColor[4] = {0.8f, 0.8f, 0.0f, 0.5f};
const float Manip::_unselectedFaceColor[4] = {0.8f, 0.2f, 0.0f, 0.5f};

#ifndef M_PI
    #define M_PI (3.14159265358979323846)
#endif

// X, Y, and Z axes are indexed 0, 1, and 2
#define X 0
#define Y 1
#define Z 2

// 6 element arrays that describe 3D extents have the ordering
//   MINX, MINY, MINZ, MAXX, MAXY, MAXZ
#define MINX 0
#define MINY 1
#define MINZ 2
#define MAXX 3
#define MAXY 4
#define MAXZ 5

TranslateStretchManip::TranslateStretchManip(GLManager *glManager) : Manip(glManager)
{
    _buttonNum = 0;
    _selectedHandle = -1;
    _isStretching = false;
    _tempRotation = 0.f;
    _tempRotAxis = -1;
    _handleSizeInScene = 1.;
    _dragDistance = 0.f;
    _mouseDownHere = false;

    for (int i = 0; i < 3; i++) {
        _initialSelectionRay[i] = 0.;
        _cameraPosition[i] = 0.;
        _handleMid[i] = 0.f;

        _selection[i] = 0.;
        _selection[i + 3] = 0.;
        _extents[i] = 0.;
        _extents[i + 3] = 0.;
    }
}

void TranslateStretchManip::transformMatrix(VAPoR::Transform *transform)
{
    MatrixManager *mm = _glManager->matrixManager;

    mm->MatrixModeModelView();

    // Use the ModelViewMatrix passed in upon Update(),
    // not what is currently held in the gl state
    mm->LoadMatrixd(_modelViewMatrix);

    mm->PushMatrix();

    if (transform == NULL) return;

    vector<double> translations = transform->GetTranslations();
    vector<double> rotations = transform->GetRotations();
    vector<double> scales = transform->GetScales();
    vector<double> origins = transform->GetOrigin();
    assert(translations.size() == 3);
    assert(rotations.size() == 3);
    assert(scales.size() == 3);
    assert(origins.size() == 3);

    mm->Translate(origins[X], origins[Y], origins[Z]);
    mm->Scale(scales[X], scales[Y], scales[Z]);
    mm->Rotate(rotations[X], 1, 0, 0);
    mm->Rotate(rotations[Y], 0, 1, 0);
    mm->Rotate(rotations[Z], 0, 0, 1);
    mm->Translate(-origins[X], -origins[Y], -origins[Z]);

    mm->Translate(translations[X], translations[Y], translations[Z]);

    // Retrieve the transformed matrix and stick it back into
    // our _modelViewMatrix array
    mm->GetDoublev(MatrixManager::Mode::ModelView, _modelViewMatrix);
}

void TranslateStretchManip::Update(std::vector<double> llc, std::vector<double> urc, std::vector<double> minExts, std::vector<double> maxExts, VAPoR::Transform *rpTransform,
                                   VAPoR::Transform *dmTransform, bool constrain)
{
    MatrixManager *mm = _glManager->matrixManager;
    mm->GetDoublev(MatrixManager::Mode::Projection, _projectionMatrix);
    mm->GetDoublev(MatrixManager::Mode::ModelView, _modelViewMatrix);

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    _windowSize[0] = viewport[2];
    _windowSize[1] = viewport[3];

    double minv[16];
    int    rc = minvert(_modelViewMatrix, minv);
    assert(rc >= 0);

    _cameraPosition[X] = minv[12];
    _cameraPosition[Y] = minv[13];
    _cameraPosition[Z] = minv[14];

    if (llc.size() == 2) llc.push_back(0);
    if (urc.size() == 2) urc.push_back(0);
    if (minExts.size() == 2) minExts.push_back(0);
    if (maxExts.size() == 2) maxExts.push_back(0);

    std::copy(llc.begin(), llc.end(), _selection);
    std::copy(urc.begin(), urc.end(), _selection + 3);
    std::copy(minExts.begin(), minExts.end(), _extents);
    std::copy(maxExts.begin(), maxExts.end(), _extents + 3);

    _rpTransform = rpTransform;
    _dmTransform = dmTransform;

    _constrain = constrain;
}

void TranslateStretchManip::GetBox(std::vector<double> &llc, std::vector<double> &urc) const
{
    llc.resize(3);
    urc.resize(3);
    llc[X] = _selection[MINX];
    llc[Y] = _selection[MINY];
    llc[Z] = _selection[MINZ];
    urc[X] = _selection[MAXX];
    urc[Y] = _selection[MAXY];
    urc[Z] = _selection[MAXZ];
}

bool TranslateStretchManip::MouseEvent(int buttonNum, std::vector<double> vscreenCoords, double handleMidpoint[3], bool release)
{
    double screenCoords[2] = {vscreenCoords[0], vscreenCoords[1]};

    if (_selectedHandle < 0) _selectedHandle = _mouseIsOverHandle(screenCoords, handleMidpoint);

    if (_selectedHandle < 0) return false;

    if (release) {    // Release
        _mouseRelease(screenCoords);
    } else if (buttonNum == _buttonNum) {    // Dragging
        if (!_mouseDownHere) return false;
        _mouseDrag(screenCoords, handleMidpoint);
    } else if (_buttonNum == 0) {    // Press
        _mousePress(screenCoords, handleMidpoint, buttonNum);
    }

    return true;
}

void TranslateStretchManip::_mouseDrag(double screenCoords[2], double handleMidpoint[3])
{
    if (_selectedHandle >= 0) {
        double projScreenCoords[2];
        bool   success = projectPointToLine(screenCoords, projScreenCoords);
        if (success) {
            double dirVec[3];
            pixelToVector(projScreenCoords, dirVec, handleMidpoint);
            slideHandle(_selectedHandle, dirVec, false);
        }
    }
}

void TranslateStretchManip::_mousePress(double screenCoords[2], double handleMidpoint[3], int buttonNum)
{
    double dirVec[3];
    pixelToVector(screenCoords, dirVec, handleMidpoint);
    _captureMouseDown(_selectedHandle, buttonNum, handleMidpoint);
    startHandleSlide(screenCoords, _selectedHandle);
    setMouseDown(true);
}

void TranslateStretchManip::_mouseRelease(double screenCoords[2])
{
    if (_selectedHandle >= 0) {
        int axis = (_selectedHandle < 3) ? (2 - _selectedHandle) : (_selectedHandle - 3);
        // Convert _dragDistance to world coords:
        float dist = _dragDistance;

        // Check if we are stretching.  If so, only move coords associated with
        // handle:
        if (_isStretching) {
            // boxMin gets changed for nearHandle, boxMax for farHandle
            if (_selectedHandle < 3) {
                _selection[axis] += dist;
            } else {
                _selection[axis + 3] += dist;
            }
        } else {
            // boxMin gets changed for nearHandle, boxMax for farHandle
            _selection[axis] += dist;
            _selection[axis + 3] += dist;
        }
    }

    if (_constrain) _constrainExtents();

    _dragDistance = 0.f;
    _selectedHandle = -1;
    _buttonNum = 0;
    setMouseDown(false);
}

int TranslateStretchManip::_mouseIsOverHandle(const double screenCoords[2], double handleMid[3]) const
{
    double handle[8][3];

    int octant = 0;
    int face, handleNum;
    for (int axis = 0; axis < 3; axis++) {
        double axisBoundary = 0.5f * (_selection[axis] + _selection[axis + 3]);
        if (_cameraPosition[axis] > axisBoundary) { octant |= 1 << axis; }
    }

    // Front handles
    for (int sortNum = 0; sortNum < 3; sortNum++) {
        handleNum = makeHandleFaces(sortNum, handle, octant, _selection);
        if ((face = pointIsOnBox(handle, screenCoords)) >= 0) {
            for (int i = 0; i < 3; i++) {
                handleMid[i] = 0.;
                for (int k = 0; k < 8; k++) handleMid[i] += handle[k][i];
                handleMid[i] /= 8.;
            }
            return handleNum;
        }
    }

    // Back handles
    for (int sortNum = 3; sortNum < 6; sortNum++) {
        handleNum = makeHandleFaces(sortNum, handle, octant, _selection);
        if ((face = pointIsOnBox(handle, screenCoords)) >= 0) {
            for (int i = 0; i < 3; i++) {
                handleMid[i] = 0.;
                for (int k = 0; k < 8; k++) handleMid[i] += handle[k][i];
                handleMid[i] /= 8.;
            }
            return handleNum;
        }
    }
    return -1;
}

void TranslateStretchManip::_constrainExtents()
{
    for (int i = 0; i < 3; i++) {
        // correct selection minimum
        if (_selection[i] < _extents[i]) _selection[i] = _extents[i];
        if (_selection[i] > _extents[i + 3]) _selection[i] = _extents[i + 3];

        // correct selection maximum
        if (_selection[i + 3] < _extents[i]) _selection[i + 3] = _extents[i];
        if (_selection[i + 3] > _extents[i + 3]) _selection[i + 3] = _extents[i + 3];
    }
}

bool TranslateStretchManip::pointIsOnQuad(double cor1[3], double cor2[3], double cor3[3], double cor4[3], const double pickPt[2]) const
{
    double winCoord1[2];
    double winCoord2[2];
    double winCoord3[2];
    double winCoord4[2];
    bool   returnValue = true;
    if (!_projectPointToWin(cor1, winCoord1)) returnValue = false;
    if (!_projectPointToWin(cor2, winCoord2)) returnValue = false;
    if (pointOnRight(winCoord1, winCoord2, pickPt)) returnValue = false;
    if (!_projectPointToWin(cor3, winCoord3)) returnValue = false;
    if (pointOnRight(winCoord2, winCoord3, pickPt)) returnValue = false;
    if (!_projectPointToWin(cor4, winCoord4)) returnValue = false;
    if (pointOnRight(winCoord3, winCoord4, pickPt)) returnValue = false;
    if (pointOnRight(winCoord4, winCoord1, pickPt)) returnValue = false;

#ifdef DEBUG
    drawHitBox(winCoord1, winCoord2, winCoord3, winCoord4);
#endif

    return returnValue;
}

int TranslateStretchManip::pointIsOnBox(double corners[8][3], const double pickPt[2]) const
{
    // front (-Z)
    if (pointIsOnQuad(corners[0], corners[1], corners[3], corners[2], pickPt)) return 2;
    // back (+Z)
    if (pointIsOnQuad(corners[4], corners[6], corners[7], corners[5], pickPt)) return 3;
    // right (+X)
    if (pointIsOnQuad(corners[1], corners[5], corners[7], corners[3], pickPt)) return 5;
    // left (-X)
    if (pointIsOnQuad(corners[0], corners[2], corners[6], corners[4], pickPt)) return 0;
    // top (+Y)
    if (pointIsOnQuad(corners[2], corners[3], corners[7], corners[6], pickPt)) return 4;
    // bottom (-Y)
    if (pointIsOnQuad(corners[0], corners[4], corners[5], corners[1], pickPt)) return 1;
    return -1;
}

// Construct one of 6 cube-handles.  The first 3 (0,1,2) are those in front of the probe
// the octant (between 0 and 7) is a binary representation of the viewer relative to
// the probe center.  In binary, this value is ZYX, where Z is 1 if the viewer is above,
// 0 if the viewer is below, similarly for Y and X.  The handles are numbered from 0 to 5,
// in the order -X, +X, -Y, +Y, -Z, +Z.
//
// We provide the handle in the appropriate sort order, based on the octant
// in which the viewer is located.  The sort position of a handle is either
// N or (5-N) where N is the order sorted on x,y,z axes.  It is 5-N if the
// corresponding bit in the octant is 1.  Octant is between 0 and 7
// with a bit for each axis (Z,Y,X order).
// Return value is the absolute handle index, which may differ from the sort order.
// If you want the faces in absolute order, just specify octant = 0
//
// We make the handles in cube coords, since rendering and picking use that system.
// This requires that we have boxCenter and boxWidth in cube coords.
// HANDLE_DIAMETER is measured in pixels
// These can be calculated from the boxRegion
int TranslateStretchManip::makeHandleFaces(int sortPosition, double handle[8][3], int octant, const double boxRegion[6]) const
{
    // Identify the axis this handle is on:
    int axis = (sortPosition < 3) ? (2 - sortPosition) : (sortPosition - 3);
    int newPosition = sortPosition;

    // If octant and axis share their sign, we invert the sortPosition to get newPosition...?
    if ((octant >> axis) & 1) newPosition = 5 - sortPosition;

    // Now create the cube associated with newPosition.  It's just the handle translated
    // in the direction associated with newPosition
    float translateSign = (newPosition > 2) ? 1.f : -1.f;
    for (int vertex = 0; vertex < 8; vertex++) {
        for (int coord = 0; coord < 3; coord++) {
            float worldHandleDiameter = _handleSizeInScene;

            // Obtain the coordinate of unit cube corner.  It's either +0.5 or -0.5
            // multiplied by the handle diameter, then translated along the
            // specific axis corresponding to
            double fltCoord = (((double)((vertex >> coord) & 1) - 0.5f) * worldHandleDiameter);    //_handleSizeInScene);
            // First offset it from the probeCenter:
            fltCoord += 0.5f * (boxRegion[coord + 3] + boxRegion[coord]);

            deScaleScalarOnAxis(worldHandleDiameter, coord);

            // Displace all the c - coords of this handle if this handle is on the c-axis
            if (coord == axis) {
                double boxWidth = (boxRegion[coord + 3] - boxRegion[coord]);
                // Note we are putting the handle 2 diameters from the box edge
                fltCoord += translateSign * (boxWidth * 0.5f + 2.f * worldHandleDiameter);    //_handleSizeInScene);
            }
            handle[vertex][coord] = fltCoord;
        }
    }
    deScaleExtents(handle);
    return newPosition;
}

bool TranslateStretchManip::startHandleSlide(const double mouseCoords[2], int handleNum)
{
    // When the mouse is first pressed over a handle,
    // need to save the
    // windows coordinates of the click, as well as
    // calculate a 2D unit vector in the direction of the slide,
    // projected into the window.

    _mouseDownPoint[0] = mouseCoords[0];
    _mouseDownPoint[1] = mouseCoords[1];

    // Get the cube coords of the rotation center:
    double boxCtr[3];
    double winCoords[2] = {0., 0.};
    double dispCoords[2];

    if (handleNum > 2)
        handleNum = handleNum - 3;
    else
        handleNum = 2 - handleNum;

    for (int i = 0; i < 3; i++) { boxCtr[i] = (_selection[i] + _selection[i + 3]) * 0.5f; }
    // project the boxCtr and one more point, to get a direction vector

    if (!_projectPointToWin(boxCtr, winCoords)) return false;

    boxCtr[handleNum] += 0.1f;

    if (!_projectPointToWin(boxCtr, dispCoords)) return false;

    // Direction vector is difference:
    _handleProjVec[0] = dispCoords[0] - winCoords[0];
    _handleProjVec[1] = dispCoords[1] - winCoords[1];
    float vecNorm = sqrt(_handleProjVec[0] * _handleProjVec[0] + _handleProjVec[1] * _handleProjVec[1]);

    if (vecNorm == 0.f) return false;

    _handleProjVec[0] /= vecNorm;
    _handleProjVec[1] /= vecNorm;
    return true;
}

// Project the current mouse coordinates to a line in screen space.
// The line starts at the mouseDownPosition, and points in the
// direction resulting from projecting to the screen the axis
// associated with the dragHandle.  Returns false on error.
bool TranslateStretchManip::projectPointToLine(const double mouseCoords[2], double projCoords[2])
{
    //  State saved at a mouse press is:
    //	mouseDownPoint[2] = P
    //  _handleProjVec[2] unit vector (U)
    //
    // When the mouse is moved, project to the line:
    // point Q projects to P + aU, where a = (Q-P).U = dotprod
    double diff[2];
    if (!_mouseDownHere) return false;
    diff[0] = mouseCoords[0] - _mouseDownPoint[0];
    diff[1] = mouseCoords[1] - _mouseDownPoint[1];
    double dotprod = diff[0] * _handleProjVec[0] + diff[1] * _handleProjVec[1];
    projCoords[0] = _mouseDownPoint[0] + dotprod * _handleProjVec[0];
    projCoords[1] = _mouseDownPoint[1] + dotprod * _handleProjVec[1];

    return true;
}

// Convert a screen coord to a vector, representing the displacedment
// from the camera associated with the screen coords.  Note screen coords
// are OpenGL style.  strHandleMid is in stretched coordinates.
//
bool TranslateStretchManip::pixelToVector(double winCoords[2], double dirVec[3], const double strHandleMid[3])
{
    GLdouble pt[3];
    // Project handleMid to find its z screen coordinate:
    GLdouble screenx, screeny, screenz;

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    gluProject(strHandleMid[0], strHandleMid[1], strHandleMid[2], _modelViewMatrix, _projectionMatrix, viewport, &screenx, &screeny, &screenz);
    // Obtain the coords of a point in view:
    bool success = (0 != gluUnProject((GLdouble)winCoords[0], (GLdouble)winCoords[1], screenz, _modelViewMatrix, _projectionMatrix, viewport, pt, pt + 1, pt + 2));
    if (success) {
        // Subtract camera coords to get a direction vector:
        vsub(pt, _cameraPosition, dirVec);
    }
    GL_ERR_BREAK();
    return success;
}

// Find the handle extents using the boxExtents in world coords
// Set the octant to be 0 if the sortPosition is just the
// handleNum
// If this is on the same axis as the selected handle it is displaced by _dragDistance
//

void TranslateStretchManip::makeHandleExtents(int sortPosition, double handleExtents[6], int octant, double boxExtents[6])
{
    // Identify the axis this handle is on:
    int axis = (sortPosition < 3) ? (2 - sortPosition) : (sortPosition - 3);
    int newPosition = sortPosition;
    if ((octant >> axis) & 1) newPosition = 5 - sortPosition;

    // Now create the cube associated with newPosition.  It's just the handle translated
    // up or down in the direction associated with newPosition
    for (int coord = 0; coord < 3; coord++) {
        float worldHandleDiameter = _handleSizeInScene;
        // Start at the box center position
        handleExtents[coord] = .5f * (-worldHandleDiameter + (boxExtents[coord + 3] + boxExtents[coord]));
        handleExtents[coord + 3] = .5f * (worldHandleDiameter + (boxExtents[coord + 3] + boxExtents[coord]));

        deScaleScalarOnAxis(worldHandleDiameter, coord);

        if (coord == axis) {    // Translate up or down along this axis
            // The translation is 2 handles + .5 box thickness
            double boxWidth = (boxExtents[coord + 3] - boxExtents[coord]);
            if (newPosition < 3) {    //"low" handles are shifted down in the coord:
                handleExtents[coord] -= (boxWidth * 0.5f + 2.f * worldHandleDiameter);
                handleExtents[coord + 3] -= (boxWidth * 0.5f + 2.f * worldHandleDiameter);
            } else {
                handleExtents[coord] += (boxWidth * 0.5f + 2.f * worldHandleDiameter);
                handleExtents[coord + 3] += (boxWidth * 0.5f + 2.f * worldHandleDiameter);
            }
        }
    }
    deScaleExtents(handleExtents);
    return;
}

void TranslateStretchManip::deScaleScalarOnAxis(float &whd, int axis) const
{
    if (_dmTransform == NULL) return;
    if (_rpTransform == NULL) return;

    vector<double> dmScales = _dmTransform->GetScales();
    vector<double> rpScales = _rpTransform->GetScales();

    whd /= dmScales[axis];
    whd /= rpScales[axis];
}

// If we are deScaling a cube of the form extents[8][3], then that
// cube will (apparently) contain its corner coordinates in the following form:
//		j = 0   1   2   (XYZ)
//	  i
//	  0	   min min min
//	  1	   max min min
//	  2	   min max min
//	  3	   max max min
//	  4	   min min max
//	  5	   max min max
//	  6	   min max max
//	  7	   max max max
//
//  This lookup table is how the extents are being rescaled in the for
//  loop below.
//
void TranslateStretchManip::deScaleExtents(double extents[8][3]) const
{
    if (_dmTransform == NULL) return;
    if (_rpTransform == NULL) return;

    vector<double> dmScales = _dmTransform->GetScales();
    vector<double> rpScales = _rpTransform->GetScales();

    assert(rpScales.size() == 3);
    assert(dmScales.size() == 3);

    double size, midpoint, min, max;

    for (int i = 0; i < 3; i++) {
        // Vertex 0 contains the XYZ minimum coordinates
        // Vertex 7 contains the XYZ maximum coordinates
        size = extents[7][i] - extents[0][i];
        size /= dmScales[i];
        size /= rpScales[i];
        midpoint = (extents[7][i] + extents[0][i]) / 2.f;

        min = midpoint - size / 2.f;
        max = midpoint + size / 2.f;

        for (int j = 0; j < 8; j++) {
            if (i == 0) {    // X axis
                if (j % 2)
                    extents[j][i] = min;
                else
                    extents[j][i] = max;
            } else if (i == 1) {    // Y axis
                if (j / 2 % 2)
                    extents[j][i] = min;
                else
                    extents[j][i] = max;
            } else if (i == 2) {    // Z axis
                if (j < 4)
                    extents[j][i] = min;
                else
                    extents[j][i] = max;
            }
        }
    }
}

void TranslateStretchManip::deScaleExtents(double *extents) const
{
    if (_dmTransform == NULL) return;
    if (_rpTransform == NULL) return;

    vector<double> dmScales = _dmTransform->GetScales();
    vector<double> rpScales = _rpTransform->GetScales();

    assert(rpScales.size() == 3);
    assert(dmScales.size() == 3);

    double size, midpoint;
    for (int i = 0; i < 3; i++) {
        size = extents[i + 3] - extents[i];
        midpoint = (extents[i + 3] + extents[i]) / 2.f;
        size /= dmScales[i];
        size /= rpScales[i];
        extents[i] = midpoint - size / 2.f;
        extents[i + 3] = midpoint + size / 2.f;
    }
}

// Draw all the faces of a cube with specified extents.
// Currently just used for handles.
void TranslateStretchManip::drawCubeFaces(double *extents, bool isSelected)
{
    LegacyGL *lgl = _glManager->legacy;

    GLfloat lineWidthRange[2] = {0.0f, 0.0f};
    glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Do left (x=0)
    if (isSelected)
        lgl->Color4fv(_faceSelectionColor);
    else
        lgl->Color4fv(_unselectedFaceColor);
    lgl->Begin(LGL_QUADS);
    lgl->Vertex3f(extents[MINX], extents[MINY], extents[MINZ]);
    lgl->Vertex3f(extents[MINX], extents[MINY], extents[MAXZ]);
    lgl->Vertex3f(extents[MINX], extents[MAXY], extents[MAXZ]);
    lgl->Vertex3f(extents[MINX], extents[MAXY], extents[MINZ]);
    lgl->End();

    // do right
    if (isSelected)
        lgl->Color4fv(_faceSelectionColor);
    else
        lgl->Color4fv(_unselectedFaceColor);
    lgl->Begin(LGL_QUADS);
    lgl->Vertex3f(extents[MAXX], extents[MINY], extents[MINZ]);
    lgl->Vertex3f(extents[MAXX], extents[MINY], extents[MAXZ]);
    lgl->Vertex3f(extents[MAXX], extents[MAXY], extents[MAXZ]);
    lgl->Vertex3f(extents[MAXX], extents[MAXY], extents[MINZ]);
    lgl->End();

    // top
    if (isSelected)
        lgl->Color4fv(_faceSelectionColor);
    else
        lgl->Color4fv(_unselectedFaceColor);
    lgl->Begin(LGL_QUADS);
    lgl->Vertex3f(extents[MINX], extents[MAXY], extents[MINZ]);
    lgl->Vertex3f(extents[MAXX], extents[MAXY], extents[MINZ]);
    lgl->Vertex3f(extents[MAXX], extents[MAXY], extents[MAXZ]);
    lgl->Vertex3f(extents[MINX], extents[MAXY], extents[MAXZ]);
    lgl->End();

    // bottom
    if (isSelected)
        lgl->Color4fv(_faceSelectionColor);
    else
        lgl->Color4fv(_unselectedFaceColor);
    lgl->Begin(LGL_QUADS);
    lgl->Vertex3f(extents[MINX], extents[MINY], extents[MINZ]);
    lgl->Vertex3f(extents[MINX], extents[MINY], extents[MAXZ]);
    lgl->Vertex3f(extents[MAXX], extents[MINY], extents[MAXZ]);
    lgl->Vertex3f(extents[MAXX], extents[MINY], extents[MINZ]);
    lgl->End();

    // back
    if (isSelected)
        lgl->Color4fv(_faceSelectionColor);
    else
        lgl->Color4fv(_unselectedFaceColor);
    lgl->Begin(LGL_QUADS);
    lgl->Vertex3f(extents[MINX], extents[MINY], extents[MINZ]);
    lgl->Vertex3f(extents[MAXX], extents[MINY], extents[MINZ]);
    lgl->Vertex3f(extents[MAXX], extents[MAXY], extents[MINZ]);
    lgl->Vertex3f(extents[MINX], extents[MAXY], extents[MINZ]);
    lgl->End();

    // do the front:
    //
    if (isSelected)
        lgl->Color4fv(_faceSelectionColor);
    else
        lgl->Color4fv(_unselectedFaceColor);
    lgl->Begin(LGL_QUADS);
    lgl->Vertex3f(extents[MINX], extents[MINY], extents[MAXZ]);
    lgl->Vertex3f(extents[MAXX], extents[MINY], extents[MAXZ]);
    lgl->Vertex3f(extents[MAXX], extents[MAXY], extents[MAXZ]);
    lgl->Vertex3f(extents[MINX], extents[MAXY], extents[MAXZ]);
    lgl->End();

    lgl->Color4f(1, 1, 1, 1);
    glDisable(GL_BLEND);
}

// Renders handles and box
// If it is stretching, it only moves the one handle that is doing the stretching
void TranslateStretchManip::Render()
{
    transformMatrix(_dmTransform);
    if (_rpTransform != NULL) transformMatrix(_rpTransform);

    _handleSizeInScene = getPixelSize() * (float)HANDLE_DIAMETER;

    double handleExtents[6];
    for (int handleNum = 0; handleNum < 6; handleNum++) {
        makeHandleExtents(handleNum, handleExtents, 0 /*octant*/, _selection);

        if (_selectedHandle >= 0) {
            int axis = (_selectedHandle < 3) ? (2 - _selectedHandle) : (_selectedHandle - 3);
            // displace handleExtents appropriately
            if (_isStretching) {
                // modify the extents for the one grabbed handle
                if (_selectedHandle == handleNum) {
                    handleExtents[axis] += _dragDistance;
                    handleExtents[axis + 3] += _dragDistance;
                }    // and make the handles on the non-grabbed axes move half as far:
                else if (handleNum != (5 - _selectedHandle)) {
                    handleExtents[axis] += 0.5f * _dragDistance;
                    handleExtents[axis + 3] += 0.5f * _dragDistance;
                }
            } else {
                handleExtents[axis] += _dragDistance;
                handleExtents[axis + 3] += _dragDistance;
            }
        }
        drawCubeFaces(handleExtents, (handleNum == _selectedHandle));
        drawHandleConnector(handleNum, handleExtents, _selection);
    }
    // Then render the full box, unhighlighted and displaced
    drawBoxFaces();

    MatrixManager *mm = _glManager->matrixManager;

    mm->PopMatrix();             // Pop the matrix applied for the DataMgr's transform
    if (_rpTransform != NULL)    // Pop the matrix applied for the RenderParams' transform
        mm->PopMatrix();
}

double TranslateStretchManip::getPixelSize() const
{
    double temp[3];

    // Window height is subtended by viewing angle (45 degrees),
    // at viewer distance (dist from camera to view center)

    size_t height = _windowSize[1];

    double              origin[3];
    std::vector<double> vorigin(3, 0.f);
    if (_dmTransform != NULL) vorigin = _dmTransform->GetOrigin();

    for (int i = 0; i < 3; i++) { origin[i] = vorigin[i]; }

    vsub(origin, _cameraPosition, temp);

    float distToScene = vlength(temp);
    // tan(45 deg *0.5) is ratio between half-height and dist to scene
    double halfHeight = tan(M_PI * 0.125) * distToScene;
    return (2.f * halfHeight / (double)height);
}

// Draw the main box, just rendering the lines.
// the highlightedFace is not the same as the selectedFace!!
//

void TranslateStretchManip::drawBoxFaces() const
{
    double corners[8][3];

    // -X -Y -Z
    corners[0][X] = _selection[MINX];
    corners[0][Y] = _selection[MINY];
    corners[0][Z] = _selection[MINZ];

    // -X -Y +Z
    corners[1][X] = _selection[MINX];
    corners[1][Y] = _selection[MINY];
    corners[1][Z] = _selection[MAXZ];

    // -X +Y -Z
    corners[2][X] = _selection[MINX];
    corners[2][Y] = _selection[MAXY];
    corners[2][Z] = _selection[MINZ];

    // -X +Y +Z
    corners[3][X] = _selection[MINX];
    corners[3][Y] = _selection[MAXY];
    corners[3][Z] = _selection[MAXZ];

    // +X -Y -Z
    corners[4][X] = _selection[MAXX];
    corners[4][Y] = _selection[MINY];
    corners[4][Z] = _selection[MINZ];

    // +X -Y +Z
    corners[5][X] = _selection[MAXX];
    corners[5][Y] = _selection[MINY];
    corners[5][Z] = _selection[MAXZ];

    // +X +Y -Z
    corners[6][X] = _selection[MAXX];
    corners[6][Y] = _selection[MAXY];
    corners[6][Z] = _selection[MINZ];

    // +X +Y +Z
    corners[7][X] = _selection[MAXX];
    corners[7][Y] = _selection[MAXY];
    corners[7][Z] = _selection[MAXZ];

    if (_selectedHandle >= 0) {
        if (_isStretching)
            _stretchCorners(corners);
        else if (_dragDistance != 0.)
            _translateCorners(corners);
    }

    // Now render the edges:
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    GL_LEGACY(glLineWidth(2.0));

    LegacyGL *lgl = _glManager->legacy;

    lgl->Color3f(1.f, 0.f, 0.f);
    lgl->Begin(GL_LINES);
    lgl->Vertex3dv(corners[0]);
    lgl->Vertex3dv(corners[1]);
    lgl->Vertex3dv(corners[0]);
    lgl->Vertex3dv(corners[2]);
    lgl->Vertex3dv(corners[0]);
    lgl->Vertex3dv(corners[4]);

    lgl->Vertex3dv(corners[1]);
    lgl->Vertex3dv(corners[3]);
    lgl->Vertex3dv(corners[1]);
    lgl->Vertex3dv(corners[5]);

    lgl->Vertex3dv(corners[2]);
    lgl->Vertex3dv(corners[3]);
    lgl->Vertex3dv(corners[2]);
    lgl->Vertex3dv(corners[6]);

    lgl->Vertex3dv(corners[3]);
    lgl->Vertex3dv(corners[7]);

    lgl->Vertex3dv(corners[4]);
    lgl->Vertex3dv(corners[5]);
    lgl->Vertex3dv(corners[4]);
    lgl->Vertex3dv(corners[6]);

    lgl->Vertex3dv(corners[5]);
    lgl->Vertex3dv(corners[7]);

    lgl->Vertex3dv(corners[6]);
    lgl->Vertex3dv(corners[7]);
    lgl->End();

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

void TranslateStretchManip::_stretchCorners(double corners[8][3]) const
{
    int axis = (_selectedHandle < 3) ? (2 - _selectedHandle) : (_selectedHandle - 3);

    // X axis
    if (axis == 0) {
        if (_selectedHandle == 2)
            _moveMinusXCorners(corners);
        else if (_selectedHandle == 3)
            _movePlusXCorners(corners);
    }
    // Y axis
    else if (axis == 1) {
        if (_selectedHandle == 4)
            _moveMinusYCorners(corners);
        else if (_selectedHandle == 1)
            _movePlusYCorners(corners);
    }
    // Z axis
    else if (axis == 2) {
        if (_selectedHandle == 0)
            _moveMinusZCorners(corners);
        else if (_selectedHandle == 5)
            _movePlusZCorners(corners);
    }
}

void TranslateStretchManip::_translateCorners(double corners[8][3]) const
{
    int axis = (_selectedHandle < 3) ? (2 - _selectedHandle) : (_selectedHandle - 3);

    if (axis == 0) {
        _moveMinusXCorners(corners);
        _movePlusXCorners(corners);
    } else if (axis == 1) {
        _moveMinusYCorners(corners);
        _movePlusYCorners(corners);
    } else if (axis == 2) {
        _moveMinusZCorners(corners);
        _movePlusZCorners(corners);
    }
}

void TranslateStretchManip::_moveMinusXCorners(double corners[8][3]) const
{
    corners[2][X] += _dragDistance;
    corners[0][X] += _dragDistance;
    corners[1][X] += _dragDistance;
    corners[3][X] += _dragDistance;
}

void TranslateStretchManip::_movePlusXCorners(double corners[8][3]) const
{
    corners[4][X] += _dragDistance;
    corners[5][X] += _dragDistance;
    corners[6][X] += _dragDistance;
    corners[7][X] += _dragDistance;
}

void TranslateStretchManip::_moveMinusYCorners(double corners[8][3]) const
{
    corners[2][Y] += _dragDistance;
    corners[3][Y] += _dragDistance;
    corners[6][Y] += _dragDistance;
    corners[7][Y] += _dragDistance;
}

void TranslateStretchManip::_movePlusYCorners(double corners[8][3]) const
{
    corners[0][Y] += _dragDistance;
    corners[1][Y] += _dragDistance;
    corners[4][Y] += _dragDistance;
    corners[5][Y] += _dragDistance;
}

void TranslateStretchManip::_moveMinusZCorners(double corners[8][3]) const
{
    corners[0][Z] += _dragDistance;
    corners[2][Z] += _dragDistance;
    corners[4][Z] += _dragDistance;
    corners[6][Z] += _dragDistance;
}

void TranslateStretchManip::_movePlusZCorners(double corners[8][3]) const
{
    corners[1][Z] += _dragDistance;
    corners[3][Z] += _dragDistance;
    corners[5][Z] += _dragDistance;
    corners[7][Z] += _dragDistance;
}

// Note: This is performed in local (unstretched) world coordinates!
void TranslateStretchManip::_captureMouseDown(int handleNum, int buttonNum, const double strHandleMid[3])
{
    _buttonNum = buttonNum;

    // Grab a probe handle
    _selectedHandle = handleNum;
    _dragDistance = 0.f;

    // Calculate intersection of ray with specified plane in unstretched coords
    // The selection ray is the vector from the camera to the intersection point
    for (int i = 0; i < 3; i++) _initialSelectionRay[i] = strHandleMid[i] - _cameraPosition[i];

    if (buttonNum > 1)
        _isStretching = true;
    else
        _isStretching = false;
    // Reset any active rotation
    _tempRotation = 0.f;
    _tempRotAxis = -1;
}

// Slide the handle based on mouse move from previous capture.
// Requires new direction vector associated with current mouse position
// The new face position requires finding the planar displacement such that
// the ray (in the scene) associated with the new mouse position is as near
// as possible to the line projected from the original mouse position in the
// direction of planar motion.
// Initially calc done in  WORLD coords,
// Converted to stretched world coords for bounds testing,
// then final displacement is in cube coords.
// If constrain is true, the slide will not go out of the full extents of the data.
//

void TranslateStretchManip::slideHandle(int handleNum, const double movedRay[3], bool constrain)
{
    double normalVector[3] = {0.f, 0.f, 0.f};
    double q[3], r[3], w[3];
    assert(handleNum >= 0);
    int coord = (handleNum < 3) ? (2 - handleNum) : (handleNum - 3);

    normalVector[coord] = 1.f;
    // Calculate W:
    vcopy(movedRay, w);
    vnormal(w);
    double scaleFactor = 1.f / vdot(w, normalVector);

    // Calculate q:
    vmult(w, scaleFactor, q);
    vsub(q, normalVector, q);

    // Calculate R:
    scaleFactor *= vdot(_initialSelectionRay, normalVector);
    vmult(w, scaleFactor, r);
    vsub(r, _initialSelectionRay, r);

    double denom = vdot(q, q);
    _dragDistance = 0.f;
    if (denom != 0.) { _dragDistance = -vdot(q, r) / denom; }

    // Make sure the displacement is OK.  Not allowed to
    // slide box out of full domain box.
    // If stretching, not allowed to push face through opposite face.

    float temp = _dragDistance;
    deScaleScalarOnAxis(temp, coord);
    _dragDistance = temp;

    if (_isStretching) {    // don't push through opposite face ..
        // Depends on whether we are pushing the "low" or "high" handle
        // E.g., The low handle is limited by the low end of the extents, and the
        // big end of the box
        if (handleNum < 3) {
            if (_dragDistance + _selection[coord] > _selection[coord + 3]) { _dragDistance = _selection[coord + 3] - _selection[coord]; }
        } else {    // Moving "high" handle:
            if (_dragDistance + _selection[coord + 3] < _selection[coord]) { _dragDistance = _selection[coord] - _selection[coord + 3]; }
        }
    }
}

// Draw a line connecting the specified handle to the box center.
// Highlight both ends of the line to the selected handle
// Note that handleExtents have already been displaced.
// if _isStretching is true,
// the box center only moves half as far.
void TranslateStretchManip::drawHandleConnector(int handleNum, double *handleExtents, double *boxExtents)
{
    LegacyGL *lgl = _glManager->legacy;
    // Determine the side of the handle and the side of the box that is connected:
    int    axis = (handleNum < 3) ? (2 - handleNum) : (handleNum - 3);
    bool   posSide = (handleNum > 2);
    double handleDisp[3], boxDisp[3];
    // Every handle gets a line from its inside face to the center of the box.
    for (int i = 0; i < 3; i++) {
        // determine the box displacement, based on dimension:
        if ((i == (2 - _selectedHandle)) || (i == (_selectedHandle - 3))) {
            if (_isStretching)
                boxDisp[i] = 0.5f * _dragDistance;
            else
                boxDisp[i] = _dragDistance;
        } else
            boxDisp[i] = 0.f;
        // The line to the handle is displaced according to what axis the
        // handle is on (independent of the _dragDistance)
        //
        if (i == axis) {
            if (posSide) {    // displace to lower (in-) side of handle
                handleDisp[i] = -0.5f * (handleExtents[i + 3] - handleExtents[i]);
            } else {    // displace to upper (out-) side of handle
                handleDisp[i] = 0.5f * (handleExtents[i + 3] - handleExtents[i]);
            }
        } else {    // don't displace other coordinates
            handleDisp[i] = 0.f;
        }
    }
    GL_LEGACY(glLineWidth(2.0));
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GL_LEGACY(glPolygonMode(GL_FRONT, GL_FILL));
    if ((handleNum == _selectedHandle) || (handleNum == (5 - _selectedHandle)))
        lgl->Color4fv(_faceSelectionColor);
    else
        lgl->Color4fv(_unselectedFaceColor);
    lgl->Begin(GL_LINES);
    lgl->Vertex3f(0.5f * (handleExtents[MAXX] + handleExtents[MINX]) + handleDisp[X], 0.5f * (handleExtents[MAXY] + handleExtents[MINY]) + handleDisp[Y],
                  0.5f * (handleExtents[MAXZ] + handleExtents[MINZ]) + handleDisp[Z]);
    lgl->Vertex3f(0.5f * (boxExtents[MAXX] + boxExtents[MINX]) + boxDisp[X], 0.5f * (boxExtents[MAXY] + boxExtents[MINY]) + boxDisp[Y], 0.5f * (boxExtents[MAXZ] + boxExtents[MINZ]) + boxDisp[Z]);
    lgl->End();
    glDisable(GL_BLEND);
}

// This is a utility to draw the hitbox over the handle.  It can be
// called from TranslateStretchManip::pointIsOnQuad(), but with a caveat.
// Hitboxes are calculated during mouse events, at which time we are not
// in a good state to perform rendering.
//
// VizWin has an openGL setup and cleanup sequence which must be applied
// during VizWin::_mousePressEvent().  When done as shown below, the hitboxes
// will be colored green during mouse press events.  Hold the mouse to
// illuminate the boxes.
//
// VizWin setup sequence:
//
// void VizWin:;_mousePressEvent(QMouseEvent* e) {
// 		...
// 		...
// 		glMatrixMode(GL_PROJECTION);	// Begin setup sequence
//		glPushMatrix();
//		_setUpProjMatrix();
//		glMatrixMode(GL_MODELVIEW);
//		glPushMatrix();
//		_setUpModelViewMatrix();			// End setup sequence
//
//	    std::vector<double> screenCoords = getScreenCoords(e);
//		bool mouseOnManip = _manip->MouseEvent(
//			_buttonNum, screenCoords, _strHandleMid
//		);
//
//		swapBuffers();					// Begin cleanup sequence
//		glMatrixMode(GL_PROJECTION);
//		glPopMatrix();
//		glMatrixMode(GL_MODELVIEW);
//		glPopMatrix();					// End cleanup sequence
//		...
//		...
//

#ifdef DEBUG
void TranslateStretchManip::drawHitBox(double winCoord1[2], double winCoord2[2], double winCoord4[2], double winCoord3[2]) const
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    GLint dims[4] = {0};
    glGetIntegerv(GL_VIEWPORT, dims);
    GLint width = dims[2];
    GLint height = dims[3];

    gluOrtho2D(0, width, 0, height);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBegin(GL_QUADS);    // Each set of 4 vertices form a quad
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2f(winCoord1[0], winCoord1[1]);
    glVertex2f(winCoord2[0], winCoord2[1]);
    glVertex2f(winCoord3[0], winCoord3[1]);
    glVertex2f(winCoord4[0], winCoord4[1]);

    glEnd();
    glFlush();
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glDisable(GL_BLEND);
}
#endif

//_projectPointToWin returns true if point is in front of camera
// resulting screen coords returned in 2nd argument.  Note that
// OpenGL coords are 0 at bottom of window!
//
bool TranslateStretchManip::_projectPointToWin(const double cubeCoords[3], double winCoords[2]) const
{
    double   depth;
    GLdouble wCoords[2];
    GLdouble cbCoords[3];
    for (int i = 0; i < 3; i++) cbCoords[i] = (double)cubeCoords[i];

    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    bool success = (0 != gluProject(cbCoords[0], cbCoords[1], cbCoords[2], _modelViewMatrix, _projectionMatrix, viewport, wCoords, (wCoords + 1), (GLdouble *)(&depth)));

    if (!success) return false;
    winCoords[0] = wCoords[0];
    winCoords[1] = wCoords[1];
    return (depth > 0.0);
}
