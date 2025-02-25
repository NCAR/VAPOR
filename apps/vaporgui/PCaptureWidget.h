#pragma once

#include "PWidget.h"

namespace VAPoR {
class ControlExec;
}

class VSection;
class PSection;
class PRadioButtons;
//class TimeRangeSelector;
class PTimeRangeSelector;
class QLabel;
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
    static const std::string ALL;
};

struct CaptureFileTypes{ 
    static const std::string TIFF;
    static const std::string PNG;
};

class PCaptureLabel : public PWidget {
    //VLabel *_label;
    QLabel *_label;

public:
    PCaptureLabel();
    void updateGUI() const override;
};

class PCaptureToolbar : public PWidget {
    Q_OBJECT;

    ControlExec *_ce;
    MainForm *_mf;
    //PGroup *_pGroup;
    PHGroup *_pGroup;
    VHBoxWidget *_hBox;
    PStringDropdown *_fileTypeSelector;

    VComboBox *_typeCombo;
    VLabel *_fileLabel;
    PButton *_captureButton;

public:
    PCaptureToolbar(VAPoR::ControlExec *ce, MainForm *mf);

protected:
    virtual void updateGUI() const;

private slots:
    void _captureSingleImage();
    //void _captureTimeseries();
};
    
class PCaptureWidget : public PWidget {
    Q_OBJECT

    ControlExec *_ce;
    MainForm *_mf;
    PSection *_section;

public:
    PCaptureWidget(VAPoR::ControlExec *ce, MainForm *mf);
    //PCaptureWidget(VAPoR::ControlExec *ce);
    //void Update() override;

protected:
    virtual void updateGUI() const;

private slots:
    void _captureSingleImage();
    void _captureTimeseries();
};
