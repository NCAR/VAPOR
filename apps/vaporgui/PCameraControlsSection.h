#pragma once


// =====================================
//           PTrackballWidget
// =====================================


#include "PWidget.h"
#include "NavigationUtils.h"

class VGroup;
class V3DInput;

class PTrackballWidget : public PWidget {
    ControlExec *_ce;
    VGroup *_group;
    V3DInput *_direction;
    V3DInput *_up;
    V3DInput *_position;
    V3DInput *_origin;
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
    ControlExec *_ce;
    VComboBox *_dropdown;
    static const string Perspective;
    static const string MapOrthographic;
public:
    PCameraProjectionWidget(ControlExec *ce);
    
protected:
    void updateGUI() const override;
    void dropdownChanged(string s);
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
    }) {}
    // clang-format on
};
