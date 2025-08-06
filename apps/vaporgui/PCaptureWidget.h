#pragma once

#include "PWidget.h"

namespace VAPoR {
class ControlExec;
}

class PSection;
class PEnumDropdownStandalone;
class VLabel;
class PButton;
class VHBoxWidget;
class VComboBox;
class VLabel;
class CaptureController;

class PCaptureHBox : public PWidget {
    Q_OBJECT;

    ControlExec *_ce;
    CaptureController *_captureController;
    VHBoxWidget *_hBox;
    PStringDropdown *_fileTypeSelector;
    const std::vector<long> _enumMap;

    PEnumDropdownStandalone *_typeCombo;
    VLabel *_fileLabel;
    PButton *_captureButton;
    PButton *_captureTimeSeriesButton;

public:
    PCaptureHBox(VAPoR::ControlExec *ce, CaptureController *captureController);

protected:
    virtual void updateGUI() const;

private slots:
    void _dropdownIndexChanged(int index);
};
    
class PCaptureWidget : public PWidget {
    Q_OBJECT

    ControlExec *_ce;
    PSection *_section;

public:
    PCaptureWidget(VAPoR::ControlExec *ce, CaptureController *captureController);

protected:
    virtual void updateGUI() const;
};
