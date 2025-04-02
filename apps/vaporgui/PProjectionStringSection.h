#pragma once

#include "PWidget.h"
#include "PWidgetsFwd.h"
#include "VFrame.h"
#include "Updatable.h"

namespace VAPoR {
class ControlExec;
};
using VAPoR::ControlExec;
class QLabel;
class QPlainTextEdit;
class VComboBox;
class VPushButton;
class VLabel;

class PProjectionStringSection : public PWidget {
    Q_OBJECT
    ControlExec *   _ce;
    PDisplay *      _currentProjDisp;
    VLabel *        _selectedProjDisp;
    VComboBox *     _datasetDropdown;
    QPlainTextEdit *_customStrEdit;
    VPushButton *   _applyButton;
    std::string     _changeToDataset;

public:
    PProjectionStringSection(ControlExec *ce);

protected:
    void updateGUI() const override;
    void datasetDropdownChanged(std::string value);
    void applyClicked();
};

class VProjectionStringFrame : public VFrame , public ParamsUpdatable {
    Q_OBJECT
   
    PProjectionStringSection *_section;
 
public:
    VProjectionStringFrame(PProjectionStringSection *psection);
    void Update(VAPoR::ParamsBase *p, VAPoR::ParamsMgr *pm = nullptr, VAPoR::DataMgr *dm = nullptr) override;

protected:
    void closeEvent(QCloseEvent *event) override;

signals:
    void closed();
};
