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

class PProjectionStringWidget : public PWidget {
    Q_OBJECT
    ControlExec *   _ce;
    PDisplay *      _currentProjDisp;
    VLabel *        _selectedProjDisp;
    VComboBox *     _datasetDropdown;
    QPlainTextEdit *_customStrEdit;
    VPushButton *   _applyButton;
    std::string     _changeToDataset;

public:
    PProjectionStringWidget(ControlExec *ce);

protected:
    void updateGUI() const override;
    void datasetDropdownChanged(std::string value);
    void applyClicked();
};
