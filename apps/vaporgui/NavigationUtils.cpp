#include "NavigationUtils.h"
#include <vapor/DataStatus.h>
#include <vapor/ParamsMgr.h>
#define INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
#include <vapor/LegacyVectorMath.h>
#include "TrackBall.h"
#include "ErrorReporter.h"
#include "AnimationParams.h"
#include "GUIStateParams.h"
#include "AnimationParams.h"
#include <cassert>
#include <cfloat>

using namespace VAPoR;

void NavigationUtils::SetHomeViewpoint(ControlExec *ce)
{
    ParamsMgr *      paramsMgr = ce->GetParamsMgr();
    ViewpointParams *vpParams = GetActiveViewpointParams(ce);
    GUIStateParams * guiParams = GetGUIStateParams(ce);

    // Get the current model view matrix and it home
    //
    vector<double> m = vpParams->GetModelViewMatrix();
    vector<double> c = vpParams->GetRotationCenter();

    paramsMgr->BeginSaveStateGroup("Set home viewpoint");

    guiParams->SetValueDoubleVec("HomeModelViewMatrix", "Modelview matrix", m);
    guiParams->SetValueDoubleVec("HomeRotationCenter", "Camera rotation center", c);
    paramsMgr->EndSaveStateGroup();
}


void NavigationUtils::UseHomeViewpoint(ControlExec *ce)
{
    GUIStateParams *guiParams = GetGUIStateParams(ce);

    // Get the home matrix and make it the current model view matrix
    //
    vector<double> defaultV(16, 0.0);
    defaultV[0] = defaultV[5] = defaultV[10] = defaultV[15] = 1.0;

    vector<double> m = guiParams->GetValueDoubleVec("HomeModelViewMatrix", defaultV);

    vector<double> defaultC(3, 0.0);
    vector<double> c = guiParams->GetValueDoubleVec("HomeRotationCenter", defaultC);

    SetAllCameras(ce, m, c);
}


void NavigationUtils::ViewAll(ControlExec *ce)
{
    DataStatus *dataStatus = ce->GetDataStatus();
    ParamsMgr * paramsMgr = ce->GetParamsMgr();
    size_t      ts = GetCurrentTimeStep(ce);

    vector<double> minExts, maxExts;
    dataStatus->GetActiveExtents(paramsMgr, ts, minExts, maxExts);
    VAssert(minExts.size() == 3);
    VAssert(maxExts.size() == 3);

    double maxSide = max(maxExts[2] - minExts[2], max(maxExts[1] - minExts[1], maxExts[0] - minExts[0]));

    // calculate the camera position: center - 1.5*dirvec*maxSide;
    // Position the camera 1.5*maxSide units away from the center, aimed
    // at the center.
    //

    // Make sure the dirvec is normalized:
    double dirvec[] = {0.0, 0.0, -1.0};
    //    vnormal(dirvec);

    double upvec[] = {0.0, 1.0, 0.0};

    double posvec[3], center[3];
    for (int i = 0; i < 3; i++) {
        center[i] = 0.5f * (maxExts[i] + minExts[i]);
        posvec[i] = center[i] - 1.5 * maxSide * dirvec[i];
    }

    SetAllCameras(ce, posvec, dirvec, upvec, center);
}


void NavigationUtils::AlignView(ControlExec *ce, int axis)
{
    float axes[3][3] = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}};

    double dirvec[3] = {0.0, 0.0, 0.0};
    double upvec[3] = {0.0, 1.0, 0.0};
    (void)(upvec);
    ViewpointParams *vpParams = GetActiveViewpointParams(ce);
    if (!vpParams) return;

    double m[16], curPosVec[3], curViewDir[3], curUpVec[3], curCenter[3];
    vpParams->GetModelViewMatrix(m);
    bool status = vpParams->ReconstructCamera(m, curPosVec, curUpVec, curViewDir);
    vpParams->GetRotationCenter(curCenter);
    if (!status) return;

    if (axis == 1) {    // Special case to align to closest axis.
        // determine the closest view direction and up vector to the current viewpoint.
        // Check the dot product with all axes
        float maxVDot = -1.f;
        int   bestVDir = 0;
        float maxUDot = -1.f;
        int   bestUDir = 0;
        for (int i = 0; i < 3; i++) {
            double dotVProd = 0.;
            double dotUProd = 0.;
            for (int j = 0; j < 3; j++) {
                dotUProd += (axes[i][j] * curViewDir[j]);
                dotVProd += (axes[i][j] * curUpVec[j]);
            }
            if (abs(dotVProd) > maxVDot) {
                maxVDot = abs(dotVProd);
                bestVDir = i + 1;
                if (dotVProd < 0.f) bestVDir = -i - 1;
            }
            if (abs(dotUProd) > maxUDot) {
                maxUDot = abs(dotUProd);
                bestUDir = i + 1;
                if (dotUProd < 0.f) bestUDir = -i - 1;
            }
        }
        for (int i = 0; i < 3; i++) {
            if (bestUDir > 0)
                dirvec[i] = axes[bestUDir - 1][i];
            else
                dirvec[i] = -axes[-1 - bestUDir][i];

            if (bestVDir > 0)
                upvec[i] = axes[bestVDir - 1][i];
            else
                upvec[i] = -axes[-1 - bestVDir][i];
        }
    } else {
        // establish view direction, up vector:
        switch (axis) {
        case (2): dirvec[0] = 1.f; break;
        case (3):
            dirvec[1] = 1.f;
            upvec[1] = 0.f;
            upvec[0] = 1.f;
            break;
        case (4): dirvec[2] = 1.f; break;
        case (5): dirvec[0] = -1.f; break;
        case (6):
            dirvec[1] = -1.f;
            upvec[1] = 0.f;
            upvec[0] = 1.f;
            break;
        case (7): dirvec[2] = -1.f; break;
        default: return;
        }
    }

    vector<double> stretch = vpParams->GetStretchFactors();

    // Determine distance from center to camera, in stretched coordinates

    // determine the relative position in stretched coords:
    vsub(curPosVec, curCenter, curPosVec);
    float viewDist = vlength(curPosVec);
    // Position the camera the same distance from the center but down the -axis direction
    for (int i = 0; i < 3; i++) {
        dirvec[i] = dirvec[i] * viewDist;
        curPosVec[i] = (curCenter[i] - dirvec[i]);
    }

    SetAllCameras(ce, curPosVec, curViewDir, curUpVec, curCenter);
}


void NavigationUtils::SetAllCameras(ControlExec *ce, const double position[3], const double direction[3], const double up[3], const double origin[3])
{
    Trackball trackball;
    bool      ok = trackball.setFromFrame(position, direction, up, origin, true);
    if (!ok) {
        MSG_ERR("Invalid camera settings");
        return;
    }

    trackball.TrackballSetMatrix();
    const double *m = trackball.GetModelViewMatrix();
    SetAllCameras(ce, m, origin);
}


void NavigationUtils::SetAllCameras(ControlExec *ce, const vector<double> &position, const vector<double> &direction, const vector<double> &up, const vector<double> &origin)
{
    SetAllCameras(ce, position.data(), direction.data(), up.data(), origin.data());
}


void NavigationUtils::SetAllCameras(ControlExec *ce, const double matrix[16], const double origin[3])
{
    ParamsMgr *pm = ce->GetParamsMgr();
    auto       vizNames = pm->GetVisualizerNames();
    pm->BeginSaveStateGroup("Set Camera");
    for (auto &viz : vizNames) {
        ViewpointParams *vp = pm->GetViewpointParams(viz);
        vp->SetModelViewMatrix(matrix);
        vp->SetRotationCenter(origin);
    }
    pm->EndSaveStateGroup();
}


void NavigationUtils::SetAllCameras(ControlExec *ce, const vector<double> &m, const vector<double> &origin)
{
    assert(m.size() == 16);
    SetAllCameras(ce, m.data(), origin.data());
}


void NavigationUtils::SetTimestep(ControlExec *ce, size_t ts)
{
    DataStatus *dataStatus = ce->GetDataStatus();
    ParamsMgr * paramsMgr = ce->GetParamsMgr();

    paramsMgr->BeginSaveStateGroup("Set Timestep");

    // First set current *global* timestep in AnimationParams
    //
    AnimationParams *aParams = GetAnimationParams(ce);
    aParams->SetCurrentTimestep(ts);

    // Now set *local* time step for each RenderParams instance
    //
    vector<string> winNames = paramsMgr->GetVisualizerNames();
    for (int i = 0; i < winNames.size(); i++) {
        vector<string> dataSetNames = dataStatus->GetDataMgrNames();
        for (int j = 0; j < dataSetNames.size(); j++) {
            vector<RenderParams *> rParams;
            paramsMgr->GetRenderParams(winNames[i], dataSetNames[j], rParams);

            if (!rParams.size()) continue;

            size_t local_ts = dataStatus->MapGlobalToLocalTimeStep(dataSetNames[j], ts);

            for (int k = 0; k < rParams.size(); k++) {
                RenderParams *rp = rParams[k];
                rp->SetCurrentTimestep(local_ts);

                // If variable was not initialzed but now is, reset the TF mapping range
                std::string varNames[2] = {rp->GetVariableName(), rp->GetActualColorMapVariableName()};

                for (int l = 0; l < 2; l++) {
                    if (varNames[i].empty()) continue;

                    MapperFunction *tf = rp->GetMapperFunc(varNames[i]);
                    vector<double>  range = tf->getMinMaxMapValue();
                    if (abs(range[1] - range[0]) > FLT_EPSILON) continue;

                    DataMgr *dm = dataStatus->GetDataMgr(dataSetNames[j]);
                    if (!dm) continue;

                    if (!dm->VariableExists(local_ts, varNames[i], 0, 0)) continue;

                    if (dm->GetDataRange(local_ts, varNames[i], 0, 0, range) < 0) continue;

                    bool SSE = paramsMgr->GetSaveStateEnabled();
                    paramsMgr->SetSaveStateEnabled(false);
                    tf->setMinMaxMapValue(range[0], range[1]);
                    paramsMgr->SetSaveStateEnabled(SSE);
                }
            }
        }
    }

    paramsMgr->EndSaveStateGroup();
}


long NavigationUtils::GetCurrentTimeStep(VAPoR::ControlExec *ce)
{
    AnimationParams *aParams = (AnimationParams *)ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    VAssert(aParams);

    return aParams->GetCurrentTimestep();
}


VAPoR::ViewpointParams *NavigationUtils::GetActiveViewpointParams(ControlExec *ce)
{
    ParamsMgr *      pm = ce->GetParamsMgr();
    auto             activeViz = GetGUIStateParams(ce)->GetActiveVizName();
    ViewpointParams *vp = pm->GetViewpointParams(activeViz);
    return vp;
}


GUIStateParams *NavigationUtils::GetGUIStateParams(ControlExec *ce)
{
    ParamsMgr *pm = ce->GetParamsMgr();
    return ((GUIStateParams *)pm->GetParams(GUIStateParams::GetClassType()));
}


AnimationParams *NavigationUtils::GetAnimationParams(ControlExec *ce) { return ((AnimationParams *)ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType())); }



// This is some code that someone decided is worth keeping for future reference.
// It allegedly worked at some point.
// I don't want to have to write any camera code without refactoring the entire
// system so I am leaving it here in case the functionality is needed again.

#ifdef VAPOR3_0_0_ALPHA
void NavigationEventRouter::CenterSubRegion()
{
    cout << "NavigationEventRouter::CenterSubRegion not implemented" << endl;

    ViewpointParams *vpParams = _getActiveParams();
    if (!vpParams) return;

    // Find the largest of the dimensions of the current region, projected orthogonal to view
    // direction:
    // Make sure the dirvec is normalized:
    double dirvec[3];
    vpParams->GetCameraViewDir(dirvec);

    double upvec[3];
    vpParams->GetCameraUpVec(upvec);

    vnormal(dirvec);
    float          regionSideVector[3], compVec[3], projvec[3];
    float          maxProj = -1.f;
    vector<double> stretch = vpParams->GetStretchFactors();

    vector<double> minExts, maxExts;
    rParams->GetBox()->GetExtents(minExts, maxExts);
    for (int i = 0; i < 3; i++) {
        // Make a vector that points along side(i) of subregion,

        for (int j = 0; j < 3; j++) {
            regionSideVector[j] = 0.f;
            if (j == i) { regionSideVector[j] = maxExts[j] - minExts[j]; }
        }
        // Now find its component orthogonal to view direction:
        double dotprod = 0.;

        for (int j = 0; j < 3; j++) dotprod += (dirvec[j] * regionSideVector[j]);
        for (int j = 0; j < 3; j++) compVec[j] = dotprod * dirvec[j];
        // projvec is projection orthogonal to view dir:
        vsub(regionSideVector, compVec, projvec);
        float proj = vlength(projvec);
        if (proj > maxProj) maxProj = proj;
    }

    // calculate the camera position: center - 1.5*dirvec*maxSide;
    // Position the camera 1.5*maxSide units away from the center, aimed
    // at the center

    double         posvec[3], center[3];
    vector<double> rotCtr;
    for (int i = 0; i < 3; i++) {
        center[i] = 0.5 * (minExts[i] + maxExts[i]);
        posvec[i] = center[i] - (1.5 * maxProj * dirvec[i] / stretch[i]);
    }

    _setViewpointParams(center, posvec, dirvec, upvec);
}
#endif

#ifdef VAPOR3_0_0_ALPHA
// Reset the center of view.  Leave the camera where it is
void NavigationEventRouter::SetCenter(const double *coords)
{
    double           vdir[3];
    vector<double>   nvdir;
    ViewpointParams *vpParams = _getActiveParams();
    if (!vpParams) return;
    vector<double> stretch = _dataStatus->getStretchFactors();

    // Determine the new viewDir in stretched world coords

    vcopy(coords, vdir);
    // Stretch the new view center coords
    for (int i = 0; i < 3; i++) vdir[i] *= stretch[i];
    double campos[3];
    vpParams->getStretchedCamPosLocal(campos);
    vsub(vdir, campos, vdir);

    vnormal(vdir);
    vector<double> vvdir;
    #ifdef VAPOR3_0_0_ALPHA
    Command *cmd = Command::CaptureStart(vpParams, "re-center view");
    #endif
    for (int i = 0; i < 3; i++) vvdir.push_back(vdir[i]);
    vpParams->setViewDir(vvdir);
    vector<double> rotCtr;
    for (int i = 0; i < 3; i++) { rotCtr.push_back(coords[i]); }
    vpParams->setRotationCenterLocal(rotCtr);
    #ifdef VAPOR3_0_0_ALPHA
    Command::CaptureEnd(cmd, vpParams);
    #endif
    updateTab();
}
#endif
