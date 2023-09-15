#pragma once


// =====================================
//           PTrackballWidget
// =====================================


#include "PWidget.h"
#include "PGroup.h"
#include <vapor/NavigationUtils.h>

class VGroup;
class V3DInput;

class PTrackballWidget : public PWidget {
    ControlExec *_ce;
    VGroup *     _group;
    V3DInput *   _direction;
    V3DInput *   _up;
    V3DInput *   _position;
    V3DInput *   _origin;

public:
    PTrackballWidget(ControlExec *ce);

protected:
    void updateGUI() const override;
    void cameraChanged();
};


// =====================================
//        PCameraProjectionWidget
// =====================================


class VComboBox;

class PCameraProjectionWidget : public PWidget {
    ControlExec *       _ce;
    VComboBox *         _dropdown;
    static const string Perspective;
    static const string MapOrthographic;

public:
    PCameraProjectionWidget(ControlExec *ce);

protected:
    void updateGUI() const override;
    void dropdownChanged(string s);
};


class PCameraFileGroup : public PGroup {
    ControlExec* _ce;

public:
    PCameraFileGroup(ControlExec* ce);
    void SetAllCameras(std::string& path);

protected:
    void updateGUI() const override;
};

// =====================================
//        PCameraControlsSection
// =====================================


#include "PSection.h"
class PCameraControlsSection : public PSection {
public:
    // clang-format off
    PCameraControlsSection(ControlExec *ce)
    :
    PSection("Camera Controls", {
        new PTrackballWidget(ce),
        new PCameraProjectionWidget(ce),
        new PCameraFileGroup(ce)
    }) {}
    // clang-format on
};
