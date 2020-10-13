#pragma once

#include "PWidget.h"

class PEnumDropdown;

class POrientationSelector : public PWidget {
    PEnumDropdown *_dropdown;
public:
    POrientationSelector();
protected:
    void updateGUI() const override;
};
