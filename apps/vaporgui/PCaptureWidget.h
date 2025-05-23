#pragma once

#include "PWidget.h"

namespace VAPoR {
class ControlExec;
}

class PSection;
class PEnumDropdownStandalone;
class VLabel;
class MainForm;
class PButton;
class VHBoxWidget;
class VComboBox;
class VLabel;

struct TiffStrings {
    static const std::string FileFilter;
    static const std::string FileSuffix;
};

struct PngStrings {
    static const std::string FileFilter;
    static const std::string FileSuffix;
};

class PCaptureHBox : public PWidget {
    Q_OBJECT;

    ControlExec *_ce;
    MainForm *_mf;
    VHBoxWidget *_hBox;
    PStringDropdown *_fileTypeSelector;
    const std::vector<long> _enumMap;

    PEnumDropdownStandalone *_typeCombo;
    VLabel *_fileLabel;
    PButton *_captureButton;
    PButton *_captureTimeSeriesButton;

public:
    PCaptureHBox(VAPoR::ControlExec *ce, MainForm *mf);

protected:
    virtual void updateGUI() const;

private slots:
    void _dropdownIndexChanged(int index);
    void _captureSingleImage();
    void _captureTimeSeries();
};
    
class PCaptureWidget : public PWidget {
    Q_OBJECT

    ControlExec *_ce;
    MainForm *_mf;
    PSection *_section;

public:
    PCaptureWidget(VAPoR::ControlExec *ce, MainForm *mf);

protected:
    virtual void updateGUI() const;
};
