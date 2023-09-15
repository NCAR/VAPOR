#pragma once

#include "PWidget.h"

class PEnumDropdown;

//! \class POrientationSelector
//! \brief Widget provides the user the option of switching a renderer between 2D and 3D.
//! \author Stas Jaroszynski

class POrientationSelector : public PWidget {
    PEnumDropdown *_dropdown;

public:
    POrientationSelector();

protected:
    void updateGUI() const override;
};
