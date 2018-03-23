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
//		This is the QGLWidget that performs OpenGL rendering (using associated Visualizer)
//		Plus supports mouse event reporting
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

    setMouseTracking(false);    // Only track mouse when button clicked/held

    _manip = new TranslateStretchManip();
    bool initialize = true;
    updateManip(initialize);
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
void VizWin::getNearFarDist(const double posVec[3], const double dirVec[3], double &boxNear, double &boxFar) const
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

void VizWin::setUpProjMatrix()
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
    getNearFarDist(posvec, dirvec, nearDist, farDist);
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

void VizWin::setUpModelViewMatrix()
{
    makeCurrent();    // necessary?

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    GUIStateParams * guiP = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    MouseModeParams *p = guiP->GetMouseModeParams();
    string           modeName = p->GetCurrentMouseMode();

    // If currently navigating with mouse set matrix from trackball
    //
    if (_mouseClicked && modeName == MouseModeParams::GetNavigateModeName()) {
        // Set the modelview matrix via the trackball
        //
        glLoadIdentity();
        _trackBall->TrackballSetMatrix();
    } else {
        // Else we set trackball from params
        //
        double center[3], posvec[3], dirvec[3], upvec[3];
        p->GetRotationCenter(center);
        p->GetCameraPos(posvec);
        p->GetCameraViewDir(dirvec);
        p->GetCameraUpVec(upvec);

        _trackBall->setFromFrame(posvec, dirvec, upvec, center, true);
        _trackBall->TrackballSetMatrix();
    }

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
}

void VizWin::initializeGL()
{
    printOpenGLErrorMsg("GLVizWindowInitializeEvent");
    int rc = _controlExec->InitializeViz(_winName);
    if (rc < 0) { MSG_ERR("Failure to initialize Visualizer"); }
    printOpenGLErrorMsg("GLVizWindowInitializeEvent");

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);

    bool enabled = _controlExec->GetSaveStateEnabled();
    _controlExec->SetSaveStateEnabled(false);
    vParams->SetWindowSize(width(), height());
    _controlExec->SetSaveStateEnabled(enabled);
}

void VizWin::mousePressEventNavigate(QMouseEvent *e)
{
    // Let trackball handle mouse events for navigation
    //
    _trackBall->MouseOnTrackball(0, _buttonNum, e->x(), e->y(), width(), height());

    // Create a state saving group.
    // Only save camera parameters after user release mouse
    //
    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    paramsMgr->BeginSaveStateGroup("Navigate scene");
}

// If the user presses the mouse on the active viz window,
// We record the position of the click.
//
void VizWin::mousePressEvent(QMouseEvent *e)
{
    _buttonNum = 0;
    _mouseClicked = true;

    //	double screenCoords[2];
    //	screenCoords[0] = (float)e->x()-3.f;
    //	screenCoords[1] = (float)(height() - e->y()) - 5.f;
    // double coords[2] = {(double)e->x(), (double)e->y()};

    std::vector<double> screenCoords;
    screenCoords.push_back((double)e->x() - 3.f);
    screenCoords.push_back((double)(height() - e->y()) - 5.f);

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

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    GUIStateParams * guiP = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    MouseModeParams *p = guiP->GetMouseModeParams();
    string           modeName = p->GetCurrentMouseMode();

    bool mouseOnManip = _manip->MouseEvent(_buttonNum, screenCoords);
    if (mouseOnManip) { return; }

    /*	int handle = _manip->mouseIsOverHandle(screenCoords, _strHandleMid);
    if (handle >= 0) {
        double dirVec[3];
        _manip->pixelToVector(screenCoords, dirVec, _strHandleMid);
        double startCoords[3];
        _manip->captureMouseDown(handle, _buttonNum, _strHandleMid);//screenCoords);
        _manip->startHandleSlide(screenCoords, handle);
        _manip->setMouseDown(true);
*/

    /*
528             double dirVec[3];
529             //Find the direction vector of the camera ( Local coords)
530             _visualizer->pixelToVector(screenCoords,
531                 vParams->getStretchedCamPosLocal(), dirVec, _strHandleMid);
532             //Remember which handle we hit, highlight it, save the intersection point.
533             manip->captureMouseDown(handleNum,  vParams->getCameraPosLocal(), dirVec, buttonNum, _strHandleMid);
534             EventRouter* rep = VizWinMgr::getInstance()->getEventRouter(tag);
535             rep->captureMouseDown(buttonNum);
536             setMouseDown(true, manip);
*/

    //	}
    if (modeName == MouseModeParams::GetNavigateModeName()) {
        mousePressEventNavigate(e);
        return;
    }

#ifdef DEAD
    // To keep orientation correct in plane, and use
    // OpenGL convention (Y 0 at bottom of window), reverse
    // value of y:
    //
    double screenCoords[2];
    screenCoords[0] = (float)e->x() - 3.f;
    screenCoords[1] = (float)(height() - e->y()) - 5.f;

    // double coords[2] = {(double)e->x(), (double)e->y()};

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    AnimationParams *p = paramsMgr->GetAnimationParams(_winName);
    int              timestep = p->GetCurrentTimestep();

    double           boxExtents[6];
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);
    assert(vParams);
    string  tag = MouseModeParams::getModeTag(mode);
    Params *rParams = paramsMgr->GetCurrentParams(_windowNum, tag);
    if (!rParams) return;
    int                    handleNum = -1;
    int                    manipType;
    TranslateStretchManip *manip = 0;

    if (rParams) {
        manip = _visualizer->getManip(tag);
        manip->setParams(rParams);
        manipType = MouseModeParams::getModeManipType(mode);
        if (manipType != 3)
            rParams->GetBox()->GetStretchedLocalExtents(boxExtents, timestep);    // non-rotated manip
        else
            rParams->GetBox()->calcContainingStretchedBoxExtents(boxExtents, true);    // rotated
        handleNum = manip->mouseIsOverHandle(screenCoords, boxExtents, _strHandleMid);
    }
    if (handleNum >= 0 && manip->startHandleSlide(_visualizer, screenCoords, handleNum, rParams)) {
        // With manip type 2, need to prevent right mouse slide on orthogonal handle
        // With manip type 3, need to use orientation
        bool OK = true;      // Flag indicates whether the manip takes the mouse
        int  orient = -1;    // orientation of 2D boxes
        switch (manipType) {
        case 1:    // 3d box manip
            break;
        case 2:    // 2d box manip, ok if not right mouse on orthog direction
            // Do nothing if grabbing orthog direction with right mouse:
            orient = rParams->GetBox()->GetOrientation();
            if (buttonNum == 2 && ((handleNum == (orient + 3)) || (handleNum == (orient - 2)))) { OK = false; }
            break;
        case 3:                           // Rotate-stretch:  check if it's rotated too far
            if (buttonNum <= 1) break;    // OK if left mouse button
            {
                bool doStretch = true;
                // check if the rotation angle is approx a multiple of 90 degrees:
                int                  tolerance = 20;
                const vector<double> angles = rParams->GetBox()->GetAngles();
                int                  thet = (int)(fabs(angles[0]) + 0.5f);
                int                  ph = (int)(fabs(angles[1]) + 0.5f);
                int                  ps = (int)(fabs(angles[2]) + 0.5f);
                // Make sure that these are within tolerance of a multiple of 90
                if (abs(((thet + 45) / 90) * 90 - thet) > tolerance) doStretch = false;
                if (abs(((ps + 45) / 90) * 90 - ps) > tolerance) doStretch = false;
                if (abs(((ph + 45) / 90) * 90 - ph) > tolerance) doStretch = false;

                if (!doStretch) {
                    /*
                        MessageReporter::warningMsg("Manipulator is not axis-aligned.\n%s %s %s",
                            "To stretch or shrink,\n",
                            "You must use the size\n",
                            "(sliders or text) in the tab.");
                            */
                    OK = false;
                }
            }
            break;
        default: assert(0);    // Invalid manip type
        }                      // end switch
        if (OK) {
            double dirVec[3];
            // Find the direction vector of the camera ( Local coords)
            _visualizer->pixelToVector(screenCoords, vParams->getStretchedCamPosLocal(), dirVec, _strHandleMid);
            // Remember which handle we hit, highlight it, save the intersection point.
            manip->captureMouseDown(handleNum, vParams->getCameraPosLocal(), dirVec, buttonNum, _strHandleMid);
            EventRouter *rep = VizWinMgr::getInstance()->getEventRouter(tag);
            rep->captureMouseDown(buttonNum);
            setMouseDown(true, manip);
            VizWinParams::SetVizDirty(_windowNum);

        }    // otherwise, fall through to navigation mode
    }
    // Set up for spin animation
#endif
}

void VizWin::mouseReleaseEventNavigate(QMouseEvent *e)
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

    //	float screenCoords[2];
    //	screenCoords[0] = (float)e->x()-3.f;
    //	screenCoords[1] = (float)(height() - e->y()) - 5.f;
    std::vector<double> screenCoords;
    screenCoords.push_back((double)e->x() - 3.f);
    screenCoords.push_back((double)(height() - e->y()) - 5.f);
    //	_manip->setMouseDown(false);
    //	_manip->mouseRelease(screenCoords);
    _manip->MouseEvent(_buttonNum, screenCoords);

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    GUIStateParams * guiP = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    MouseModeParams *p = guiP->GetMouseModeParams();
    string           modeName = p->GetCurrentMouseMode();
    if (modeName == MouseModeParams::GetNavigateModeName()) {
        mouseReleaseEventNavigate(e);
        _buttonNum = 0;
        return;
    }

#ifdef DEAD
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

void VizWin::mouseMoveEventNavigate(QMouseEvent *e)
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

    //	double screenCoords[2];
    //	screenCoords[0] = (float)e->x()-3.f;
    //	screenCoords[1] = (float)(height() - e->y()) - 5.f;
    //	double coords[2] = {(double)e->x(), (double)e->y()};

    std::vector<double> screenCoords;
    screenCoords.push_back((double)e->x() - 3.f);
    screenCoords.push_back((double)(height() - e->y()) - 5.f);

    bool mouseOnManip = _manip->MouseEvent(_buttonNum, screenCoords);
    if (mouseOnManip) {
        cout << "Mouse is on manip" << endl;
        return;
    }
    cout << "Mouse not on manip" << endl;
    /*	int handle = _manip->draggingHandle();
    if (handle >= 0) {
        //Respond based on what activity we are tracking
        //Need to tell the appropriate params about the change,
        //And it should refresh the panel
//		double mouseCoords[2];
//		double projMouseCoords[2];
//		mouseCoords[0] = (float) e->x();
//		mouseCoords[1] = (float) height()-e->y();

        double projScreenCoords[2];
//		if (_manip->projectPointToLine(mouseCoords,projMouseCoords)) {
        if (_manip->projectPointToLine(screenCoords,projScreenCoords)) {
            double dirVec[3];
            //_manip->pixelToVector(projMouseCoords, dirVec, _strHandleMid);
            _manip->pixelToVector(projScreenCoords, dirVec, _strHandleMid);
            _manip->slideHandle(handle, dirVec, false);
        }
        return;
    }
*/

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    GUIStateParams * guiP = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    MouseModeParams *p = guiP->GetMouseModeParams();

    string modeName = p->GetCurrentMouseMode();

    if (modeName == MouseModeParams::GetNavigateModeName()) {
        mouseMoveEventNavigate(e);
        return;
    }

#ifdef DEAD

    string                 tag = MouseModeParams::getModeTag(mode);
    TranslateStretchManip *manip = _visualizer->getManip(tag);
    // bool constrain = manip->getParams()->isDomainConstrained();
    bool             constrain = true;
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_windowNum);
    assert(vParams);
    int handleNum = manip->draggingHandle();
    // check first to see if we are dragging face
    if (handleNum >= 0) {
        if (manip->projectPointToLine(mouseCoords, projMouseCoords)) {
            double dirVec[3];
            _visualizer->pixelToVector(projMouseCoords, vParams->getStretchedCamPosLocal(), dirVec, _strHandleMid);
            // qWarning("Sliding handle %d, direction %f %f %f", handleNum, dirVec[0],dirVec[1],dirVec[2]);
            manip->slideHandle(handleNum, dirVec, constrain);
            VizWinParams::SetVizDirty(_windowNum);
            doNavigate = false;
        }
    }
#endif
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

    // Only rendering if state has changed.
    //
    //	if (! _controlExec->GetParamsMgr()->StateChanged()) {
    //		glMatrixMode(GL_PROJECTION);
    //		glPopMatrix();
    //		glMatrixMode(GL_MODELVIEW);
    //		glPopMatrix();
    //		return;
    //	}

    // Set up projection and modelview matrices
    //
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    setUpProjMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    setUpModelViewMatrix();

#ifdef DEBUG1
    double m[16];
    cout << "Projection matrix: " << endl;
    glGetDoublev(GL_PROJECTION_MATRIX, m);
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) { cout << m[j * 4 + i] << " "; }
        cout << endl;
    }
    cout << endl;

    cout << "ModelView matrix: " << endl;
    glGetDoublev(GL_MODELVIEW_MATRIX, m);
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) { cout << m[j * 4 + i] << " "; }
        cout << endl;
    }
    cout << endl;
#endif

    int rc = _controlExec->Paint(_winName, false);
    if (rc < 0) { MSG_ERR("Paint failed"); }

    updateManip();

    swapBuffers();

    rc = printOpenGLErrorMsg("VizWindowPaintGL");
    if (rc < 0) { MSG_ERR("OpenGL error"); }

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void VizWin::updateManip(bool initialize)
{
    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    GUIStateParams *guiP = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    /*
// ParamsMgr or ControlExec can do this
335  RenderParams *GetRenderParams(
336     string winName, string dataSetName, string renderType,  string instName
337  ) const;

    string type, inst;
    guiP->GetActiveRenderer(_winName, type, inst);
    string dataSet =
    RenderParams* rParams = _controlExec->GetRenderParams(
        _winName, dataSet, type, inst
    );
*/
    vector<double> minExts, maxExts;
    //  ds->GetActiveExtents(pm, _winName, 0, minExts, maxExts);
    minExts.push_back(-1.038e+06);
    minExts.push_back(2.81684e+06);
    minExts.push_back(0.f);
    maxExts.push_back(220425);
    maxExts.push_back(4.04548e+06);
    maxExts.push_back(100000);

    // double m[16];
    // vParams->GetModelViewMatrix(m);
    // double posvec[3], upvec[3], dirvec[3];
    // bool status = vParams->ReconstructCamera(m, posvec, upvec, dirvec);

    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);
    MouseModeParams *p = guiP->GetMouseModeParams();
    string           modeName = p->GetCurrentMouseMode();
    double           rotCenter[3], cameraPos[3], dirvec[3], upvec[3];
    p->GetRotationCenter(rotCenter);
    p->GetCameraPos(cameraPos);

    std::vector<double> vrotCenter;
    std::vector<double> vcameraPos;
    for (int i = 0; i < 3; i++) {
        vrotCenter.push_back(rotCenter[i]);
        vcameraPos.push_back(cameraPos[i]);
    }

    size_t width, height;
    vParams->GetWindowSize(width, height);
    std::vector<int> windowSize;
    windowSize.push_back(width);
    windowSize.push_back(height);

    double mv[16];
    vParams->GetModelViewMatrix(mv);
    double proj[16];
    vParams->GetProjectionMatrix(proj);

    std::vector<double> llc, urc;
    if (initialize) {
        llc = minExts;
        urc = maxExts;
    } else
        _manip->GetBox(llc, urc);    // get box from active RenderParams

    _manip->Update(llc, urc, minExts, maxExts, vcameraPos, vrotCenter, mv, proj, windowSize);

    _manip->render();
}
