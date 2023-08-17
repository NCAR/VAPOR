#include <vapor/NavigationUtils.h>
#include <vapor/DataStatus.h>
#include <vapor/ParamsMgr.h>
#define INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
#include <vapor/LegacyVectorMath.h>
#include <vapor/AnimationParams.h>
#include <vapor/GUIStateParams.h>
#include <vapor/AnimationParams.h>
#include <cassert>
#include <cfloat>
#include <glm/glm.hpp>

using namespace VAPoR;
using glm::vec3;

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

    CoordType minExts, maxExts;
    dataStatus->GetActiveExtents(paramsMgr, ts, minExts, maxExts);

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
    // This fixes a bug in this legacy function where the up dir is occationally wrong
    if (axis != 3 && axis != 1) AlignView(ce, 3);

    float axes[3][3] = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}};

    double dirvec[3] = {0.0, 0.0, 0.0};
    double upvec[3] = {0.0, 1.0, 0.0};
    (void)(upvec);
    ViewpointParams *vpParams = GetActiveViewpointParams(ce);
    assert(vpParams);
    if (!vpParams) return;

    double m[16], curPosVec[3], curViewDir[3], curUpVec[3], curCenter[3];
    vpParams->GetModelViewMatrix(m);
    bool status = vpParams->ReconstructCamera(m, curPosVec, curUpVec, curViewDir);
    vpParams->GetRotationCenter(curCenter);
    if (!status) return;
    
#define V3S(v) (string("[")+to_string(v[0])+","+to_string(v[1])+","+to_string(v[2])+"]").c_str()

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

    SetAllCameras(ce, curPosVec, dirvec, upvec, curCenter);
}


void NavigationUtils::SetAllCameras(ControlExec *ce, const double position[3], const double direction[3], const double up[3], const double origin[3])
{
    Trackball trackball;
    bool      ok = trackball.setFromFrame(position, direction, up, origin, true);
    if (!ok) {
        MyBase::SetErrMsg("Invalid camera settings");
        return;
    }

    trackball.TrackballSetMatrix();
    const double *m = trackball.GetModelViewMatrix();
    SetAllCameras(ce, m, origin);
}


void NavigationUtils::SetAllCameras(ControlExec *ce, const double position[3], const double direction[3], const double up[3])
{
    ViewpointParams *vpParams = GetActiveViewpointParams(ce);
    assert(vpParams);
    if (!vpParams) return;

    double curCenter[3];
    vpParams->GetRotationCenter(curCenter);
    
    SetAllCameras(ce, position, direction, up, curCenter);
}


void NavigationUtils::SetAllCameras(ControlExec *ce, const vector<double> &position, const vector<double> &direction, const vector<double> &up, const vector<double> &origin)
{
    SetAllCameras(ce, position.data(), direction.data(), up.data(), origin.data());
}


void NavigationUtils::SetAllCameras(ControlExec *ce, const vector<double> &position, const vector<double> &direction, const vector<double> &up)
{
    SetAllCameras(ce, position.data(), direction.data(), up.data());
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

void NavigationUtils::SetAllCameras(ControlExec *ce, const Trackball &trackball)
{
    double center[3];
    trackball.GetCenter(center);
    const double *m = trackball.GetModelViewMatrix();
    NavigationUtils::SetAllCameras(ce, m, center);
}

void NavigationUtils::ConfigureTrackball(ControlExec *ce, Trackball &trackball)
{
    vector<double> pos, dir, up, tgt;
    GetCameraProperties(ce, &pos, &dir, &up, &tgt);
    trackball.setFromFrame(pos, dir, up, tgt, true);
    
    if (ce->GetDataNames().size() == 0) return;

    DataStatus *dataStatus = ce->GetDataStatus();
    ParamsMgr  *paramsMgr  = ce->GetParamsMgr();
    size_t ts = GetAnimationParams(ce)->GetCurrentTimestep();

    VAPoR::CoordType minExts, maxExts;
    dataStatus->GetActiveExtents(paramsMgr, ts, minExts, maxExts);

    double scale[3];
    scale[0] = scale[1] = scale[2] = max(maxExts[0] - minExts[0], (maxExts[1] - minExts[1]));
    trackball.SetScale(scale);
}

void NavigationUtils::LookAt(ControlExec *ce, const vector<double> &position, const vector<double> &target, const vector<double> &up)
{
    vec3 pos(position[0], position[1], position[2]);
    vec3 tar(target[0], target[1], target[2]);
    vec3 dir = glm::normalize(tar - pos);
    vector<double> direction = {dir.x, dir.y, dir.z};
    SetAllCameras(ce, position, direction, up, target);
}


void NavigationUtils::SetTimestep(ControlExec *ce, size_t ts)
{
    AnimationParams *aParams = GetAnimationParams(ce);
    size_t ots = aParams->GetCurrentTimestep();

    propagateTimestep(ce, ts);

    handleMovingDomainAdjustments(ce, ots, ts);
}

void NavigationUtils::propagateTimestep(ControlExec *ce, size_t ts)
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
                    if (abs(range[1] - range[0]) > FLT_EPSILON * max(abs(range[0]), abs(range[1]))) continue;

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


static vector<double> DoubleArr3ToVector(double a[3])
{
    vector<double> v(3);
    v[0] = a[0];
    v[1] = a[1];
    v[2] = a[2];
    return v;
}


void NavigationUtils::GetCameraProperties(ControlExec *ce, vector<double> *position, vector<double> *direction, vector<double> *up, vector<double> *target)
{
    position->resize(3, 0);
    direction->resize(3, 0);
    up->resize(3, 0);
    target->resize(3, 0);
    
    ViewpointParams *vpParams = GetActiveViewpointParams(ce);
    assert(vpParams);
    if (!vpParams) return;
    
    double m[16], posD[3] = {0}, dirD[3] = {0}, upD[3] = {0}, targetD[3] = {0};
    vpParams->GetModelViewMatrix(m);
    vpParams->ReconstructCamera(m, posD, upD, dirD);
    vpParams->GetRotationCenter(targetD);
    
    *position = DoubleArr3ToVector(posD);
    *direction = DoubleArr3ToVector(dirD);
    *up = DoubleArr3ToVector(upD);
    *target = DoubleArr3ToVector(targetD);
}


vector<double> NavigationUtils::GetCameraPosition(ControlExec *ce)
{
    vector<double> pos, dir, up, tgt;
    GetCameraProperties(ce, &pos, &dir, &up, &tgt);
    return pos;
}


vector<double> NavigationUtils::GetCameraDirection(ControlExec *ce)
{
    vector<double> pos, dir, up, tgt;
    GetCameraProperties(ce, &pos, &dir, &up, &tgt);
    return dir;
}


vector<double> NavigationUtils::GetCameraUp(ControlExec *ce)
{
    vector<double> pos, dir, up, tgt;
    GetCameraProperties(ce, &pos, &dir, &up, &tgt);
    return up;
}


vector<double> NavigationUtils::GetCameraTarget(ControlExec *ce)
{
    vector<double> pos, dir, up, tgt;
    GetCameraProperties(ce, &pos, &dir, &up, &tgt);
    return tgt;
}


void NavigationUtils::SetCameraPosition(ControlExec *ce, const vector<double> &v)
{
    vector<double> pos, dir, up, tgt;
    GetCameraProperties(ce, &pos, &dir, &up, &tgt);
    SetAllCameras(ce, v, dir, up, tgt);
}


void NavigationUtils::SetCameraDirection(ControlExec *ce, const vector<double> &v)
{
    vector<double> pos, dir, up, tgt;
    GetCameraProperties(ce, &pos, &dir, &up, &tgt);
    SetAllCameras(ce, pos, v, up, tgt);
}


void NavigationUtils::SetCameraUp(ControlExec *ce, const vector<double> &v)
{
    vector<double> pos, dir, up, tgt;
    GetCameraProperties(ce, &pos, &dir, &up, &tgt);
    SetAllCameras(ce, pos, dir, v, tgt);
}


void NavigationUtils::SetCameraTarget(ControlExec *ce, const vector<double> &v)
{
    vector<double> pos, dir, up, tgt;
    GetCameraProperties(ce, &pos, &dir, &up, &tgt);
    SetAllCameras(ce, pos, dir, up, v);
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


static vec3 CoordTypeToVec3(const CoordType &c) { return vec3(c[0], c[1], c[2]); }
static CoordType Vec3ToCoordType(const vec3 &c) { return CoordType {c[0], c[1], c[2]}; }


void NavigationUtils::handleMovingDomainAdjustments(ControlExec *ce, size_t ts_from, size_t ts_to)
{
    auto paramsMgr = ce->GetParamsMgr();
    auto guiParams = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    bool trackCamera = guiParams->GetValueLong(GUIStateParams::MovingDomainTrackCameraTag, false);
    bool trackDomain = guiParams->GetValueLong(GUIStateParams::MovingDomainTrackRenderRegionsTag, false);

    if (trackCamera) movingDomainTrackCamera(ce, ts_from, ts_to);
    if (trackDomain) movingDomainTrackRenderRegions(ce, ts_from, ts_to);
}


glm::vec3 NavigationUtils::getDomainMovementBetweenTimesteps(ControlExec *ce, string dataset, size_t from, size_t to)
{
    auto paramsMgr = ce->GetParamsMgr();
    auto dataStatus = ce->GetDataStatus();
    auto guiParams = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    string viz = guiParams->GetActiveVizName();

    CoordType minExts_, maxExts_;
    dataStatus->GetActiveExtents(paramsMgr, viz, dataset, from, minExts_, maxExts_);
    vec3 oldMinExts = CoordTypeToVec3(minExts_);
    vec3 oldMaxExts = CoordTypeToVec3(maxExts_);
    vec3 oldDomainCenter = (oldMinExts+oldMaxExts)/2.f;
    dataStatus->GetActiveExtents(paramsMgr, viz, dataset, to, minExts_, maxExts_);
    vec3 newMinExts = CoordTypeToVec3(minExts_);
    vec3 newMaxExts = CoordTypeToVec3(maxExts_);
    vec3 newDomainCenter = (newMinExts+newMaxExts)/2.f;
    vec3 domainMove = newDomainCenter - oldDomainCenter;

    return domainMove;
}


void NavigationUtils::movingDomainTrackCamera(ControlExec *ce, size_t from, size_t to)
{
    auto paramsMgr = ce->GetParamsMgr();
    auto dataStatus = ce->GetDataStatus();
    GUIStateParams *guiParams = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    string viz = guiParams->GetActiveVizName();
    auto viewpointParams = paramsMgr->GetViewpointParams(guiParams->GetActiveVizName());
    auto datasets = guiParams->GetOpenDataSetNames();

    vec3 domainMoveSum = vec3(0);
    int nDomainsMoved = 0;

    for (const auto &dataset : datasets) {
        if (!dataStatus->GetDataMgr(dataset)->HasMovingDomain()) continue;
        auto domainMove = getDomainMovementBetweenTimesteps(ce, dataset, from, to);
        domainMoveSum += domainMove;
        nDomainsMoved++;
    }

    if (nDomainsMoved) {
        vec3 domainMove = domainMoveSum / (float)nDomainsMoved;
        double camMat[16], camPos[3], camDir[3], camUp[3];
        viewpointParams->GetModelViewMatrix(camMat);
        viewpointParams->ReconstructCamera(camMat, camPos, camUp, camDir);
        for (int i = 0; i < 3; i++) camPos[i] += domainMove[i];
        NavigationUtils::SetAllCameras(ce, camPos, camDir, camUp);
    }
}


void NavigationUtils::movingDomainTrackRenderRegions(ControlExec *ce, size_t from, size_t to)
{
    auto paramsMgr = ce->GetParamsMgr();
    auto dataStatus = ce->GetDataStatus();
    GUIStateParams *guiParams = (GUIStateParams *)paramsMgr->GetParams(GUIStateParams::GetClassType());
    string viz = guiParams->GetActiveVizName();
    auto datasets = guiParams->GetOpenDataSetNames();

    for (const auto &dataset : datasets) {
        if (!dataStatus->GetDataMgr(dataset)->HasMovingDomain()) continue;
        auto domainMove = getDomainMovementBetweenTimesteps(ce, dataset, from, to);

        vector<string> renderers;
        paramsMgr->GetRenderParamNames(viz, dataset, renderers);
        for (const auto &renderer : renderers) {
            string _1, _2, className;
            paramsMgr->RenderParamsLookup(renderer, _1, _2, className);
            RenderParams *rp = paramsMgr->GetRenderParams(viz, dataset, className, renderer);

            CoordType minExts_, maxExts_;
            rp->GetBox()->GetExtents(minExts_, maxExts_);
            rp->GetBox()->SetExtents(Vec3ToCoordType(domainMove+CoordTypeToVec3(minExts_)), Vec3ToCoordType(domainMove+CoordTypeToVec3(maxExts_)));
        }
    }
}


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
