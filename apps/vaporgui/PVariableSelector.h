#pragma once

#include "PStringDropdown.h"

//! \class PVariableSelector
//! \author Stas Jaroszynski
//! Allows the user to select variables. Automatically switches between 2D and 3D
//! based on the currently selected variable.
//!
//! Designed to be used with a RenderParams object.

class PVariableSelector : public PStringDropdown {
    bool _addNull = false;
    int  _onlyShowForDim = -1;

public:
    PVariableSelector(const std::string &tag, const std::string &label = "");
    PVariableSelector *AddNullOption()
    {
        _addNull = true;
        return this;
    }
    PVariableSelector *OnlyShowForDim(int dim)
    {
        _onlyShowForDim = dim;
        return this;
    }

protected:
    void         updateGUI() const override;
    bool         isShown() const override;
    bool         requireDataMgr() const override { return true; }
    int          getRendererDimension() const;
    virtual int  getDimensionality() const;
    virtual void dropdownTextChanged(std::string text) override;
};

//! \class PVariableSelector2D
//! \author Stas Jaroszynski
//! 2D only version of PVariableSelector
//! \copydoc PVariableSelector

class PVariableSelector2D : public PVariableSelector {
public:
    using PVariableSelector::PVariableSelector;

protected:
    int getDimensionality() const override { return 2; }
};

//! \class PVariableSelector3D
//! \author Stas Jaroszynski
//! 3D only version of PVariableSelector
//! \copydoc PVariableSelector

class PVariableSelector3D : public PVariableSelector {
public:
    using PVariableSelector::PVariableSelector;

protected:
    int getDimensionality() const override { return 3; }
};

class PScalarVariableSelector : public PVariableSelector {
public:
    PScalarVariableSelector();
};
class PColorMapVariableSelector : public PVariableSelector {
public:
    PColorMapVariableSelector();
};
class PHeightVariableSelector : public PVariableSelector2D {
public:
    PHeightVariableSelector();
};
class PXFieldVariableSelector : public PVariableSelector {
public:
    PXFieldVariableSelector();
};
class PYFieldVariableSelector : public PVariableSelector {
public:
    PYFieldVariableSelector();
};
class PZFieldVariableSelector : public PVariableSelector {
public:
    PZFieldVariableSelector();
};
