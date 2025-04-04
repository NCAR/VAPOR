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
#include "vapor/VAssert.h"
#include <QResizeEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QApplication>
#include <QWindow>
#include <QDesktopWidget>
#include <QIcon>
#include <vapor/ControlExecutive.h>
#include <vapor/ViewpointParams.h>
#include <vapor/Viewpoint.h>
#include <vapor/debug.h>
#include <vapor/ImageParams.h>
#include <vapor/TrackBall.h>
#include <vapor/GUIStateParams.h>
#include <vapor/MouseModeParams.h>
#include <vapor/AnimationParams.h>
#include "qdatetime.h"
#include "ErrorReporter.h"
#include "RenderEventRouterGUI.h"
#include "FlowEventRouter.h"
#include "images/vapor-icon-32.xpm"
#include "VizWin.h"
#include "Core3_2_context.h"
#include <glm/gtc/type_ptr.hpp>
#include "vapor/GLManager.h"
#include "vapor/LegacyGL.h"
#include "vapor/FontManager.h"
#include "vapor/FileUtils.h"
#include "vapor/Visualizer.h"
#include <vapor/FlowParams.h>
#include <vapor/SliceParams.h>
#include <vapor/ContourParams.h>
#define INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
#include <vapor/LegacyVectorMath.h>
#include "hide_std_error_util.h"

using namespace VAPoR;

#define ORIGIN_TIC_WIDTH .03

/*
 *  Constructs a VizWindow as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 */
VizWin::VizWin(const QGLFormat &format, QWidget *parent, const QString &name, string winName, ControlExec *ce, Trackball *trackBall) : QGLWidget(new Core3_2_context(format), parent)
{
    _trackBall = trackBall;

    setAttribute(Qt::WA_DeleteOnClose);
    _winName = winName;
    setWindowIcon(QPixmap(vapor_icon___));
    _controlExec = ce;

    _glManager = nullptr;
    _manip = nullptr;

    setAutoBufferSwap(false);
    _mouseClicked = false;
    _buttonNum = 0;
    _navigateFlag = false;
    _manipFlag = false;
    _openGLInitFlag = false;

    setMouseTracking(false);    // Only track mouse when button clicked/held
}


VizWin::~VizWin()
{
    // Sometimes when closing the application this will crash as the OpenGL context is
    // destroyed prior to the destructor being called.
    // Qt specifically states you are supposed to call ::makeCurrent() in the destructor.
    // https://doc.qt.io/qt-6/qopenglwidget.html#resource-initialization-and-cleanup
    // TODO Migrate to QOpenGLWidget
    this->makeCurrent();
    delete _glManager;
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

    VAPoR::CoordType minExts, maxExts;
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
    MatrixManager *  mm = _glManager->matrixManager;

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
    width *= this->devicePixelRatioF();
    height *= this->devicePixelRatioF();
    int wWidth = width;
    int wHeight = height;

    if (vParams->GetValueLong(ViewpointParams::UseCustomFramebufferTag, 0)) {
        width = vParams->GetValueLong(ViewpointParams::CustomFramebufferWidthTag, 0);
        height = vParams->GetValueLong(ViewpointParams::CustomFramebufferHeightTag, 0);
        if (width == 0) width = 1;
        if (height == 0) height = 1;

        int maxSize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
        if (width > maxSize) {
            width = maxSize;
            vParams->SetValueLong(ViewpointParams::CustomFramebufferWidthTag, ViewpointParams::CustomFramebufferWidthTag, width);
            MSG_ERR("Selected width is larger than your OpenGL implementation supports");
        }
        if (height > maxSize) {
            height = maxSize;
            vParams->SetValueLong(ViewpointParams::CustomFramebufferHeightTag, ViewpointParams::CustomFramebufferHeightTag, height);
            MSG_ERR("Selected height is larger than your OpenGL implementation supports");
        }

        float fa = width / (float)height;
        float wa = wWidth / (float)wHeight;

        if (fa >= wa) {
            int x = 0;
            int y = (wHeight / 2) - (wHeight / fa * wa / 2);
            int w = wWidth;
            int h = wHeight / fa * wa;
            glViewport(x, y, w, h);
        } else {
            int x = (wWidth / 2) - (wWidth * fa / wa / 2);
            int y = 0;
            int w = wWidth * fa / wa;
            int h = wHeight;
            glViewport(x, y, w, h);
        }
    } else {
        glViewport(0, 0, width, height);
    }

    mm->MatrixModeProjection();
    mm->LoadIdentity();

    GLfloat w = (float)width / (float)height;

    if (vParams->GetProjectionType() == ViewpointParams::MapOrthographic) {
        float s = _trackBall->GetOrthoSize();
        mm->Ortho(-s * w, s * w, -s, s, nearDist, farDist);
    } else {
        double fov = vParams->GetFOV();
        mm->Perspective(glm::radians(fov), w, nearDist, farDist);
    }

    double pMatrix[16];
    mm->GetDoublev(MatrixManager::Mode::Projection, pMatrix);

    bool enabled = _controlExec->GetSaveStateEnabled();
    _controlExec->SetSaveStateEnabled(false);

    vParams->SetProjectionMatrix(pMatrix);

    if (vParams->GetProjectionType() == ViewpointParams::MapOrthographic) vParams->SetOrthoProjectionSize(_trackBall->GetOrthoSize());

    _controlExec->SetSaveStateEnabled(enabled);

    mm->MatrixModeModelView();
}

void VizWin::_setUpModelViewMatrix()
{
    makeCurrent();    // necessary?

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);

    double m[16];
    vParams->GetModelViewMatrix(m);
    _glManager->matrixManager->LoadMatrixd(m);
}

// React to a user-change in window size/position (or possibly max/min)
// Either the window is minimized, maximized, restored, or just resized.
//
void VizWin::resizeGL(int width, int height)
{
    _resizeGL(width, height);
    if (paintOnResize)
        Render(true);
}

void VizWin::_resizeGL(int width, int height)
{
    if (!_openGLInitFlag || !FrameBufferReady()) { return; }

    int rc = CheckGLErrorMsg("GLVizWindowResizeEvent");
    if (rc < 0) { MSG_ERR("OpenGL error"); }

    rc = _controlExec->ResizeViz(_winName, width, height);
    if (rc < 0) { MSG_ERR("OpenGL error"); }

    glViewport(0, 0, (GLint)width, (GLint)height);

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
    bool ok = gladLoaderLoadGL();
    VAssert(GLManager::CheckError());
    if (!ok) {
        MyBase::SetErrMsg("Error: Unable to initialize GLAD");
        MSG_ERR("OpenGL Error");
    }
    
    _glManager = new GLManager;
    _manip = new TranslateStretchManip(_glManager);
    bool initialize = true;
    updateManip(initialize);

    CheckGLErrorMsg("GLVizWindowInitializeEvent");
    int rc = _controlExec->InitializeViz(_winName, _glManager);
    if (rc < 0) {
        MSG_FATAL("Failure to initialize Visualizer");
        return;
    }
    _glManager->legacy->Initialize();
    CheckGLErrorMsg("GLVizWindowInitializeEvent");

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);

    if (vParams != NULL) {
        bool enabled = _controlExec->GetSaveStateEnabled();
        _controlExec->SetSaveStateEnabled(false);
        vParams->SetWindowSize(width(), height());
        _controlExec->SetSaveStateEnabled(enabled);
    }

    _openGLInitFlag = true;
}

void VizWin::paintGL()
{
    if (_swapBufferQueued) {
        swapBuffers();
        _swapBufferQueued = false;
    }
}

void VizWin::safeSwapBuffers()
{
    if (windowHandle()->isExposed())
        swapBuffers();
    else
        _swapBufferQueued = true;
}

void VizWin::_mousePressEventManip(QMouseEvent *e)
{
    makeCurrent();

    std::vector<double> screenCoords = _getScreenCoords(e);

    screenCoords[0] *= this->devicePixelRatioF();
    screenCoords[1] *= this->devicePixelRatioF();

    _manipFlag = _manip->MouseEvent(_buttonNum, screenCoords, _strHandleMid);
}

void VizWin::_mousePressEventNavigate(QMouseEvent *e)
{
    _navigateFlag = true;

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    double           m[16];
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);
    vParams->GetModelViewMatrix(m);

    double center[3];
    vParams->GetRotationCenter(center);

    double posvec[3], upvec[3], dirvec[3];
    bool   status = vParams->ReconstructCamera(m, posvec, upvec, dirvec);
    VAssert(status);

    _trackBall->setFromFrame(posvec, dirvec, upvec, center, true);

    int trackballButtonNumber = _buttonNum;
    if (vParams->GetProjectionType() == ViewpointParams::MapOrthographic && _buttonNum == 1) trackballButtonNumber = 2;

    _trackBall->MouseOnTrackball(0, trackballButtonNumber, e->x(), e->y(), width(), height());

    _navigationPendingChanges = false;
}

void VizWin::mousePressEvent(QMouseEvent *e)
{
    if (_mouseClicked) return;

    _buttonNum = 0;
    _mouseClicked = true;

    if ((e->buttons() & Qt::LeftButton) && (e->buttons() & Qt::RightButton))
        ;    // do nothing
    else if (e->button() == Qt::LeftButton)
        _buttonNum = 1;
    else if (e->button() == Qt::RightButton)
        _buttonNum = 3;
    else if (e->button() == Qt::MiddleButton)
        _buttonNum = 2;

    // ControlModifier means [command], not [control] apparently
    if (e->button() == Qt::LeftButton && (e->modifiers() & Qt::ShiftModifier)) { _buttonNum = 2; }

    if (_buttonNum == 0) {
        _mouseClicked = true;    // mouse button is held
        return;
    }

    string modeName = _getCurrentMouseMode();

    if (modeName == MouseModeParams::GetRegionModeName()) {
        _mousePressEventManip(e);

        // Only manipulating if user managed to grab manipulator handle.
        // Otherwise we navigate
        //
        if (_manipFlag) { return; }
    }

    _mousePressEventNavigate(e);
}

void VizWin::_mouseReleaseEventManip(QMouseEvent *e)
{
    if (!_manipFlag) return;

    std::vector<double> screenCoords = _getScreenCoords(e);

    screenCoords[0] *= this->devicePixelRatioF();
    screenCoords[1] *= this->devicePixelRatioF();

    (void)_manip->MouseEvent(_buttonNum, screenCoords, _strHandleMid, true);
    _setNewExtents();

    _manipFlag = false;
}

void VizWin::_mouseReleaseEventNavigate(QMouseEvent *e)
{
    if (!_navigateFlag) return;

    _trackBall->MouseOnTrackball(2, _buttonNum, e->x(), e->y(), width(), height());
    _trackBall->TrackballSetMatrix();

    const double *m = _trackBall->GetModelViewMatrix();

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);

    if (_navigationPendingChanges) {
        vParams->SetModelViewMatrix(m);
        paramsMgr->EndSaveStateGroup();
        emit EndNavigation(_winName);
    }
    _navigationPendingChanges = false;
    _navigateFlag = false;
}

/*
 * If the user releases the mouse or moves it (with the left mouse down)
 * then we note the displacement
 */
void VizWin::mouseReleaseEvent(QMouseEvent *e)
{
    if (_buttonNum == 0) {
        _mouseClicked = false;
        return;
    }

    _mouseClicked = false;

    if (_manipFlag) {
        _mouseReleaseEventManip(e);
    } else if (_navigateFlag) {
        _mouseReleaseEventNavigate(e);
    }

    _buttonNum = 0;
}

void VizWin::_mouseMoveEventManip(QMouseEvent *e)
{
    if (!_manipFlag) return;

    std::vector<double> screenCoords = _getScreenCoords(e);

    screenCoords[0] *= this->devicePixelRatioF();
    screenCoords[1] *= this->devicePixelRatioF();

    (void)_manip->MouseEvent(_buttonNum, screenCoords, _strHandleMid);
    _controlExec->GetParamsMgr()->IntermediateChange();
}

void VizWin::_mouseMoveEventNavigate(QMouseEvent *e)
{
    if (!_navigateFlag) return;

    // if (_getCurrentMouseMode() == MouseModeParams::GetGeoRefModeName() && _buttonNum == 1)
    // return;

    // _buttonNum is ignored in MouseOnTrackball here
    _trackBall->MouseOnTrackball(1, _buttonNum, e->x(), e->y(), width(), height());

    _trackBall->TrackballSetMatrix();

    const double *m = _trackBall->GetModelViewMatrix();

    ParamsMgr *paramsMgr = _controlExec->GetParamsMgr();

    // Set modelview matrix in ViewpointParams
    //
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);
    if (!_navigationPendingChanges)
        paramsMgr->BeginSaveStateGroup("Navigate");
    _navigationPendingChanges = true;

    vParams->SetModelViewMatrix(m);
    paramsMgr->IntermediateChange();
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
    ParamsMgr *     paramsMgr = _controlExec->GetParamsMgr();
    GUIStateParams *guiP = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());

    string activeTab = guiP->ActiveTab();
    if (activeTab == RenderEventRouterGUI::GeometryTabName || activeTab == FlowEventRouter::SeedingTabName || activeTab == FlowEventRouter::IntegrationTabName)
        return MouseModeParams::GetRegionModeName();
    else
        return MouseModeParams::GetNavigateModeName();
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

    if (_manipFlowSeedFlag) {
        FlowParams *fp = dynamic_cast<FlowParams *>(rParams);
        if (fp) {
            // Sam's box format: xmin, xmax, ymin, ymax, zmin, zmax
            int dims = fp->GetRenderDim();

            if (dims == 3) {    // 3D flow renderer
                vector<float> b(6);
                b[0] = llc[0];
                b[2] = llc[1];
                b[4] = llc[2];
                b[1] = urc[0];
                b[3] = urc[1];
                b[5] = urc[2];
                fp->SetRake(b);
            } else if (dims == 2) {    // 2D flow renderer
                vector<float> b(4);
                b[0] = llc[0];
                b[2] = llc[1];
                b[1] = urc[0];
                b[3] = urc[1];
                fp->SetRake(b);
            }
        }
    } else if (_manipFlowIntegrationFlag) {
        FlowParams *fp = dynamic_cast<FlowParams *>(rParams);
        VAssert(fp);
        fp->GetIntegrationBox()->SetExtents(llc, urc);
    } else {
        box->SetExtents(llc, urc);
    }
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

    if (_manipFlag) {
        _mouseMoveEventManip(e);
    } else if (_navigateFlag) {
        _mouseMoveEventNavigate(e);
    }
    return;
}

void VizWin::setFocus() { QWidget::setFocus(); }

void VizWin::Render(bool fast)
{
    // Failsafe to prevent VizWin::Render from being called recursively.
    if (_insideRender) return;
    _insideRender = true;
    _renderHelper(fast);
    _insideRender = false;

    HideSTDERR();
    safeSwapBuffers();
    RestoreSTDERR();
}

void VizWin::_renderHelper(bool fast)
{
    // Need to call since we're not overriding QGLWidget::paintGL()
    //
    makeCurrent();

    if (!_openGLInitFlag || !FrameBufferReady()) { return; }

    glClearColor(0.3, 0.3, 0.3, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(_winName);

    bool enabled = _controlExec->GetSaveStateEnabled();
    _controlExec->SetSaveStateEnabled(false);
    vParams->SetWindowSize(width(), height());
    _controlExec->SetSaveStateEnabled(enabled);

    DataStatus *dataStatus = _controlExec->GetDataStatus();
    if (!dataStatus->GetDataMgrNames().size()) return;

    _preRender();

    int rc = _controlExec->Paint(_winName, fast);
    if (rc < 0) { MSG_ERR("Paint failed"); }

    if (_getCurrentMouseMode() == MouseModeParams::GetRegionModeName()) {
        updateManip();
        if (_getRenderParams() && _getRenderParams()->GetOrientable()) {
            _updateOriginGlyph();
            _drawContourSliceQuad();
        }
    } else if (vParams->GetProjectionType() == ViewpointParams::MapOrthographic) {
#ifndef WIN32
        _glManager->PixelCoordinateSystemPush();
        _glManager->matrixManager->Translate(10, 10, 0);
        glDisable(GL_DEPTH_TEST);
        _glManager->fontManager->GetFont("arimo", 22)->DrawText("Geo Referenced Mode");
        _glManager->PixelCoordinateSystemPop();
#endif
    }

    rc = CheckGLErrorMsg("VizWindowPaintGL");
    if (rc < 0) { MSG_ERR("OpenGL error"); }

    _postRender();
}

void VizWin::_preRender()
{
    _glManager->matrixManager->MatrixModeProjection();
    _glManager->matrixManager->PushMatrix();
    _setUpProjMatrix();

    _glManager->matrixManager->MatrixModeModelView();
    _glManager->matrixManager->PushMatrix();
    _setUpModelViewMatrix();
}

void VizWin::_postRender()
{
    _glManager->matrixManager->MatrixModeProjection();
    _glManager->matrixManager->PopMatrix();
    _glManager->matrixManager->MatrixModeModelView();
    _glManager->matrixManager->PopMatrix();
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

void VizWin::_getUnionOfFieldVarExtents(RenderParams *rParams, DataMgr *dataMgr, int timeStep, int refLevel, int lod, CoordType &minExts, CoordType &maxExts)
{
    vector<string> fieldVars = rParams->GetFieldVariableNames();
    vector<int>    axes;
    bool           ok = DataMgrUtils::GetExtents(dataMgr, timeStep, fieldVars, refLevel, lod, minExts, maxExts, axes);
    VAssert(ok);
}

void VizWin::_getActiveExtents(CoordType &minExts, CoordType &maxExts)
{
    VAPoR::RenderParams *rParams = _getRenderParams();
    if (rParams == NULL) return;

    int            refLevel = rParams->GetRefinementLevel();
    int            lod = rParams->GetCompressionLevel();
    string         varName = rParams->GetVariableName();
    vector<string> fieldVars = rParams->GetFieldVariableNames();

    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    AnimationParams *aParams = (AnimationParams *)paramsMgr->GetParams(AnimationParams::GetClassType());
    int              timeStep = aParams->GetCurrentTimestep();

    DataStatus *dataStatus = _controlExec->GetDataStatus();
    string      dataMgrName = _getCurrentDataMgrName();
    DataMgr *   dataMgr = dataStatus->GetDataMgr(dataMgrName);

    if (fieldVars[0] == "" && fieldVars[1] == "" && fieldVars[2] == "") {
        dataMgr->GetVariableExtents(timeStep, varName, refLevel, lod, minExts, maxExts);
    } else {
        _getUnionOfFieldVarExtents(rParams, dataMgr, timeStep, refLevel, lod, minExts, maxExts);
    }
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
    CoordType minExts = {0.0, 0.0, 0.0};
    CoordType maxExts = {0.0, 0.0, 0.0};

    _getActiveExtents(minExts, maxExts);

    CoordType            llc, urc;
    string               classType;
    VAPoR::RenderParams *rParams = _getRenderParams(classType);
    if (initialize || rParams == NULL) {
        llc = minExts;
        urc = maxExts;
    } else {
        _manipFlowSeedFlag = false;
        _manipFlowIntegrationFlag = false;
        GUIStateParams *gp = (GUIStateParams *)_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());

        if (rParams->GetName() == FlowParams::GetClassType()) {
            if (gp->ActiveTab() == FlowEventRouter::SeedingTabName) _manipFlowSeedFlag = true;
            if (gp->ActiveTab() == FlowEventRouter::IntegrationTabName) _manipFlowIntegrationFlag = true;
        }
        if (_manipFlowSeedFlag) {
            FlowParams *fp = dynamic_cast<FlowParams *>(rParams);
            VAssert(fp);

            // Sam's box format: xmin, xmax, ymin, ymax, zmin, zmax
            // Why doesn't Sam's code use the standard box class?
            vector<float> b = fp->GetRake();

            // If the flow renderer is 2d, add z extents or the Manip will break
            if (b.size() == 4) {
                DataStatus *dataStatus = _controlExec->GetDataStatus();
                string      dataMgrName = _getCurrentDataMgrName();
                DataMgr *   dataMgr = dataStatus->GetDataMgr(dataMgrName);

                ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
                AnimationParams *p = (AnimationParams *)paramsMgr->GetParams(AnimationParams::GetClassType());
                size_t           ts = p->GetCurrentTimestep();
                string           inst, winName, dataSetName;

                int    refLevel = rParams->GetRefinementLevel();
                int    lod = rParams->GetCompressionLevel();
                double defaultZ = VAPoR::DataMgrUtils::Get2DRendererDefaultZ(dataMgr, ts, refLevel, lod);
                b.push_back(defaultZ);
                b.push_back(defaultZ);
            }

            llc[0] = b[0];
            urc[0] = b[1];
            llc[1] = b[2];
            urc[1] = b[3];
            llc[2] = b[4];
            urc[2] = b[5];
        } else if (_manipFlowIntegrationFlag) {
            FlowParams *fp = dynamic_cast<FlowParams *>(rParams);
            VAssert(fp);
            fp->GetIntegrationBox()->GetExtents(llc, urc);
        } else {
            VAPoR::Box *box = rParams->GetBox();
            box->GetExtents(llc, urc);
        }
    }

    bool constrain = true;
    if (classType == ImageParams::GetClassType()) constrain = false;

    VAPoR::Transform *dmTransform = _getDataMgrTransform();
    VAPoR::Transform *rpTransform = NULL;
    if (rParams != NULL) rpTransform = rParams->GetTransform();

    vector<double> llcVec = {llc[0], llc[1], llc[2]};
    vector<double> urcVec = {urc[0], urc[1], urc[2]};
    vector<double> minExtsVec = {minExts[0], minExts[1], minExts[2]};
    vector<double> maxExtsVec = {maxExts[0], maxExts[1], maxExts[2]};

    _manip->Update(llcVec, urcVec, minExtsVec, maxExtsVec, rpTransform, dmTransform, constrain);

    if (!initialize) _manip->Render();

    GL_ERR_BREAK();
}

void VizWin::_updateOriginGlyph()
{
    _glManager->matrixManager->PushMatrix();
    Renderer::ApplyDatasetTransform(_glManager, _getDataMgrTransform());
    Renderer::ApplyTransform(_glManager, _getDataMgrTransform(), _getRenderParams()->GetTransform());

    VAPoR::RenderParams *rp = _getRenderParams();
    double               xOrigin = rp->GetValueDouble(RenderParams::XSlicePlaneOriginTag, 0.);
    double               yOrigin = rp->GetValueDouble(RenderParams::YSlicePlaneOriginTag, 0.);
    double               zOrigin = rp->GetValueDouble(RenderParams::ZSlicePlaneOriginTag, 0.);

    std::vector<double> scales = _getDataMgrTransform()->GetScales();
    std::vector<double> scales2 = rp->GetTransform()->GetScales();
    scales[0] *= scales2[0];
    scales[1] *= scales2[1];
    scales[2] *= scales2[2];

    int            refLevel = rp->GetRefinementLevel();
    int            lod = rp->GetCompressionLevel();
    string         varName = rp->GetVariableName();
    vector<string> fieldVars = rp->GetFieldVariableNames();

    int timeStep = rp->GetCurrentTimestep();

    DataStatus *dataStatus = _controlExec->GetDataStatus();
    string      dataMgrName = _getCurrentDataMgrName();
    DataMgr *   dataMgr = dataStatus->GetDataMgr(dataMgrName);

    CoordType min, max;
    dataMgr->GetVariableExtents(timeStep, varName, refLevel, lod, min, max);

    // Find the average magnitude of the X and Y axes.  3% of that magnitude will be the size of the
    // origin marker's crosshairs.
    double              p = .03 * ((max[0] - min[0]) + (max[1] - min[1])) / 2;
    std::vector<double> width = {p / scales[0], p / scales[1], p / scales[2]};

    int depthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);

    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    LegacyGL *lgl = _glManager->legacy;
    lgl->Color4f(1., 1., 0., 1.);

    lgl->Begin(GL_LINES);
    lgl->Vertex3f(xOrigin, yOrigin, zOrigin - width[2]);
    lgl->Vertex3f(xOrigin, yOrigin, zOrigin + width[2]);
    lgl->End();

    lgl->Begin(GL_LINES);
    lgl->Vertex3f(xOrigin - width[0], yOrigin, zOrigin);
    lgl->Vertex3f(xOrigin + width[0], yOrigin, zOrigin);
    lgl->End();

    lgl->Begin(GL_LINES);
    lgl->Vertex3f(xOrigin, yOrigin - width[1], zOrigin);
    lgl->Vertex3f(xOrigin, yOrigin + width[1], zOrigin);
    lgl->End();

    glDepthFunc(depthFunc);
    _glManager->matrixManager->PopMatrix();
}

void VizWin::_drawContourSliceQuad()
{
    _glManager->matrixManager->PushMatrix();
    Renderer::ApplyDatasetTransform(_glManager, _getDataMgrTransform());
    Renderer::ApplyTransform(_glManager, _getDataMgrTransform(), _getRenderParams()->GetTransform());

    auto quad = _getRenderParams()->GetSlicePlaneQuad();

    LegacyGL *lgl = _glManager->legacy;
    lgl->Color3f(0, 1, 0);
    lgl->Begin(GL_LINE_STRIP);
    for (auto v : quad) lgl->Vertex3dv(v.data());
    if (quad.size()) lgl->Vertex3dv(quad[0].data());
    lgl->End();
    _glManager->matrixManager->PopMatrix();
}
