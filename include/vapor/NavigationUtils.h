#pragma once

#include <vapor/ControlExecutive.h>

using VAPoR::ControlExec;
class GUIStateParams;
class AnimationParams;
namespace VAPoR {
class ViewpointParams;
}

//! \class NavigationUtils
//! \brief This class is just migrated legacy code to de-spaghetti other legacy code.
//! (It is not written by me)

class RENDER_API NavigationUtils : public Wasp::MyBase {
public:
    static void SetHomeViewpoint(ControlExec *ce);
    static void UseHomeViewpoint(ControlExec *ce);
    static void ViewAll(ControlExec *ce);
    //! Align the view direction to one of the axes.
    //! axis is 2,3,4 for +X,Y,Z,  and 5,6,7 for -X,-Y,-Z
    static void AlignView(ControlExec *ce, int axis);

    static void SetAllCameras(ControlExec *ce, const double position[3], const double direction[3], const double up[3], const double origin[3]);
    static void SetAllCameras(ControlExec *ce, const double position[3], const double direction[3], const double up[3]);
    static void SetAllCameras(ControlExec *ce, const vector<double> &position, const vector<double> &direction, const vector<double> &up, const vector<double> &origin);
    static void SetAllCameras(ControlExec *ce, const vector<double> &position, const vector<double> &direction, const vector<double> &up);
    static void SetAllCameras(ControlExec *ce, const double matrix[16], const double origin[3]);
    static void SetAllCameras(ControlExec *ce, const vector<double> &matrix, const vector<double> &origin);

    static void LookAt(ControlExec *ce, const vector<double> &position, const vector<double> &target, const vector<double> &up);
    
    static void SetTimestep(ControlExec *ce, size_t ts);
    
    static void GetCameraProperties(ControlExec *ce, vector<double> *position, vector<double> *direction, vector<double> *up, vector<double> *target);
    static vector<double> GetCameraPosition(ControlExec *ce);
    static vector<double> GetCameraDirection(ControlExec *ce);
    static vector<double> GetCameraUp(ControlExec *ce);
    static vector<double> GetCameraTarget(ControlExec *ce);
    
    static void SetCameraPosition(ControlExec *ce, const vector<double> &v);
    static void SetCameraDirection(ControlExec *ce, const vector<double> &v);
    static void SetCameraUp(ControlExec *ce, const vector<double> &v);
    static void SetCameraTarget(ControlExec *ce, const vector<double> &v);

    static long                    GetCurrentTimeStep(ControlExec *ce);
    static VAPoR::ViewpointParams *GetActiveViewpointParams(ControlExec *ce);
    static GUIStateParams *        GetGUIStateParams(ControlExec *ce);
    static AnimationParams *       GetAnimationParams(ControlExec *ce);
};
