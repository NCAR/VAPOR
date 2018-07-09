//************************************************************************
//									*
//			 Copyright (C)  2013				*
//	 University Corporation for Atmospheric Research			*
//			 All Rights Reserved				*
//									*
//************************************************************************/
//	File:		VizWin.cpp
//
//	Author:		Alan Norton
//			National Center for Atmospheric Research
//			PO 3000, Boulder, Colorado
//
//	Date:		October 2013
//
//	Description:	Implements the VizWin class
//		This is the QGLWidget that performs OpenGL rendering (using associated
//		Visualizer).
//
//		Supports mouse event reporting.
//
#include <vapor/glutil.h>    // Must be included first!!!
#include <cassert>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QIcon>
#include <vapor/ControlExecutive.h>
#include <vapor/ViewpointParams.h>
#include <vapor/Viewpoint.h>
#include <vapor/debug.h>
#include <vapor/ImageParams.h>
#include "TrackBall.h"
#include "GUIStateParams.h"
#include "MouseModeParams.h"
#include "AnimationParams.h"
#include "qdatetime.h"
#include "ErrorReporter.h"
#include "images/vapor-icon-32.xpm"
#include "VizWin.h"

using namespace VAPoR;

/*
 *  Constructs a VizWindow as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
VizWin::VizWin(QWidget *parent, const QString &name, string winName, ControlExec *ce, Trackball *trackBall) : QGLWidget(parent)
{
    _trackBall = trackBall;

    setAttribute(Qt::WA_DeleteOnClose);
    _winName = winName;
    setWindowIcon(QPixmap(vapor_icon___));
    _controlExec = ce;

    setAutoBufferSwap(false);
    _mouseClicked = false;
    _buttonNum = 0;
    _navigating = false;

    setMouseTracking(false);    // Only track mouse when button clicked/held

    _manip = new TranslateStretchManip();
    bool initialize = true;
    updateManip(initialize);

    for (int i = 0; i < 3; i++) {
        _center[i] = 0.0;
        _posvec[i] = 0.0;
        _dirvec[i] = 0.0;
        _upvec[i] = 0.0;
    }
}

/*
 *  Destroys the object and frees any allocated resources
 */
VizWin::~VizWin() {}

void VizWin::closeEvent(QCloseEvent *e)
{
    // Remove the visualizer. This must be done inside of VizWin so that
    // the OpenGL context can be made current because removing a visualizer
    // triggers OpenGL calls in the renderer destructors
    //
    makeCurrent();

    emit Closing(_winName);

    QWidget::closeEvent(e);
}

/******************************************************
 * React when focus is on window:
 ******************************************************/
void VizWin::focusInEvent(QFocusEvent *e)
{
    // Test for hidden here, since a vanishing window can get this event.
    if (e->gotFocus() && !isHidden()) { emit HasFocus(_winName); }
}

// First project all 8 box corners to the center line of the camera
// view, finding the furthest and nearest projection in front of the
// camera.  The furthest distance is used as the far distance.
// If some point projects behind the camera, then either the
// camera is inside the box, or a corner of the
// box is behind the camera.  This calculation is always performed in
// local coordinates since a translation won't affect
// the result
//
void VizWin::_getNearFarDist(const double posVec[3], const double dirVec[3], double &boxNear, double &boxFar) const
{
    // First check full box
    double wrk[3], cor[3], boxcor[3];
    double camPosBox[3], dvdir[3];
#ifdef WIN32
    double maxProj = -DBL_MAX;
    double minProj = DBL_MAX;
#else
    double maxProj = -std::numeric_limits<double>::max();
    double minProj = std::numeric_limits<double>::max();
#endif

    DataStatus *dataStatus = _controlExec->GetDataStatus();
    ParamsMgr * paramsMgr = _controlExec->GetParamsMgr();

    AnimationParams *p = (AnimationParams *)paramsMgr->GetParams(AnimationParams::GetClassType());
    size_t           ts = p->GetCurrentTimestep();

    vector<double> minExts, maxExts;
    dataStatus->GetActiveExtents(paramsMgr, _winName, ts, minExts, maxExts);

    for (int i = 0; i < 3; i++) camPosBox[i] = posVec[i];

    for (int i = 0; i < 3; i++) dvdir[i] = dirVec[i];
    vnormal(dvdir);

    // For each box corner,
    //   convert to box coords, then project to line of view
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 3; j++) { cor[j] = ((i >> j) & 1) ? maxExts[j] : minExts[j]; }
        for (int k = 0; k < 3; k++) boxcor[k] = cor[k];

        vsub(boxcor, camPosBox, wrk);

        float mdist = abs(vdot(wrk, dvdir));
        if (minProj > mdist) { minProj = mdist; }
        if (maxProj < mdist) { maxProj = mdist; }
    }

    if (maxProj < 1.e-10) maxProj = 1.e-10;
    if (minProj > 0.03 * maxProj) minProj = 0.03 * maxProj;

    // minProj will be < 0 if either the camera is in the box, or
    // if some of the region is behind the camera plane.  In that case, just
    // set the nearDist a reasonable multiple of the fardist
    //
    if (minProj <= 0.0) minProj = 0.0002 * maxProj;
    boxFar = (float)maxProj;
    boxNear = (float)minProj;

    return;
}

void VizWin::_setUpProjMatrix()
{
    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);

    double m[16];
    vParams->GetModelViewMatrix(m);

    double posvec[3], upvec[3], dirvec[3];
    bool   status = vParams->ReconstructCamera(m, posvec, upvec, dirvec);
    if (!status) {
        MSG_ERR("Failed to get camera parameters");
        return;
    }

    double nearDist, farDist;
    _getNearFarDist(posvec, dirvec, nearDist, farDist);
    nearDist *= 0.25;
    farDist *= 4.0;

    size_t width, height;
    vParams->GetWindowSize(width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    GLfloat w = (float)width / (float)height;

    double fov = vParams->GetFOV();
    gluPerspective(fov, w, nearDist, farDist);

    double pMatrix[16];
    glGetDoublev(GL_PROJECTION_MATRIX, pMatrix);

    bool enabled = _controlExec->GetSaveStateEnabled();
    _controlExec->SetSaveStateEnabled(false);

    vParams->SetProjectionMatrix(pMatrix);

    _controlExec->SetSaveStateEnabled(enabled);

    glMatrixMode(GL_MODELVIEW);
}

void VizWin::_setMatrixFromModeParams()
{
    // This is a hack to see if camera parameters have been changed
    // via MouseModeParams. Because the only way to convert camera
    // parameter such as position vector, etc. into a ModelView matrix
    // is via the TrackBall we have to do the conversion in a window
    // with an OpenGL context :-(
    //

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    GUIStateParams * guiP = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    MouseModeParams *p = guiP->GetMouseModeParams();

    double center[3], posvec[3], dirvec[3], upvec[3];
    p->GetRotationCenter(center);
    p->GetCameraPos(posvec);
    p->GetCameraViewDir(dirvec);
    p->GetCameraUpVec(upvec);
    bool updateFromMouseMode = false;
    for (int i = 0; i < 3; i++) {
        if (center[i] != _center[i] || posvec[i] != _posvec[i] || dirvec[i] != _dirvec[i] || upvec[i] != _upvec[i]) { updateFromMouseMode = true; }
    }
    if (!updateFromMouseMode) return;

    _trackBall->setFromFrame(posvec, dirvec, upvec, center, true);
    _trackBall->TrackballSetMatrix();

    double m[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, m);

    // Disable state saving for modelview matrix. It's handled elsewhere and
    // don't want to double up
    //
    bool enabled = _controlExec->GetSaveStateEnabled();
    _controlExec->SetSaveStateEnabled(false);

    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);
    vParams->SetModelViewMatrix(m);

    _controlExec->SetSaveStateEnabled(enabled);

    for (int i = 0; i < 3; i++) {
        _center[i] = center[i];
        _posvec[i] = posvec[i];
        _dirvec[i] = dirvec[i];
        _upvec[i] = upvec[i];
    }
}

void VizWin::_setUpModelViewMatrix()
{
    makeCurrent();    // necessary?

    _setMatrixFromModeParams();

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);

    double m[16];
    vParams->GetModelViewMatrix(m);
    glLoadMatrixd(m);
}

// React to a user-change in window size/position (or possibly max/min)
// Either the window is minimized, maximized, restored, or just resized.
//
void VizWin::resizeGL(int width, int height)
{
    if (!FrameBufferReady()) { return; }

    int rc = printOpenGLErrorMsg("GLVizWindowResizeEvent");
    if (rc < 0) { MSG_ERR("OpenGL error"); }

    rc = _controlExec->ResizeViz(_winName, width, height);
    if (rc < 0) { MSG_ERR("OpenGL error"); }

    glViewport(0, 0, (GLint)width, (GLint)height);

    glClearColor(0., 0., 0., 1.);
    glClear(GL_COLOR_BUFFER_BIT);
    swapBuffers();

    // Necessary?
    //
    glClearColor(0., 0., 0., 1.);
    glClear(GL_COLOR_BUFFER_BIT);
    swapBuffers();

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);

    // Window size can't be undone, so disable state saving
    //
    bool enabled = _controlExec->GetSaveStateEnabled();
    _controlExec->SetSaveStateEnabled(false);
    vParams->SetWindowSize(width, height);
    _controlExec->SetSaveStateEnabled(enabled);
}

void VizWin::initializeGL()
{
    printOpenGLErrorMsg("GLVizWindowInitializeEvent");
    int rc = _controlExec->InitializeViz(_winName);
    if (rc < 0) { MSG_ERR("Failure to initialize Visualizer"); }
    printOpenGLErrorMsg("GLVizWindowInitializeEvent");

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);

    if (vParams != NULL) {
        bool enabled = _controlExec->GetSaveStateEnabled();
        _controlExec->SetSaveStateEnabled(false);
        vParams->SetWindowSize(width(), height());
        _controlExec->SetSaveStateEnabled(enabled);
    }
}

void VizWin::_mousePressEventNavigate(QMouseEvent *e)
{
    _navigating = true;

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    GUIStateParams * guiP = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    MouseModeParams *p = guiP->GetMouseModeParams();

    double           m[16];
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);
    vParams->GetModelViewMatrix(m);

    double center[3];
    p->GetRotationCenter(center);

    double posvec[3], upvec[3], dirvec[3];
    bool   status = vParams->ReconstructCamera(m, posvec, upvec, dirvec);
    assert(status);

    // Set trackball from current ViewpointParams matrix;
    //
    _trackBall->setFromFrame(posvec, dirvec, upvec, center, true);
    _trackBall->TrackballSetMatrix();

    // Let trackball handle mouse events for navigation
    //
    _trackBall->MouseOnTrackball(0, _buttonNum, e->x(), e->y(), width(), height());

    // Create a state saving group.
    // Only save camera parameters after user release mouse
    //
    paramsMgr->BeginSaveStateGroup("Navigate scene");
}

// If the user presses the mouse on the active viz window,
// We record the position of the click.
//
void VizWin::mousePressEvent(QMouseEvent *e)
{
    _buttonNum = 0;
    _mouseClicked = true;

    if ((e->buttons() & Qt::LeftButton) && (e->buttons() & Qt::RightButton))
        ;    // do nothing
    else if (e->button() == Qt::LeftButton)
        _buttonNum = 1;
    else if (e->button() == Qt::RightButton)
        _buttonNum = 3;
    else if (e->button() == Qt::MidButton)
        _buttonNum = 2;
    // If ctrl + left button is pressed, only respond in navigation mode
    if ((_buttonNum == 1) && ((e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)))) { _buttonNum = 0; }

    if (_buttonNum == 0) {
        _mouseClicked = true;    // mouse button is held
        return;
    }

    string modeName = _getCurrentMouseMode();

    if (modeName == MouseModeParams::GetRegionModeName()) {
        std::vector<double> screenCoords = _getScreenCoords(e);

        glMatrixMode(GL_PROJECTION);    // Begin setup sequence
        glPushMatrix();
        _setUpProjMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        _setUpModelViewMatrix();    // End setup sequence

        bool mouseOnManip = _manip->MouseEvent(_buttonNum, screenCoords, _strHandleMid);

        swapBuffers();    // Begin cleanup sequence
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();    // End cleanup sequence

        if (mouseOnManip) { return; }
    }

    //	if (modeName == MouseModeParams::GetNavigateModeName()) {
    _mousePressEventNavigate(e);
    return;
    //	}
}

void VizWin::_mouseReleaseEventNavigate(QMouseEvent *e)
{
    _trackBall->MouseOnTrackball(2, _buttonNum, e->x(), e->y(), width(), height());
    _trackBall->TrackballSetMatrix();

    double m[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, m);

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    // Set modelview matrix in ViewpointParams. The Visualizer
    // will use download this to OpenGL
    //
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);
    vParams->SetModelViewMatrix(m);

    // Also need to set camera parameters in MouseModeParams
    //
    double posvec[3], upvec[3], dirvec[3];
    bool   status = vParams->ReconstructCamera(m, posvec, upvec, dirvec);
    assert(status);

    GUIStateParams * guiP = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    MouseModeParams *p = guiP->GetMouseModeParams();

    p->SetCameraPos(posvec);
    p->SetCameraViewDir(dirvec);
    p->SetCameraUpVec(upvec);

    paramsMgr->EndSaveStateGroup();
}

/*
 * If the user releases the mouse or moves it (with the left mouse down)
 * then we note the displacement
 */
void VizWin::mouseReleaseEvent(QMouseEvent *e)
{
    if (_buttonNum == 0) return;

    _mouseClicked = false;

    string modeName = _getCurrentMouseMode();

    if (modeName == MouseModeParams::GetRegionModeName()) {
        std::vector<double> screenCoords = _getScreenCoords(e);
        bool                b = _manip->MouseEvent(_buttonNum, screenCoords, _strHandleMid, true);
        if (!b)
            _mouseReleaseEventNavigate(e);
        else
            _setNewExtents();
    }

    if (modeName == MouseModeParams::GetNavigateModeName()) _mouseReleaseEventNavigate(e);

    _navigating = false;

#ifdef VAPOR3_0_0_ALPHA
    string                 tag = MouseModeParams::getModeTag(mode);
    TranslateStretchManip *myManip = _visualizer->getManip(tag);
    // Check if the seed bounds were moved
    if (myManip->draggingHandle() >= 0) {
        float screenCoords[2];
        screenCoords[0] = (float)e->x();
        screenCoords[1] = (float)(height() - e->y());
        setMouseDown(false, myManip);
        // The manip must move the region, and then tells the params to
        // record end of move
        myManip->mouseRelease(screenCoords);
        VizWinMgr::getInstance()->getEventRouter(tag)->captureMouseUp();
        VizWinParams::SetVizDirty(_windowNum);

    } else {    // otherwise fall through to navigate mode
        doNavigate = true;
    }
#endif

    _buttonNum = 0;
}

void VizWin::_mouseMoveEventNavigate(QMouseEvent *e)
{
    _trackBall->MouseOnTrackball(1, _buttonNum, e->x(), e->y(), width(), height());

    _trackBall->TrackballSetMatrix();

    double m[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, m);

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    // Set modelview matrix in ViewpointParams
    //
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);
    vParams->SetModelViewMatrix(m);
}

std::vector<double> VizWin::_getScreenCoords(QMouseEvent *e) const
{
    std::vector<double> screenCoords;
    screenCoords.push_back((double)e->x());
    screenCoords.push_back((double)(height() - e->y()));
    return screenCoords;
}

string VizWin::_getCurrentMouseMode() const
{
    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    GUIStateParams * guiP = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    MouseModeParams *p = guiP->GetMouseModeParams();
    string           modeName = p->GetCurrentMouseMode();
    return modeName;
}

void VizWin::_setNewExtents()
{
    std::vector<double> llc, urc;
    _manip->GetBox(llc, urc);
    VAPoR::RenderParams *rParams = _getRenderParams();
    if (rParams == NULL) return;
    VAPoR::Box *        box = rParams->GetBox();
    std::vector<double> pllc, purc;
    box->GetExtents(pllc, purc);

    box->SetExtents(llc, urc);
}

/*
 * When the mouse is moved, it can affect navigation,
 * region position, light position, or probe position, depending
 * on current mode.  The values associated with the window are
 * changed whether or not the tabbed panel is visible.
 *  It's important that coordinate changes eventually get recorded in the
 * viewpoint params panel.  This requires some work every time there is
 * mouse navigation.  Changes in the viewpoint params panel will notify
 * the viztab if it is active and change the values there.
 * Conversely, when values are changed in the viztab, the viewpoint
 * values are set in the VizWin class, provided they did not originally
 * come from the mouse navigation.  Such a change forces a reinitialization
 * of the trackball and the new values will be used at the next rendering.
 *
 */
void VizWin::mouseMoveEvent(QMouseEvent *e)
{
    if (_buttonNum == 0) return;

    string modeName = _getCurrentMouseMode();

    if (modeName == MouseModeParams::GetRegionModeName()) {
        if (!_navigating) {
            std::vector<double> screenCoords = _getScreenCoords(e);

            bool mouseOnManip = _manip->MouseEvent(_buttonNum, screenCoords, _strHandleMid);
            if (mouseOnManip)
                return;
            else
                _navigating = true;
        }
    }

    _mouseMoveEventNavigate(e);
    return;
}

void VizWin::setFocus() { QWidget::setFocus(); }

void VizWin::paintGL()
{
    if (!FrameBufferReady()) { return; }

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);

    bool enabled = _controlExec->GetSaveStateEnabled();
    _controlExec->SetSaveStateEnabled(false);
    vParams->SetWindowSize(width(), height());
    _controlExec->SetSaveStateEnabled(enabled);

    glClearColor(0., 0., 0., 1.);
    glClear(GL_COLOR_BUFFER_BIT);

    DataStatus *dataStatus = _controlExec->GetDataStatus();
    if (!dataStatus->GetDataMgrNames().size()) return;

    // Set up projection and modelview matrices
    //
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    _setUpProjMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    _setUpModelViewMatrix();

    int rc = _controlExec->Paint(_winName, false);
    if (rc < 0) { MSG_ERR("Paint failed"); }

    if (_getCurrentMouseMode() == MouseModeParams::GetRegionModeName()) updateManip();

    swapBuffers();

    rc = printOpenGLErrorMsg("VizWindowPaintGL");
    if (rc < 0) { MSG_ERR("OpenGL error"); }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

VAPoR::RenderParams *VizWin::_getRenderParams()
{
    string className;
    return _getRenderParams(className);
}

VAPoR::RenderParams *VizWin::_getRenderParams(string &className)
{
    ParamsMgr *     paramsMgr = _controlExec->GetParamsMgr();
    GUIStateParams *guiP = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());

    string inst, winName, dataSetName;
    guiP->GetActiveRenderer(_winName, className, inst);

    bool exists = paramsMgr->RenderParamsLookup(inst, winName, dataSetName, className);

    if (!exists) return NULL;

    //	VAPoR::RenderParams* rParams = _controlExec->GetRenderParams(
    //		_winName, dataSetName, className, inst
    //	);
    VAPoR::RenderParams *rParams = paramsMgr->GetRenderParams(_winName, dataSetName, className, inst);

    return rParams;
}

string VizWin::_getCurrentDataMgrName() const
{
    ParamsMgr *     paramsMgr = _controlExec->GetParamsMgr();
    GUIStateParams *guiP = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());

    string inst, winName, className, dataSetName;
    guiP->GetActiveRenderer(_winName, className, inst);

    bool exists = paramsMgr->RenderParamsLookup(inst, winName, dataSetName, className);

    if (!exists) return "";

    return dataSetName;
}

void VizWin::_getUnionOfFieldVarExtents(RenderParams *rParams, DataMgr *dataMgr, int timeStep, int refLevel, std::vector<double> &minExts, std::vector<double> &maxExts)
{
    vector<string> fieldVars = rParams->GetFieldVariableNames();
    for (int i = 0; i < 3; i++) {
        std::vector<double> tmpMin, tmpMax;
        string              varName = fieldVars[i];
        if (varName == "") continue;

        dataMgr->GetVariableExtents(timeStep, varName, refLevel, tmpMin, tmpMax);

        if (minExts.size() == 0) {
            for (int j = 0; j < 3; j++) {
                minExts = tmpMin;
                maxExts = tmpMax;
            }
        } else {
            for (int j = 0; j < 3; j++) {
                if (tmpMin[j] < minExts[j]) minExts[j] = tmpMin[j];
                if (tmpMax[j] > maxExts[j]) maxExts[j] = tmpMax[j];
            }
        }
    }
}

void VizWin::_getActiveExtents(std::vector<double> &minExts, std::vector<double> &maxExts)
{
    VAPoR::RenderParams *rParams = _getRenderParams();
    if (rParams == NULL) return;

    int            refLevel = rParams->GetRefinementLevel();
    string         varName = rParams->GetVariableName();
    vector<string> fieldVars = rParams->GetFieldVariableNames();

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    AnimationParams *aParams = (AnimationParams *)paramsMgr->GetParams(AnimationParams::GetClassType());
    int              timeStep = aParams->GetCurrentTimestep();

    DataStatus *dataStatus = _controlExec->GetDataStatus();
    string      dataMgrName = _getCurrentDataMgrName();
    DataMgr *   dataMgr = dataStatus->GetDataMgr(dataMgrName);

    if (fieldVars[0] == "" && fieldVars[1] == "" && fieldVars[2] == "") {
        dataMgr->GetVariableExtents(timeStep, varName, refLevel, minExts, maxExts);
    } else {
        _getUnionOfFieldVarExtents(rParams, dataMgr, timeStep, refLevel, minExts, maxExts);
    }
}

void VizWin::_getCenterAndCamPos(std::vector<double> &rotationCenter, std::vector<double> &cameraPosition)
{
    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    GUIStateParams * guiP = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    MouseModeParams *p = guiP->GetMouseModeParams();
    string           modeName = _getCurrentMouseMode();
    double           rotCenter[3], cameraPos[3];
    p->GetRotationCenter(rotCenter);
    p->GetCameraPos(cameraPos);

    rotationCenter.clear();
    cameraPosition.clear();
    for (int i = 0; i < 3; i++) {
        rotationCenter.push_back(rotCenter[i]);
        cameraPosition.push_back(cameraPos[i]);
    }
}

void VizWin::_getWindowSize(std::vector<int> &windowSize)
{
    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);

    size_t width, height;
    vParams->GetWindowSize(width, height);

    windowSize.push_back(width);
    windowSize.push_back(height);
}

VAPoR::Transform *VizWin::_getDataMgrTransform() const
{
    string dataMgrName = _getCurrentDataMgrName();
    if (dataMgrName.empty()) return NULL;

    ParamsMgr *       paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams * vpParams = paramsMgr->GetViewpointParams(_winName);
    vector<string>    names = paramsMgr->GetDataMgrNames();
    VAPoR::Transform *t = vpParams->GetTransform(dataMgrName);
    return t;
}

void VizWin::updateManip(bool initialize)
{
    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    std::vector<double> minExts(3, numeric_limits<double>::max());
    std::vector<double> maxExts(3, numeric_limits<double>::lowest());
    // std::vector<double> minExts;	// This dumps core...
    // std::vector<double> maxExts;	// This dumps core...
    _getActiveExtents(minExts, maxExts);

    std::vector<double> rotationCenter, cameraPosition;
    _getCenterAndCamPos(rotationCenter, cameraPosition);

    std::vector<int> windowSize;
    _getWindowSize(windowSize);

    double           mv[16];
    double           proj[16];
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);
    vParams->GetModelViewMatrix(mv);
    vParams->GetProjectionMatrix(proj);

    std::vector<double>  llc, urc;
    string               classType;
    VAPoR::RenderParams *rParams = _getRenderParams(classType);
    if (initialize || rParams == NULL) {
        llc = minExts;
        urc = maxExts;
    } else {
        VAPoR::Box *box = rParams->GetBox();
        box->GetExtents(llc, urc);
    }

    bool constrain = true;
    if (classType == ImageParams::GetClassType()) constrain = false;

    VAPoR::Transform *dmTransform = _getDataMgrTransform();
    VAPoR::Transform *rpTransform = NULL;
    if (rParams != NULL) rpTransform = rParams->GetTransform();

    _manip->Update(llc, urc, minExts, maxExts, cameraPosition, mv, proj, windowSize, rpTransform, dmTransform, constrain);

    _manip->Render();
}
