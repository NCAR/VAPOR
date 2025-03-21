#pragma once

#include "PWidget.h"

namespace VAPoR {
class ControlExec;
}

class PSection;
class VLabel;
class PHGroup;
class MainForm;
class PButton;
class VHBoxWidget;
class VComboBox;
class VLabel;

struct CaptureModes { 
    static const std::string CURRENT;
    static const std::string RANGE;
};

struct TiffStrings {
    static const std::string CaptureFileType;
    static const std::string FileFilter;
    static const std::string FileSuffix;
};

struct PngStrings {
    static const std::string CaptureFileType;
    static const std::string FileFilter;
    static const std::string FileSuffix;
};

class PCaptureHBox : public PWidget {
    Q_OBJECT;

    ControlExec *_ce;
    MainForm *_mf;
    PHGroup *_pGroup;
    VHBoxWidget *_hBox;
    PStringDropdown *_fileTypeSelector;

    VComboBox *_typeCombo;
    VLabel *_fileLabel;
    PButton *_captureButton;
    PButton *_captureTimeSeriesButton;

public:
    PCaptureHBox(VAPoR::ControlExec *ce, MainForm *mf);

protected:
    virtual void updateGUI() const;

private slots:
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
