#pragma once
#include "PWidget.h"

class VComboBox;

class PVisualizerSelector : public PWidget {
    Q_OBJECT

    VComboBox *_dropdown;

public:
    PVisualizerSelector();

protected:
    void updateGUI() const override;
    bool requireParamsMgr() const override { return true; }

private:
    void dropdownTextChanged(std::string text);
};
