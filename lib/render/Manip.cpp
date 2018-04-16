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
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		November 2005
//
//	Description:	Implements the Manip class and some of its subclasses
// TOGO:

#include <iostream>
#include <cassert>

#include "vapor/Manip.h"

using namespace VAPoR;
using namespace std;

const float Manip::_faceSelectionColor[4] = {0.8f, 0.8f, 0.0f, 0.5f};
const float Manip::_unselectedFaceColor[4] = {0.8f, 0.2f, 0.0f, 0.5f};

TranslateStretchManip::TranslateStretchManip() : Manip()
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
        _rotationCenter[i] = 0.;
        _handleMid[i] = 0.f;

        _selection[i] = 0.;
        _selection[i + 3] = 0.;
        _extents[i] = 0.;
        _extents[i + 3] = 0.;
    }
}

void TranslateStretchManip::Update(std::vector<double> llc, std::vector<double> urc, std::vector<double> minExts, std::vector<double> maxExts, std::vector<double> cameraPosition,
                                   std::vector<double> rotationCenter, double modelViewMatrix[16], double projectionMatrix[16], std::vector<int> windowSize, VAPoR::Transform *transform,
                                   bool constrain)
{
    for (int i = 0; i < 16; i++) {
        _modelViewMatrix[i] = modelViewMatrix[i];
        _projectionMatrix[i] = projectionMatrix[i];
    }

    _windowSize = windowSize;
    _cameraPosition[0] = cameraPosition[0];
    _cameraPosition[1] = cameraPosition[1];
    _cameraPosition[2] = cameraPosition[2];

    _rotationCenter[0] = _rotationCenter[0];
    _rotationCenter[1] = _rotationCenter[1];
    _rotationCenter[2] = _rotationCenter[2];

    std::copy(llc.begin(), llc.end(), _selection);
    std::copy(urc.begin(), urc.end(), _selection + 3);
    std::copy(minExts.begin(), minExts.end(), _extents);
    std::copy(maxExts.begin(), maxExts.end(), _extents + 3);

    _transform = transform;

    _constrain = constrain;
}

void TranslateStretchManip::GetBox(std::vector<double> &llc, std::vector<double> &urc)
{
    llc.resize(3);
    urc.resize(3);
    llc[0] = _selection[0];
    llc[1] = _selection[1];
    llc[2] = _selection[2];
    urc[0] = _selection[3];
    urc[1] = _selection[4];
    urc[2] = _selection[5];
}

bool TranslateStretchManip::MouseEvent(int buttonNum, std::vector<double> vscreenCoords, double handleMidpoint[3], bool release)
{
    double screenCoords[2] = {vscreenCoords[0], vscreenCoords[1]};

    if (_selectedHandle < 0) _selectedHandle = mouseIsOverHandle(screenCoords, handleMidpoint);

    if (_selectedHandle < 0) return false;

    if (release) {    // Release
        mouseRelease(screenCoords);
    } else if (buttonNum == _buttonNum) {    // Dragging
        if (!_mouseDownHere) return false;
        mouseDrag(screenCoords, handleMidpoint);
    } else if (_buttonNum == 0) {    // Press
        mousePress(screenCoords, handleMidpoint, buttonNum);
    }

    return true;
}

void TranslateStretchManip::mouseDrag(double screenCoords[2], double handleMidpoint[3])
{
    if (_selectedHandle >= 0) {
        double projScreenCoords[2];
        bool   success = projectPointToLine(screenCoords, projScreenCoords);
        if (success) {
            double dirVec[3];
            pixelToVector(projScreenCoords, dirVec, handleMidpoint);
            slideHandle(_selectedHandle, dirVec, false);
            // slideHandle(_selectedHandle, dirVec);
        }
    }
}

void TranslateStretchManip::mousePress(double screenCoords[2], double handleMidpoint[3], int buttonNum)
{
    double dirVec[3];
    pixelToVector(screenCoords, dirVec, handleMidpoint);
    captureMouseDown(_selectedHandle, buttonNum, handleMidpoint);
    startHandleSlide(screenCoords, _selectedHandle);
    setMouseDown(true);
}

void TranslateStretchManip::mouseRelease(double screenCoords[2])
{
    if (_selectedHandle >= 0) {
        double boxExts[6];
        int    axis = (_selectedHandle < 3) ? (2 - _selectedHandle) : (_selectedHandle - 3);
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
            _selection[axis] += dist;
            _selection[axis + 3] += dist;
        }
    }

    if (_constrain) constrainExtents();

    _dragDistance = 0.f;
    _selectedHandle = -1;
    _buttonNum = 0;
    setMouseDown(false);
}

int TranslateStretchManip::mouseIsOverHandle(double screenCoords[2], double handleMid[3])
{
    double boxExtents[6];
    std::copy(std::begin(_selection), std::end(_selection), std::begin(boxExtents));

    // double handleMid[3];
    double handle[8][3];

    // double pos[3], upVec[3], viewDir[3];
    // ReconstructCamera(pos, upVec, viewDir);

    int octant = 0;
    int face, handleNum;
    for (int axis = 0; axis < 3; axis++) {
        double axisBoundary = 0.5f * (boxExtents[axis] + boxExtents[axis + 3]);
        if (_cameraPosition[axis] > axisBoundary) { octant |= 1 << axis; }
    }

    // Front handles
    for (int sortNum = 0; sortNum < 3; sortNum++) {
        handleNum = makeHandleFaces(sortNum, handle, octant, boxExtents);
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
        handleNum = makeHandleFaces(sortNum, handle, octant, boxExtents);
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

void TranslateStretchManip::constrainExtents()
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

bool TranslateStretchManip::pointIsOnQuad(double cor1[3], double cor2[3], double cor3[3], double cor4[3], double pickPt[2])
{
    double winCoord1[2];
    double winCoord2[2];
    double winCoord3[2];
    double winCoord4[2];
    if (!projectPointToWin(cor1, winCoord1)) return false;
    if (!projectPointToWin(cor2, winCoord2)) return false;
    if (pointOnRight(winCoord1, winCoord2, pickPt)) return false;
    if (!projectPointToWin(cor3, winCoord3)) return false;
    if (pointOnRight(winCoord2, winCoord3, pickPt)) return false;
    if (!projectPointToWin(cor4, winCoord4)) return false;
    if (pointOnRight(winCoord3, winCoord4, pickPt)) return false;
    if (pointOnRight(winCoord4, winCoord1, pickPt)) return false;
    return true;
}

int TranslateStretchManip::pointIsOnBox(double corners[8][3], double pickPt[2])
{
    // front (-Z)
    if (pointIsOnQuad(corners[0], corners[1], corners[3], corners[2], pickPt)) return 2;
    // if (pointIsOnQuad(corners[0],corners[1],corners[3],corners[2],pickPt)) return 4;
    // back (+Z)
    if (pointIsOnQuad(corners[4], corners[6], corners[7], corners[5], pickPt)) return 3;
    // if (pointIsOnQuad(corners[4],corners[6],corners[7],corners[5],pickPt)) return 5;
    // right (+X)
    if (pointIsOnQuad(corners[1], corners[5], corners[7], corners[3], pickPt)) return 5;
    // if (pointIsOnQuad(corners[1],corners[5],corners[7],corners[3],pickPt)) return 1;
    // left (-X)
    if (pointIsOnQuad(corners[0], corners[2], corners[6], corners[4], pickPt)) return 0;
    // if (pointIsOnQuad(corners[0],corners[2],corners[6],corners[4],pickPt)) return 0;
    // top (+Y)
    if (pointIsOnQuad(corners[2], corners[3], corners[7], corners[6], pickPt)) return 4;
    // if (pointIsOnQuad(corners[2],corners[3],corners[7],corners[6],pickPt)) return 3;
    // bottom (-Y)
    if (pointIsOnQuad(corners[0], corners[4], corners[5], corners[1], pickPt)) return 1;
    // if (pointIsOnQuad(corners[0],corners[4],corners[5],corners[1],pickPt)) return 2;
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
int TranslateStretchManip::makeHandleFaces(int sortPosition, double handle[8][3], int octant, double boxRegion[6])
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
            // Obtain the coordinate of unit cube corner.  It's either +0.5 or -0.5
            // multiplied by the handle diameter, then translated along the
            // specific axis corresponding to
            double fltCoord = (((double)((vertex >> coord) & 1) - 0.5f) * _handleSizeInScene);
            // First offset it from the probeCenter:
            fltCoord += 0.5f * (boxRegion[coord + 3] + boxRegion[coord]);
            // Displace all the c - coords of this handle if this handle is on the c-axis
            if (coord == axis) {
                double boxWidth = (boxRegion[coord + 3] - boxRegion[coord]);
                // Note we are putting the handle 2 diameters from the box edge
                fltCoord += translateSign * (boxWidth * 0.5f + 2.f * _handleSizeInScene);
            }
            handle[vertex][coord] = fltCoord;
        }
    }
    return newPosition;
}

bool TranslateStretchManip::ReconstructCamera(double position[3], double upVec[3], double viewDir[3]) const
{
    double m[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, m);

    double minv[16];

    int rc = minvert(m, minv);
    if (rc < 0) return (false);

    vscale(minv + 8, -1.0);

    for (int i = 0; i < 3; i++) {
        position[i] = minv[12 + i];    // position vector is minv[12..14]
        upVec[i] = minv[4 + i];        // up vector is minv[4..6]
        viewDir[i] = minv[8 + i];      // view direction is minv[8..10]
    }
    vnormal(upVec);
    vnormal(viewDir);

    return (true);
}

bool TranslateStretchManip::startHandleSlide(double mouseCoords[2], int handleNum)
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
    double boxExtents[6];

    for (int i = 0; i < 3; i++) { boxCtr[i] = (_selection[i] + _selection[i + 3]) * 0.5f; }
    // project the boxCtr and one more point, to get a direction vector

    if (!projectPointToWin(boxCtr, winCoords)) return false;

    boxCtr[handleNum] += 0.1f;

    if (!projectPointToWin(boxCtr, dispCoords)) return false;

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
bool TranslateStretchManip::projectPointToLine(double mouseCoords[2], double projCoords[2])
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
bool TranslateStretchManip::pixelToVector(double winCoords[2], double dirVec[3], double strHandleMid[3])
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
    float worldHandleDiameter = _handleSizeInScene;
    for (int coord = 0; coord < 3; coord++) {
        // Start at the box center position
        handleExtents[coord] = .5f * (-worldHandleDiameter + (boxExtents[coord + 3] + boxExtents[coord]));
        handleExtents[coord + 3] = .5f * (worldHandleDiameter + (boxExtents[coord + 3] + boxExtents[coord]));

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
    return;
}

// Draw all the faces of a cube with specified extents.
// Currently just used for handles.
void TranslateStretchManip::drawCubeFaces(double *extents, bool isSelected)
{
    glLineWidth(2.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (isSelected)
        glColor4fv(_faceSelectionColor);
    else
        glColor4fv(_unselectedFaceColor);

    // Do left (x=0)
    glBegin(GL_QUADS);
    glVertex3f(extents[0], extents[1], extents[2]);
    glVertex3f(extents[0], extents[1], extents[5]);
    glVertex3f(extents[0], extents[4], extents[5]);
    glVertex3f(extents[0], extents[4], extents[2]);
    glEnd();

    // do right
    if (isSelected)
        glColor4fv(_faceSelectionColor);
    else
        glColor4fv(_unselectedFaceColor);
    glBegin(GL_QUADS);
    glVertex3f(extents[3], extents[1], extents[2]);
    glVertex3f(extents[3], extents[1], extents[5]);
    glVertex3f(extents[3], extents[4], extents[5]);
    glVertex3f(extents[3], extents[4], extents[2]);
    glEnd();

    // top
    if (isSelected)
        glColor4fv(_faceSelectionColor);
    else
        glColor4fv(_unselectedFaceColor);
    glBegin(GL_QUADS);
    glVertex3f(extents[0], extents[4], extents[2]);
    glVertex3f(extents[3], extents[4], extents[2]);
    glVertex3f(extents[3], extents[4], extents[5]);
    glVertex3f(extents[0], extents[4], extents[5]);
    glEnd();

    // bottom
    if (isSelected)
        glColor4fv(_faceSelectionColor);
    else
        glColor4fv(_unselectedFaceColor);
    glBegin(GL_QUADS);
    glVertex3f(extents[0], extents[1], extents[2]);
    glVertex3f(extents[0], extents[1], extents[5]);
    glVertex3f(extents[3], extents[1], extents[5]);
    glVertex3f(extents[3], extents[1], extents[2]);
    glEnd();

    // back
    if (isSelected)
        glColor4fv(_faceSelectionColor);
    else
        glColor4fv(_unselectedFaceColor);
    glBegin(GL_QUADS);
    glVertex3f(extents[0], extents[1], extents[2]);
    glVertex3f(extents[3], extents[1], extents[2]);
    glVertex3f(extents[3], extents[4], extents[2]);
    glVertex3f(extents[0], extents[4], extents[2]);
    glEnd();

    // do the front:
    //
    if (isSelected)
        glColor4fv(_faceSelectionColor);
    else
        glColor4fv(_unselectedFaceColor);
    glBegin(GL_QUADS);
    glVertex3f(extents[0], extents[1], extents[5]);
    glVertex3f(extents[3], extents[1], extents[5]);
    glVertex3f(extents[3], extents[4], extents[5]);
    glVertex3f(extents[0], extents[4], extents[5]);
    glEnd();

    glColor4f(1, 1, 1, 1);
    glDisable(GL_BLEND);
}

bool TranslateStretchManip::rayHandleIntersect(double ray[3], const std::vector<double> &cameraPos, int handleNum, int faceNum, double intersect[3])
{
    double val;
    double handleExtents[6];
    double boxExtents[6];

    makeHandleExtents(handleNum, handleExtents, 0, _selection);
    int coord;

    switch (faceNum) {
    case (2):    // back; z = zmin
        val = handleExtents[2];
        coord = 2;
        break;
    case (3):    // front; z = zmax
        val = handleExtents[5];
        coord = 2;
        break;
    case (1):    // bot; y = min
        val = handleExtents[1];
        coord = 1;
        break;
    case (4):    // top; y = max
        val = handleExtents[4];
        coord = 1;
        break;
    case (0):    // left; x = min
        val = handleExtents[0];
        coord = 0;
        break;
    case (5):    // right; x = max
        val = handleExtents[3];
        coord = 0;
        break;
    default: return false;
    }
    if (ray[coord] == 0.0) return false;
    float param = (val - cameraPos[coord]) / ray[coord];
    for (int i = 0; i < 3; i++) { intersect[i] = cameraPos[i] + param * ray[i]; }
    return true;
}

// Renders handles and box
// If it is stretching, it only moves the one handle that is doing the stretching
void TranslateStretchManip::render()
{
    double extents[6];

    _handleSizeInScene = getPixelSize() * (float)HANDLE_DIAMETER;

    std::vector<double> scales = _transform->GetScales();
    std::vector<double> selectionSize, selectionMid;
    int                 ndims = 3;    //_selection.size()/2;
    cout << "Tx Mscales " << scales[0] << " " << scales[1] << " " << scales[2] << endl;
    for (int i = 0; i < ndims; i++) {
        selectionSize.push_back(_selection[i + ndims] - _selection[i]);
        selectionSize[i] *= scales[i];

        selectionMid.push_back((_selection[i] + _selection[i + ndims]) / 2.f);

        _transformedSelection[i] = selectionMid[i] - selectionSize[i] / 2.f;
        _transformedSelection[i + ndims] = selectionMid[i] + selectionSize[i] / 2.f;
    }
    cout << "_selection " << _selection[0] << " " << _selection[3] << endl;
    cout << "scaledSele " << _transformedSelection[0] << " " << _transformedSelection[3] << endl;

    glPushAttrib(GL_CURRENT_BIT);
    double handleExtents[6];
    for (int handleNum = 0; handleNum < 6; handleNum++) {
        // makeHandleExtents(handleNum, handleExtents, 0/*octant*/, extents);
        // makeHandleExtents(handleNum, handleExtents, 0/*octant*/, _extents);
        // makeHandleExtents(handleNum, handleExtents, 0/*octant*/, _selection);
        makeHandleExtents(handleNum, handleExtents, 0 /*octant*/, _transformedSelection);

        if (_selectedHandle >= 0) {
            // int axis = (_selectedHandle < 3) ? (2-_selectedHandle) : (_selectedHandle -3);
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
        drawHandleConnector(handleNum, handleExtents, _selection);    // extents);
                                                                      // drawHandleConnector(handleNum, handleExtents, transformedSelection);//extents);
    }
    // Then render the full box, unhighlighted and displaced
    drawBoxFaces();
    glPopAttrib();
}

double TranslateStretchManip::getPixelSize() const
{
    double temp[3];

    // Window height is subtended by viewing angle (45 degrees),
    // at viewer distance (dist from camera to view center)
    //	const AnnotationParams* vfParams = getActiveAnnotationParams();
    //	const ViewpointParams* vpParams = getActiveViewpointParams();

    size_t width, height;
    //	vpParams->GetWindowSize(width, height);
    width = _windowSize[0];     // 500;
    height = _windowSize[1];    // 500;

    double center[3] = {0., 0., 0.};    //, pos[3];
    // double pos[3], upVec[3], viewDir[3];
    // ReconstructCamera(pos, upVec, viewDir);

    // vpParams->GetRotationCenter(center);
    // vpParams->GetCameraPos(pos);

    vsub(_rotationCenter, _cameraPosition, temp);

    // Apply stretch factor:

    // vector<double> stretch = vpParams->GetStretchFactors();
    // for (int i = 0; i<3; i++) temp[i] = stretch[i]*temp[i];
    float distToScene = vlength(temp);
    // tan(45 deg *0.5) is ratio between half-height and dist to scene
    double halfHeight = tan(M_PI * 0.125) * distToScene;
    return (2.f * halfHeight / (double)height);

    return (0.0);
}

// Draw the main box, just rendering the lines.
// the highlightedFace is not the same as the selectedFace!!
//

void TranslateStretchManip::drawBoxFaces()
{
    double corners[8][3];

    // -X -Y -Z
    corners[0][0] = _selection[0];
    corners[0][1] = _selection[1];
    corners[0][2] = _selection[2];

    // -X -Y +Z
    corners[1][0] = _selection[0];
    corners[1][1] = _selection[1];
    corners[1][2] = _selection[5];

    // -X +Y -Z
    corners[2][0] = _selection[0];
    corners[2][1] = _selection[4];
    corners[2][2] = _selection[2];

    // -X +Y +Z
    corners[3][0] = _selection[0];
    corners[3][1] = _selection[4];
    corners[3][2] = _selection[5];

    // +X -Y -Z
    corners[4][0] = _selection[3];
    corners[4][1] = _selection[1];
    corners[4][2] = _selection[2];

    // +X -Y +Z
    corners[5][0] = _selection[3];
    corners[5][1] = _selection[1];
    corners[5][2] = _selection[5];

    // +X +Y -Z
    corners[6][0] = _selection[3];
    corners[6][1] = _selection[4];
    corners[6][2] = _selection[2];

    // +X +Y +Z
    corners[7][0] = _selection[3];
    corners[7][1] = _selection[4];
    corners[7][2] = _selection[5];

    if (_selectedHandle >= 0) {
        if (_isStretching)
            stretchCorners(corners);
        else if (_dragDistance != 0.)
            translateCorners(corners);
    }

    // Now render the edges:
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glLineWidth(2.0);
    glColor3f(1.f, 0.f, 0.f);
    glBegin(GL_LINES);
    glVertex3dv(corners[0]);
    glVertex3dv(corners[1]);
    glColor3f(1.f, 1.f, 0.f);
    glVertex3dv(corners[0]);
    glVertex3dv(corners[2]);
    glVertex3dv(corners[0]);
    glVertex3dv(corners[4]);

    glVertex3dv(corners[1]);
    glVertex3dv(corners[3]);
    glVertex3dv(corners[1]);
    glVertex3dv(corners[5]);

    glVertex3dv(corners[2]);
    glVertex3dv(corners[3]);
    glVertex3dv(corners[2]);
    glVertex3dv(corners[6]);

    glVertex3dv(corners[3]);
    glVertex3dv(corners[7]);

    glVertex3dv(corners[4]);
    glVertex3dv(corners[5]);
    glVertex3dv(corners[4]);
    glVertex3dv(corners[6]);

    glVertex3dv(corners[5]);
    glVertex3dv(corners[7]);

    glVertex3dv(corners[6]);
    glVertex3dv(corners[7]);
    glEnd();

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
}

void TranslateStretchManip::stretchCorners(double corners[8][3])
{
    int axis = (_selectedHandle < 3) ? (2 - _selectedHandle) : (_selectedHandle - 3);

    // X axis
    if (axis == 0) {
        if (_selectedHandle == 2)
            moveMinusXCorners(corners);
        else if (_selectedHandle == 3)
            movePlusXCorners(corners);
    }
    // Y axis
    else if (axis == 1) {
        if (_selectedHandle == 4)
            moveMinusYCorners(corners);
        else if (_selectedHandle == 1)
            movePlusYCorners(corners);
    }
    // Z axis
    else if (axis == 2) {
        if (_selectedHandle == 0)
            moveMinusZCorners(corners);
        else if (_selectedHandle == 5)
            movePlusZCorners(corners);
    }
}

void TranslateStretchManip::translateCorners(double corners[8][3])
{
    int axis = (_selectedHandle < 3) ? (2 - _selectedHandle) : (_selectedHandle - 3);

    if (axis == 0) {
        moveMinusXCorners(corners);
        movePlusXCorners(corners);
    } else if (axis == 1) {
        moveMinusYCorners(corners);
        movePlusYCorners(corners);
    } else if (axis == 2) {
        moveMinusZCorners(corners);
        movePlusZCorners(corners);
    }
}

void TranslateStretchManip::moveMinusXCorners(double corners[8][3])
{
    corners[2][0] += _dragDistance;
    corners[0][0] += _dragDistance;
    corners[1][0] += _dragDistance;
    corners[3][0] += _dragDistance;
}

void TranslateStretchManip::movePlusXCorners(double corners[8][3])
{
    corners[4][0] += _dragDistance;
    corners[5][0] += _dragDistance;
    corners[6][0] += _dragDistance;
    corners[7][0] += _dragDistance;
}

void TranslateStretchManip::moveMinusYCorners(double corners[8][3])
{
    corners[2][1] += _dragDistance;
    corners[3][1] += _dragDistance;
    corners[6][1] += _dragDistance;
    corners[7][1] += _dragDistance;
}

void TranslateStretchManip::movePlusYCorners(double corners[8][3])
{
    corners[0][1] += _dragDistance;
    corners[1][1] += _dragDistance;
    corners[4][1] += _dragDistance;
    corners[5][1] += _dragDistance;
}

void TranslateStretchManip::moveMinusZCorners(double corners[8][3])
{
    corners[0][2] += _dragDistance;
    corners[2][2] += _dragDistance;
    corners[4][2] += _dragDistance;
    corners[6][2] += _dragDistance;
}

void TranslateStretchManip::movePlusZCorners(double corners[8][3])
{
    corners[1][2] += _dragDistance;
    corners[3][2] += _dragDistance;
    corners[5][2] += _dragDistance;
    corners[7][2] += _dragDistance;
}

// Note: This is performed in local (unstretched) world coordinates!
void TranslateStretchManip::captureMouseDown(int handleNum, int buttonNum, double strHandleMid[3])
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

void TranslateStretchManip::
    // slideHandle(int handleNum, double movedRay[3]){
    slideHandle(int handleNum, double movedRay[3], bool constrain)
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

    // Do this calculation in stretched world coords
    double sizes[3];
    int    ndims = 3;    //_extents.size()/2;
    for (int i = 0; i < ndims; i++) { sizes[i] = _extents[i + ndims] - _extents[i]; }

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
    glLineWidth(2.0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPolygonMode(GL_FRONT, GL_FILL);
    if ((handleNum == _selectedHandle) || (handleNum == (5 - _selectedHandle)))
        glColor4fv(_faceSelectionColor);
    else
        glColor4fv(_unselectedFaceColor);
    glBegin(GL_LINES);
    glVertex3f(0.5f * (handleExtents[3] + handleExtents[0]) + handleDisp[0], 0.5f * (handleExtents[4] + handleExtents[1]) + handleDisp[1],
               0.5f * (handleExtents[5] + handleExtents[2]) + handleDisp[2]);
    glVertex3f(0.5f * (boxExtents[3] + boxExtents[0]) + boxDisp[0], 0.5f * (boxExtents[4] + boxExtents[1]) + boxDisp[1], 0.5f * (boxExtents[5] + boxExtents[2]) + boxDisp[2]);
    glEnd();
    glDisable(GL_BLEND);
}

// projectPointToWin returns true if point is in front of camera
// resulting screen coords returned in 2nd argument.  Note that
// OpenGL coords are 0 at bottom of window!
//
bool TranslateStretchManip::projectPointToWin(double cubeCoords[3], double winCoords[2])
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
