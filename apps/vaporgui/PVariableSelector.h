#pragma once

#include "PStringDropdown.h"

class VCheckBox;

//! \class PVariableSelector
//! Allows the user to select variables. Automatically switches between 2D and 3D
//! based on the currently selected variable.
//!
//! Designed to be used with a RenderParams object.

class PVariableSelector : public PStringDropdown {
    Q_OBJECT

public:
    PVariableSelector(const std::string &tag, const std::string &label = "");

protected:
    void        updateGUI() const override;
    bool        requireDataMgr() const override { return true; }
    virtual int getDimensionality() const;
};

//! \class PVariableSelector2D
//! 2D only version of PVariableSelector
//! \copydoc PVariableSelector

class PVariableSelector2D : public PVariableSelector {
public:
    using PVariableSelector::PVariableSelector;

protected:
    int getDimensionality() const override { return 2; }
};

//! \class PVariableSelector3D
//! 3D only version of PVariableSelector
//! \copydoc PVariableSelector

class PVariableSelector3D : public PVariableSelector {
public:
    using PVariableSelector::PVariableSelector;

protected:
    int getDimensionality() const override { return 3; }
};
