//************************************************************************
//									*
//		     Copyright (C)  2013				*
//     University Corporation for Atmospheric Research			*
//		     All Rights Reserved				*
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
#include <vapor/glutil.h> // Must be included first!!!
#include <cassert>
#include "VizWin.h"
#include <QResizeEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QCloseEvent>
#include <vapor/ControlExecutive.h>
#include <vapor/ViewpointParams.h>
#include <vapor/Viewpoint.h>
#include "TrackBall.h"
#include "TabManager.h"
#include "MouseModeParams.h"
#include "qdatetime.h"
#include "ViewpointEventRouter.h"
#include "MainForm.h"
#include "MessageReporter.h"
#include "images/vapor-icon-32.xpm"

using namespace VAPoR;

/*
 *  Constructs a VizWindow as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
VizWin::VizWin(
    MainForm *parent, const QString &name, VizWinMgr *myMgr,
    QRect *location, string vizName, ControlExec *ce, Trackball *trackBall) : QGLWidget(parent) {
    _mainForm = parent;
    m_trackBall = trackBall;

    setAttribute(Qt::WA_DeleteOnClose);
    m_vizName = vizName;
    setWindowIcon(QPixmap(vapor_icon___));
    _controlExec = ce;

    setAutoBufferSwap(false);
    _spinTimer = 0;
    _vizWinMgr = myMgr;
    _mouseClicked = false;
    _buttonNum = 0;

    setMouseTracking(false); // Only track mouse when button clicked/held
}
/*
 *  Destroys the object and frees any allocated resources
 */
VizWin::~VizWin() {
    if (_spinTimer)
        delete _spinTimer;
}
void VizWin::closeEvent(QCloseEvent *e) {

    //Tell the winmgr that we are closing:
    _vizWinMgr->vizAboutToDisappear(m_vizName);
    QWidget::closeEvent(e);
}
/******************************************************
 * React when focus is on window:
 ******************************************************/
void VizWin::focusInEvent(QFocusEvent *e) {
    //Test for hidden here, since a vanishing window can get this event.
    if (e->gotFocus() && !isHidden()) {

        GUIStateParams *p = _mainForm->GetStateParams();
        string vizName = p->GetActiveVizName();
        if (vizName != m_vizName) {
            _vizWinMgr->setActiveViz(m_vizName);
        }
    }
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
void VizWin::getNearFarDist(
    const double posVec[3], const double dirVec[3],
    double &boxNear, double &boxFar) const {

    //First check full box
    double wrk[3], cor[3], boxcor[3];
    double camPosBox[3], dvdir[3];
    double maxProj = -std::numeric_limits<double>::max();
    double minProj = std::numeric_limits<double>::max();

    DataStatus *dataStatus = _controlExec->getDataStatus();

    vector<double> minExts, maxExts;
    dataStatus->GetExtents(minExts, maxExts);

    for (int i = 0; i < 3; i++)
        camPosBox[i] = posVec[i];

    for (int i = 0; i < 3; i++)
        dvdir[i] = dirVec[i];
    vnormal(dvdir);

    //For each box corner,
    //   convert to box coords, then project to line of view
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 3; j++) {
            cor[j] = ((i >> j) & 1) ? maxExts[j] : minExts[j];
        }
        for (int k = 0; k < 3; k++)
            boxcor[k] = cor[k];

        vsub(boxcor, camPosBox, wrk);

        float mdist = abs(vdot(wrk, dvdir));
        if (minProj > mdist) {
            minProj = mdist;
        }
        if (maxProj < mdist) {
            maxProj = mdist;
        }
    }

    if (maxProj < 1.e-10)
        maxProj = 1.e-10;
    if (minProj > 0.03 * maxProj)
        minProj = 0.03 * maxProj;

    // minProj will be < 0 if either the camera is in the box, or
    // if some of the region is behind the camera plane.  In that case, just
    // set the nearDist a reasonable multiple of the fardist
    //
    if (minProj <= 0.0)
        minProj = 0.0002 * maxProj;
    boxFar = (float)maxProj;
    boxNear = (float)minProj;

    return;
}

void VizWin::setUpProjMatrix() {

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(m_vizName);

    double dirVec[3], posVec[3];
    vParams->GetCameraViewDir(dirVec);
    vParams->GetCameraPos(posVec);

    double nearDist, farDist;
    getNearFarDist(posVec, dirVec, nearDist, farDist);
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

void VizWin::setUpModelViewMatrix() {

    // Set the modelview matrix via the trackball
    //
    glLoadIdentity();
    m_trackBall->TrackballSetMatrix();

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(m_vizName);

    double m[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, m);

    vParams->SetModelViewMatrix(m);
}

// React to a user-change in window size/position (or possibly max/min)
// Either the window is minimized, maximized, restored, or just resized.
void VizWin::resizeGL(int width, int height) {

    if (!FrameBufferReady()) {
        return;
    }

    int rc1 = printOpenGLErrorMsg("GLVizWindowResizeEvent");

    int rc2 = _controlExec->ResizeViz(m_vizName, width, height);

    glViewport(0, 0, (GLint)width, (GLint)height);

    glClearColor(0., 0., 0., 1.);
    glClear(GL_COLOR_BUFFER_BIT);
    swapBuffers();

    // Necessary?
    //
    glClearColor(0., 0., 0., 1.);
    glClear(GL_COLOR_BUFFER_BIT);
    swapBuffers();

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(m_vizName);

    bool enabled = _controlExec->GetSaveStateEnabled();
    _controlExec->SetSaveStateEnabled(false);
    vParams->SetWindowSize(width, height);
    _controlExec->SetSaveStateEnabled(enabled);
}

void VizWin::initializeGL() {

    printOpenGLErrorMsg("GLVizWindowInitializeEvent");
    int rc2 = _controlExec->InitializeViz(m_vizName);
    if (rc2) {
        MessageReporter::errorMsg("Failure to initialize Visualizer, exiting\n");
    }
    printOpenGLErrorMsg("GLVizWindowInitializeEvent");
}

void VizWin::mousePressEventNavigate(QMouseEvent *e) {

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    paramsMgr->BeginSaveStateGroup("Navigate scene");

    // prepare for spin
    // Create a timer to use to measure how long between mouse moves:
    //
    if (_spinTimer)
        delete _spinTimer;

#ifdef DEAD
    if (_buttonNum == 1) {
        _spinTimer = new QTime();
        _spinTimer->start();
        _moveCount = 0;
        _olderMoveTime = _latestMoveTime = 0;
    }
#endif

#ifdef DEAD
    ViewpointEventRouter *vep = VizWinMgr::getInstance()->getViewpointRouter();
    vep->captureMouseDown(_buttonNum);
#endif

    m_trackBall->MouseOnTrackball(
        0, _buttonNum, e->x(), e->y(), width(), height());
}

/* If the user presses the mouse on the active viz window,
 * We record the position of the click.
 */
void VizWin::mousePressEvent(QMouseEvent *e) {

    _buttonNum = 0;
    if ((e->buttons() & Qt::LeftButton) && (e->buttons() & Qt::RightButton))
        ; //do nothing
    else if (e->button() == Qt::LeftButton)
        _buttonNum = 1;
    else if (e->button() == Qt::RightButton)
        _buttonNum = 3;
    else if (e->button() == Qt::MidButton)
        _buttonNum = 2;
    //If ctrl + left button is pressed, only respond in navigation mode
    if (
        (_buttonNum == 1) &&
        ((e->modifiers() & (Qt::ControlModifier | Qt::MetaModifier)))) {
        _buttonNum = 0;
    }
    if (_buttonNum == 0)
        return;

    MouseModeParams *p = _mainForm->GetStateParams()->GetMouseModeParams();
    string modeName = p->GetCurrentMouseMode();

    if (modeName == MouseModeParams::GetNavigateModeName()) {
        mousePressEventNavigate(e);
        return;
    }

    // To keep orientation correct in plane, and use
    // OpenGL convention (Y 0 at bottom of window), reverse
    // value of y:
    //
    double screenCoords[2];
    screenCoords[0] = (float)e->x() - 3.f;
    screenCoords[1] = (float)(height() - e->y()) - 5.f;

#ifdef DEAD

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    AnimationParams *p = paramsMgr->GetAnimationParams(m_vizName);
    int timestep = p->GetCurrentTimestep();

    double boxExtents[6];
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(m_vizName);
    assert(vParams);
    string tag = MouseModeParams::getModeTag(mode);
    Params *rParams = paramsMgr->GetCurrentParams(_windowNum, tag);
    if (!rParams)
        return;
    int handleNum = -1;
    int manipType;
    TranslateStretchManip *manip = 0;

    if (rParams) {
        manip = _visualizer->getManip(tag);
        manip->setParams(rParams);
        manipType = MouseModeParams::getModeManipType(mode);
        if (manipType != 3)
            rParams->GetBox()->GetStretchedLocalExtents(boxExtents, timestep); //non-rotated manip
        else
            rParams->GetBox()->calcContainingStretchedBoxExtents(boxExtents, true); //rotated
        handleNum = manip->mouseIsOverHandle(screenCoords, boxExtents, _strHandleMid);
    }
    if (handleNum >= 0 && manip->startHandleSlide(_visualizer, screenCoords, handleNum, rParams)) {

        //With manip type 2, need to prevent right mouse slide on orthogonal handle
        //With manip type 3, need to use orientation
        bool OK = true;  //Flag indicates whether the manip takes the mouse
        int orient = -1; //orientation of 2D boxes
        switch (manipType) {
        case 1: //3d box manip
            break;
        case 2: //2d box manip, ok if not right mouse on orthog direction
            //Do nothing if grabbing orthog direction with right mouse:
            orient = rParams->GetBox()->GetOrientation();
            if (buttonNum == 2 && ((handleNum == (orient + 3)) || (handleNum == (orient - 2)))) {
                OK = false;
            }
            break;
        case 3: //Rotate-stretch:  check if it's rotated too far
            if (buttonNum <= 1)
                break; //OK if left mouse button
            {
                bool doStretch = true;
                //check if the rotation angle is approx a multiple of 90 degrees:
                int tolerance = 20;
                const vector<double> angles = rParams->GetBox()->GetAngles();
                int thet = (int)(fabs(angles[0]) + 0.5f);
                int ph = (int)(fabs(angles[1]) + 0.5f);
                int ps = (int)(fabs(angles[2]) + 0.5f);
                //Make sure that these are within tolerance of a multiple of 90
                if (abs(((thet + 45) / 90) * 90 - thet) > tolerance)
                    doStretch = false;
                if (abs(((ps + 45) / 90) * 90 - ps) > tolerance)
                    doStretch = false;
                if (abs(((ph + 45) / 90) * 90 - ph) > tolerance)
                    doStretch = false;

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
        default:
            assert(0); //Invalid manip type
        }              //end switch
        if (OK) {
            double dirVec[3];
            //Find the direction vector of the camera ( Local coords)
            _visualizer->pixelToVector(screenCoords,
                                       vParams->getStretchedCamPosLocal(), dirVec, _strHandleMid);
            //Remember which handle we hit, highlight it, save the intersection point.
            manip->captureMouseDown(handleNum, vParams->getCameraPosLocal(), dirVec, buttonNum, _strHandleMid);
            EventRouter *rep = VizWinMgr::getInstance()->getEventRouter(tag);
            rep->captureMouseDown(buttonNum);
            setMouseDown(true, manip);
            VizWinParams::SetVizDirty(_windowNum);

        } //otherwise, fall through to navigation mode
    }
    //Set up for spin animation
#endif

    _mouseClicked = true;
}

void VizWin::mouseReleaseEventNavigate(QMouseEvent *e) {

    m_trackBall->MouseOnTrackball(
        2, _buttonNum, e->x(), e->y(), width(), height());

#ifdef DEAD
    setMouseDown(false);
    //If it's a right mouse being released, must update near/far distances:
    if (e->button() == Qt::RightButton) {
        //			_vizWinMgr->resetViews(
        //			_controlExec->GetViewpointParams(_windowNum));
    }
#endif
    // Decide whether or not to start a spin animation
    bool doSpin = (_buttonNum == 1 && _spinTimer);

    // Determine if the motion is sufficient to start a spin animation.
    // Require that some time has elapsed since the last move event, and,
    // to allow users to stop spin by holding mouse steady, make sure that
    // the time from the last move event is no more than 6 times the
    // difference between the last two move times.
    if (doSpin) {
        int latestTime = _spinTimer->elapsed();

        if (
            _moveDist > 3 &&
            _moveCount > 0 &&
            (latestTime - _latestMoveTime) < 6 * (_latestMoveTime - _olderMoveTime)) {
            //				_visualizer->startSpin(latestTime/_moveCount);
        } else {
            doSpin = false;
        }
#ifdef DEAD
        if (!doSpin) {
            //terminate current mouse motion
            _vizWinMgr->getEventRouter(ViewpointEventRouter::GetClassType())->captureMouseUp();
        }

        //Force redisplay of tab and visualizer
        _vizWinMgr->getEventRouter(ViewpointEventRouter::GetClassType())->updateTab();
#endif
    }

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    paramsMgr->EndSaveStateGroup();
}

/*
 * If the user releases the mouse or moves it (with the left mouse down)
 * then we note the displacement
 */
void VizWin::mouseReleaseEvent(QMouseEvent *e) {

    if (_buttonNum == 0)
        return;

    MouseModeParams *p = _mainForm->GetStateParams()->GetMouseModeParams();
    string modeName = p->GetCurrentMouseMode();

    if (modeName == MouseModeParams::GetNavigateModeName()) {
        mouseReleaseEventNavigate(e);
        _buttonNum = 0;
        return;
    }

#ifdef DEAD
    string tag = MouseModeParams::getModeTag(mode);
    TranslateStretchManip *myManip = _visualizer->getManip(tag);
    //Check if the seed bounds were moved
    if (myManip->draggingHandle() >= 0) {
        float screenCoords[2];
        screenCoords[0] = (float)e->x();
        screenCoords[1] = (float)(height() - e->y());
        setMouseDown(false, myManip);
        //The manip must move the region, and then tells the params to
        //record end of move
        myManip->mouseRelease(screenCoords);
        VizWinMgr::getInstance()->getEventRouter(tag)->captureMouseUp();
        VizWinParams::SetVizDirty(_windowNum);

    } else { //otherwise fall through to navigate mode
        doNavigate = true;
    }
#endif

#ifdef DEAD
    if (_mouseClicked) {
        //there was just a click in the window, with no mouse move
        reallyUpdate();
        _mouseClicked = false;
    } else {
        paintGL();
    }
#endif
    _buttonNum = 0;
}

void VizWin::mouseMoveEventNavigate(QMouseEvent *e) {

#ifdef DEAD
    if (_spinTimer) {
        _moveCount++;
        if (_moveCount > 0) { //find distance from last move event...
            _moveDist = abs(_moveCoords[0] - e->x()) + abs(_moveCoords[1] - e->y());
        }
        _moveCoords[0] = e->x();
        _moveCoords[1] = e->y();
        int latestTime = _spinTimer->elapsed();
        _olderMoveTime = _latestMoveTime;
        _latestMoveTime = latestTime;
    }

#endif

    m_trackBall->MouseOnTrackball(
        1, _buttonNum, e->x(), e->y(), width(), height());

    //Note that the coords have changed in the trackball:
#ifdef DEAD
    _visualizer->SetTrackballCoordsChanged(true);
    VizWinParams::SetVizDirty(_windowNum);
#endif
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
void VizWin::mouseMoveEvent(QMouseEvent *e) {

    if (_buttonNum == 0)
        return;

    MouseModeParams *p = _mainForm->GetStateParams()->GetMouseModeParams();
    string modeName = p->GetCurrentMouseMode();

    if (modeName == MouseModeParams::GetNavigateModeName()) {
        mouseMoveEventNavigate(e);
        return;
    }

    //Respond based on what activity we are tracking
    //Need to tell the appropriate params about the change,
    //And it should refresh the panel
    double mouseCoords[2];
    double projMouseCoords[2];
    mouseCoords[0] = (float)e->x();
    mouseCoords[1] = (float)height() - e->y();

#ifdef DEAD

    string tag = MouseModeParams::getModeTag(mode);
    TranslateStretchManip *manip = _visualizer->getManip(tag);
    //bool constrain = manip->getParams()->isDomainConstrained();
    bool constrain = true;
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_windowNum);
    assert(vParams);
    int handleNum = manip->draggingHandle();
    //check first to see if we are dragging face
    if (handleNum >= 0) {
        if (manip->projectPointToLine(mouseCoords, projMouseCoords)) {
            double dirVec[3];
            _visualizer->pixelToVector(projMouseCoords, vParams->getStretchedCamPosLocal(), dirVec, _strHandleMid);
            //qWarning("Sliding handle %d, direction %f %f %f", handleNum, dirVec[0],dirVec[1],dirVec[2]);
            manip->slideHandle(handleNum, dirVec, constrain);
            VizWinParams::SetVizDirty(_windowNum);
            doNavigate = false;
        }
    }
#endif

#ifdef DEAD
    paintGL();
#endif
    return;
}

void VizWin::setFocus() {
    //qWarning("Setting Focus in win %d", _windowNum);
    //??QMainWindow::setFocus();
    QWidget::setFocus();
}

void VizWin::paintGL() {

    if (!FrameBufferReady()) {
        return;
    }

    // Set up projection and modelview matrices
    //

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    setUpProjMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    setUpModelViewMatrix();

    // Only rendering if state has changed. Can't check until after
    // View and model matrices are set up
    //
    if (!_controlExec->GetParamsMgr()->StateChanged()) {
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        return;
    }

#ifdef DEAD
    double m[16];
    cout << "Projection matrix: " << endl;
    glGetDoublev(GL_PROJECTION_MATRIX, m);
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            cout << m[j * 4 + i] << " ";
        }
        cout << endl;
    }
    cout << endl;

    cout << "ModelView matrix: " << endl;
    glGetDoublev(GL_MODELVIEW_MATRIX, m);
    for (int j = 0; j < 4; j++) {
        for (int i = 0; i < 4; i++) {
            cout << m[j * 4 + i] << " ";
        }
        cout << endl;
    }
    cout << endl;
#endif

    //only paint if necessary
    //Note that makeCurrent is needed when here we have multiple windows.

#ifdef DEAD
    static bool first = true;
    //first paint, just clear front and back buffers.
    if (first && m_vizName.empty()) {
        glClearColor(0., 0., 0., 1.);
        glClear(GL_COLOR_BUFFER_BIT);
        swapBuffers();
        glClear(GL_COLOR_BUFFER_BIT);
        first = false;

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();

        return;
    }
    int rc0 = printOpenGLErrorMsg("VizWindowPaintGL");
#endif

    _controlExec->Paint(m_vizName, false);
    swapBuffers();
    printOpenGLErrorMsg("VizWindowPaintGL");

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void VizWin::reallyUpdate() {
    makeCurrent();
    _controlExec->Paint(m_vizName, true);
    swapBuffers();
    return;
}

void VizWin::SetTrackBall(
    const double posvec[3], const double dirvec[3],
    const double upvec[3], const double centerRot[3],
    bool perspective) {
    makeCurrent();

    m_trackBall->setFromFrame(posvec, dirvec, upvec, centerRot, true);

    glPushMatrix();
    m_trackBall->TrackballSetMatrix();

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(m_vizName);

    double m[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, m);
    glPopMatrix();

    vParams->SetModelViewMatrix(m);
    vParams->SetRotationCenter(centerRot);
}
