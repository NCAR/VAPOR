#include "RenderManager.h"

#include <vapor/GUIStateParams.h>
#include <vapor/AnimationParams.h>
#include <vapor/TrackBall.h>
#include <vapor/ControlExecutive.h>
#include <vapor/ParamsMgr.h>
#include <vapor/GLManager.h>
#include <vapor/Framebuffer.h>

#include <osgl/GLInclude.h>

#define INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
#include <vapor/LegacyVectorMath.h>


using namespace VAPoR;
using namespace OSGL;

RenderManager::RenderManager(ControlExec *ce) : _controlExec(ce) {}

RenderManager::~RenderManager()
{
    if (_glManager) delete _glManager;
}

void RenderManager::getNearFarDist(const double posVec[3], const double dirVec[3], double &boxNear, double &boxFar)
{
    String _winName = GetWinName();

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

    AnimationParams *ap = ((AnimationParams *)paramsMgr->GetParams(AnimationParams::GetClassType()));
    size_t           ts = ap->GetCurrentTimestep();

    CoordType minExts, maxExts;
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

void RenderManager::setUpProjMatrix()
{
    assert(not _controlExec->GetVisualizerNames().empty());
    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(GetWinName());
    MatrixManager *  mm = _glManager->matrixManager;

    double m[16];
    vParams->GetModelViewMatrix(m);

    double posvec[3], upvec[3], dirvec[3];
    bool   status = vParams->ReconstructCamera(m, posvec, upvec, dirvec);
    if (!status) {
        LogFatal("Failed to get camera parameters");
        return;
    }

    double nearDist, farDist;
    getNearFarDist(posvec, dirvec, nearDist, farDist);
    nearDist *= 0.25;
    farDist *= 4.0;

    size_t width, height;
    vParams->GetWindowSize(width, height);
    //    width *= QApplication::desktop()->devicePixelRatio();
    //    height *= QApplication::desktop()->devicePixelRatio();
    int wWidth = width;
    int wHeight = height;

    if (vParams->GetValueLong(ViewpointParams::UseCustomFramebufferTag, 0)) {
//        printf("USE CUSTOM FRAMEBUFFER\n");
        width = vParams->GetValueLong(ViewpointParams::CustomFramebufferWidthTag, 0);
        height = vParams->GetValueLong(ViewpointParams::CustomFramebufferHeightTag, 0);
        if (width == 0) width = 1;
        if (height == 0) height = 1;

        int maxSize;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
        if (width > maxSize) {
            width = maxSize;
            vParams->SetValueLong(ViewpointParams::CustomFramebufferWidthTag, ViewpointParams::CustomFramebufferWidthTag, width);
            LogFatal("Selected width is larger than your OpenGL implementation supports");
        }
        if (height > maxSize) {
            height = maxSize;
            vParams->SetValueLong(ViewpointParams::CustomFramebufferHeightTag, ViewpointParams::CustomFramebufferHeightTag, height);
            LogFatal("Selected height is larger than your OpenGL implementation supports");
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

    float w = (float)width / (float)height;

    if (vParams->GetProjectionType() == ViewpointParams::MapOrthographic) {
        Trackball trackball;
        double    center[3];
        vParams->GetRotationCenter(center);
        trackball.setFromFrame(posvec, dirvec, upvec, center, true);
        float s = trackball.GetOrthoSize();
        mm->Ortho(-s * w, s * w, -s, s, nearDist, farDist);
        vParams->SetOrthoProjectionSize(trackball.GetOrthoSize());
    } else {
        double fov = vParams->GetFOV();
        mm->Perspective(glm::radians(fov), w, nearDist, farDist);
    }

    double pMatrix[16];
    mm->GetDoublev(MatrixManager::Mode::Projection, pMatrix);

    bool enabled = _controlExec->GetSaveStateEnabled();
    _controlExec->SetSaveStateEnabled(false);

    vParams->SetProjectionMatrix(pMatrix);

    _controlExec->SetSaveStateEnabled(enabled);

    mm->MatrixModeModelView();
}

void RenderManager::setUpModelViewMatrix()
{
    ParamsMgr *      paramsMgr = _controlExec->GetParamsMgr();
    ViewpointParams *vParams = paramsMgr->GetViewpointParams(GetWinName());

    double m[16];
    vParams->GetModelViewMatrix(m);
    _glManager->matrixManager->LoadMatrixd(m);
}



int RenderManager::Render(String imagePath, bool fast)
{
    //    GL_ERR_BREAK();
    if (!_glManager) {
        _glManager = new GLManager;
        _controlExec->InitializeViz(GetWinName(), _glManager);
    }
    
    auto res = GetResolution();
    int width = res[0];
    int height = res[1];

    Framebuffer defaultFB;
    defaultFB.Generate();
    defaultFB.SetSize(width, height);
    defaultFB.MakeRenderTarget();

    getViewpointParams()->SetWindowSize(width, height);

    _glManager->matrixManager->MatrixModeProjection();
    _glManager->matrixManager->PushMatrix();
    setUpProjMatrix();

    _glManager->matrixManager->MatrixModeModelView();
    _glManager->matrixManager->PushMatrix();
    setUpModelViewMatrix();

    int rc = _controlExec->EnableImageCapture(imagePath, GetWinName(), fast);
    if (rc < 0) { LogWarning("Paint Failed"); }

    _glManager->matrixManager->MatrixModeProjection();
    _glManager->matrixManager->PopMatrix();
    _glManager->matrixManager->MatrixModeModelView();
    _glManager->matrixManager->PopMatrix();

    return rc;
}

void RenderManager::SetResolution(int width, int height)
{
    width = std::max(width, 1);
    height = std::max(height, 1);
    auto vp = getViewpointParams();
    vp->BeginGroup("res");
    vp->SetValueLong(ViewpointParams::UseCustomFramebufferTag, "", true);
    vp->SetValueLong(ViewpointParams::CustomFramebufferWidthTag, "", width);
    vp->SetValueLong(ViewpointParams::CustomFramebufferHeightTag, "", height);
    vp->EndGroup();
}

vector<int> RenderManager::GetResolution() const
{
    vector<int> res = {600, 480};
    
    auto vp = getViewpointParams();
    if (vp->GetValueLong(vp->UseCustomFramebufferTag, false)) {
        res[0] = getViewpointParams()->GetValueLong(ViewpointParams::CustomFramebufferWidthTag, 600);
        res[1] = getViewpointParams()->GetValueLong(ViewpointParams::CustomFramebufferHeightTag, 480);
    }
    
    return res;
}

String RenderManager::GetWinName() const
{
    assert(not _controlExec->GetVisualizerNames().empty());
    String winName = _controlExec->GetVisualizerNames()[0];
    return winName;
}

VAPoR::ViewpointParams *RenderManager::getViewpointParams() const { return _controlExec->GetParamsMgr()->GetViewpointParams(GetWinName()); }
