#include <QFileDialog>

#include <vapor/ViewpointParams.h>
#include <vapor/FileUtils.h>

#include "ErrorReporter.h"
#include "PCameraControlsSection.h"
#include "PFileButton.h"
#include "PLineItem.h"
#include "VLineItem.h"
#include "V3DInput.h"
#include "VGroup.h"
#include "VPushButton.h"
#include "PButton.h"

// =====================================
//           PTrackballWidget
// =====================================


using namespace VAPoR;
using namespace Wasp;

PCameraFileGroup::PCameraFileGroup(ControlExec *ce) : PGroup(), _ce(ce) {
    Add((new PFileWriter("Save camera settings to file", [this](std::string path) { NavigationUtils::GetActiveViewpointParams(this->_ce)->SaveCameraToFile(path); }))->SetFileTypeFilter("Vapor Camera File (*.vc3)"));
    Add((new PFileReader("Load camera settings from file", [this](std::string path) { PCameraFileGroup::SetAllCameras(path); }))->SetFileTypeFilter("Vapor Camera File (*.vc3)"));
}

void PCameraFileGroup::updateGUI() const {
    auto params = NavigationUtils::GetActiveViewpointParams(_ce);
    for (PWidget *child : _children) child->Update(params);
}

void PCameraFileGroup::SetAllCameras(std::string &fileName) {
    XmlParser xmlparser;
    XmlNode *node = new XmlNode();

    int rc = xmlparser.LoadFromFile(node, fileName);
    if (rc < 0) {
        MSG_ERR("Failed to read file " + fileName);
        return;
    }

    // Ensure ModelViewMatrix and RotationCenter tags exist
    const std::string mvmTag = "ModelViewMatrix";
    const std::string rcTag  = "RotationCenter";
    if (!node->HasElementDouble(mvmTag)) {
        MSG_ERR("Invalid camera file" + fileName + ".  Missing XML node: " + mvmTag);
        return;
    }
    if (!node->HasElementDouble(rcTag)) {
        MSG_ERR("Invalid camera file" + fileName + ".  Missing XML node: " + rcTag);
        return;
    }

    // Check for valid matrix and origin values
    std::vector<double> matrix = node->GetElementDouble(mvmTag);
    std::vector<double> origin = node->GetElementDouble(rcTag);
    if (matrix.size() != 16) {
        MSG_ERR("Invalid camera file" + fileName + ".  Tag " + mvmTag + " must have 16 elements.");
        return;
    }
    if (origin.size() != 3) {
        MSG_ERR("Invalid camera file" + fileName + ".  Tag " + rcTag + " must have 3 elements.");
        return;
    }

    NavigationUtils::SetAllCameras(_ce, matrix, origin);
}

PTrackballWidget::PTrackballWidget(ControlExec *ce) : PWidget("", _group = new VGroup()), _ce(ce)
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
#include <vapor/GUIStateParams.h>


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
