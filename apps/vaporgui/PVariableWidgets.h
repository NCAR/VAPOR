#pragma once

#include <QScrollArea>

#include "vapor/RenderParams.h"

#include "PStringDropdown.h"
#include "PLineItem.h"
#include "PWidgetHLI.h"
#include "PGroup.h"

class VComboBox;
class PSection;

class PDimensionSelector : public PLineItem {
    Q_OBJECT
    VComboBox *_vComboBox;

public:
    PDimensionSelector();

protected:
    virtual void updateGUI() const override;
    bool         requireDataMgr() const override { return true; }

protected slots:
    virtual void dropdownTextChanged(std::string text);
};

//! \class PVariableSelector
//! Allows the user to select variables. Automatically switches between 2D and 3D
//! based on the currently selected variable.
//!
//! Designed to be used with a RenderParams object.

class PVariableSelector : public PStringDropdown {
    Q_OBJECT
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

// ==================================
//       HLI Variable Selectors
// ==================================

class PScalarVariableSelectorHLI : public PScalarVariableSelector, public PWidgetHLIBase<VAPoR::RenderParams, std::string> {
public:
    PScalarVariableSelectorHLI()
    : PScalarVariableSelector(), PWidgetHLIBase<VAPoR::RenderParams, std::string>((PWidget *)this, &VAPoR::RenderParams::GetVariableName, &VAPoR::RenderParams::SetVariableName)
    {
    }
};

class PScalarVariableSelector2DHLI : public PVariableSelector2D, public PWidgetHLIBase<VAPoR::RenderParams, std::string> {
public:
    PScalarVariableSelector2DHLI()
    : PVariableSelector2D("", "Variable Name"), PWidgetHLIBase<VAPoR::RenderParams, std::string>((PWidget *)this, &VAPoR::RenderParams::GetVariableName, &VAPoR::RenderParams::SetVariableName)
    {
    }
};

class PScalarVariableSelector3DHLI :
    // public PScalarVariableSelector,
    public PVariableSelector3D,
    public PWidgetHLIBase<VAPoR::RenderParams, std::string> {
public:
    PScalarVariableSelector3DHLI()
    : PVariableSelector3D("", "Variable Name"), PWidgetHLIBase<VAPoR::RenderParams, std::string>((PWidget *)this, &VAPoR::RenderParams::GetVariableName, &VAPoR::RenderParams::SetVariableName)
    {
    }
};

class PColorMapVariableSelectorHLI : public PColorMapVariableSelector, public PWidgetHLIBase<VAPoR::RenderParams, std::string> {
public:
    PColorMapVariableSelectorHLI()
    : PColorMapVariableSelector(), PWidgetHLIBase<VAPoR::RenderParams, std::string>((PWidget *)this, &VAPoR::RenderParams::GetColorMapVariableName, &VAPoR::RenderParams::SetColorMapVariableName)
    {
    }
};

class PHeightVariableSelectorHLI : public PHeightVariableSelector, public PWidgetHLIBase<VAPoR::RenderParams, std::string> {
public:
    PHeightVariableSelectorHLI()
    : PHeightVariableSelector(), PWidgetHLIBase<VAPoR::RenderParams, std::string>((PWidget *)this, &VAPoR::RenderParams::GetHeightVariableName, &VAPoR::RenderParams::SetHeightVariableName)
    {
    }
};

class PXFieldVariableSelectorHLI : public PXFieldVariableSelector, public PWidgetHLIBase<VAPoR::RenderParams, std::string> {
public:
    PXFieldVariableSelectorHLI()
    : PXFieldVariableSelector(), PWidgetHLIBase<VAPoR::RenderParams, std::string>((PWidget *)this, &VAPoR::RenderParams::GetXFieldVariableName, &VAPoR::RenderParams::SetXFieldVariableName)
    {
    }
};

class PYFieldVariableSelectorHLI : public PYFieldVariableSelector, public PWidgetHLIBase<VAPoR::RenderParams, std::string> {
public:
    PYFieldVariableSelectorHLI()
    : PYFieldVariableSelector(), PWidgetHLIBase<VAPoR::RenderParams, std::string>((PWidget *)this, &VAPoR::RenderParams::GetYFieldVariableName, &VAPoR::RenderParams::SetYFieldVariableName)
    {
    }
};

class PZFieldVariableSelectorHLI : public PZFieldVariableSelector, public PWidgetHLIBase<VAPoR::RenderParams, std::string> {
public:
    PZFieldVariableSelectorHLI()
    : PZFieldVariableSelector(), PWidgetHLIBase<VAPoR::RenderParams, std::string>((PWidget *)this, &VAPoR::RenderParams::GetZFieldVariableName, &VAPoR::RenderParams::SetZFieldVariableName)
    {
    }
};
