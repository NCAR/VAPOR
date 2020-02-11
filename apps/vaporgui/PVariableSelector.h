#pragma once

#include "PStringDropdown.h"

class VCheckBox;

//! \class PVariableSelector
//! Allows the user to select variables. Automatically switches between 2D and 3D
//! based on the configuration of the Renderer's Box class.
//!
//! Designed to be used with a RenderParams object.

class PVariableSelector : public PStringDropdown {
    Q_OBJECT
    
public:
    PVariableSelector(const std::string &tag, const std::string &label="");

protected:
    void updateGUI() const override;
    bool requireDataMgr() const override { return true; }
};
