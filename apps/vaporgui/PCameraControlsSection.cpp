#include "PCameraControlsSection.h"
#include <vapor/ViewpointParams.h>
#include "ErrorReporter.h"
#include "VLineItem.h"
#include "V3DInput.h"
#include "VGroup.h"


// =====================================
//           PTrackballWidget
// =====================================


using namespace VAPoR;

PTrackballWidget::PTrackballWidget(ControlExec *ce) : PWidget("", _group = new VGroup), _ce(ce)
{
    _group->Add(new VLineItem("Direction", _direction = new V3DInput));
    _group->Add(new VLineItem("Up Vector", _up = new V3DInput));
    _group->Add(new VLineItem("Position  ", _position = new V3DInput));
    _group->Add(new VLineItem("Origin     ", _origin = new V3DInput));

    connect(_direction, &V3DInput::ValueChanged, this, &PTrackballWidget::cameraChanged);
    connect(_up, &V3DInput::ValueChanged, this, &PTrackballWidget::cameraChanged);
    connect(_position, &V3DInput::ValueChanged, this, &PTrackballWidget::cameraChanged);
    connect(_origin, &V3DInput::ValueChanged, this, &PTrackballWidget::cameraChanged);
}


void PTrackballWidget::updateGUI() const
{
    ViewpointParams *vp = NavigationUtils::GetActiveViewpointParams(_ce);
    double           m[16], position[3], up[3], direction[3], origin[3];
    vp->GetModelViewMatrix(m);
    bool status = vp->ReconstructCamera(m, position, up, direction);
    vp->GetRotationCenter(origin);
    if (!status) {
        MSG_ERR("Failed to get camera parameters");
        return;
    }

    _position->SetValue(position);
    _up->SetValue(up);
    _direction->SetValue(direction);
    _origin->SetValue(origin);
}


void PTrackballWidget::cameraChanged() { NavigationUtils::SetAllCameras(_ce, _position->GetValue(), _direction->GetValue(), _up->GetValue(), _origin->GetValue()); }



// =====================================
//        PCameraProjectionWidget
// =====================================



#include "VComboBox.h"
#include "GUIStateParams.h"


const string PCameraProjectionWidget::Perspective = "Perspective";
const string PCameraProjectionWidget::MapOrthographic = "Map Orthographic";


PCameraProjectionWidget::PCameraProjectionWidget(ControlExec *ce) : PWidget("", new VLineItem("Projection Mode", _dropdown = new VComboBox({Perspective, MapOrthographic}))), _ce(ce)
{
    connect(_dropdown, &VComboBox::ValueChanged, this, &PCameraProjectionWidget::dropdownChanged);
}


void PCameraProjectionWidget::updateGUI() const
{
    ViewpointParams::ProjectionType t = NavigationUtils::GetActiveViewpointParams(_ce)->GetProjectionType();

    if (t == ViewpointParams::Perspective)
        _dropdown->SetValue(Perspective);
    else if (t == ViewpointParams::MapOrthographic)
        _dropdown->SetValue(MapOrthographic);
}


void PCameraProjectionWidget::dropdownChanged(string s)
{
    ViewpointParams::ProjectionType type;
    if (s == Perspective)
        type = ViewpointParams::Perspective;
    else if (s == MapOrthographic)
        type = ViewpointParams::MapOrthographic;
    else
        type = ViewpointParams::Perspective;

    ParamsMgr *pm = _ce->GetParamsMgr();
    auto       vizNames = pm->GetVisualizerNames();

    pm->BeginSaveStateGroup("Projection Mode Set");
    for (auto &viz : vizNames) {
        ViewpointParams *vp = pm->GetViewpointParams(viz);
        vp->SetProjectionType(type);
    }
    NavigationUtils::ViewAll(_ce);
    NavigationUtils::SetHomeViewpoint(_ce);
    pm->EndSaveStateGroup();
}
