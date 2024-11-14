#pragma once

#include "PWidget.h"
#include "PWidgetsFwd.h"

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
