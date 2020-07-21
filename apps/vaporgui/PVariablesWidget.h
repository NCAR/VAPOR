#pragma once

#include "vapor/RenderParams.h"

#include "PVariableSelector.h"
#include "PLineItem.h"
#include "PWidgetHLI.h"

class VComboBox;


class PDimensionSelector : public PLineItem {
    Q_OBJECT
    VComboBox *_vComboBox;
public:
    PDimensionSelector();

protected:
    virtual void updateGUI() const override;
    bool requireDataMgr() const override { return true; }
    
private slots:
    void dropdownTextChanged(std::string text);
};


class PScalarVariableSelector   : public PVariableSelector   { 
public: 
    PScalarVariableSelector(); 
};

class PColorMapVariableSelector : public PVariableSelector   { 
public: 
    PColorMapVariableSelector(); 
};

class PHeightVariableSelector   : public PVariableSelector2D { 
public: 
    PHeightVariableSelector(); 
};

class PXFieldVariableSelector   : public PVariableSelector   { 
public: 
    PXFieldVariableSelector(); 
};

class PYFieldVariableSelector   : public PVariableSelector   { 
public: 
    PYFieldVariableSelector(); 
};

class PZFieldVariableSelector   : public PVariableSelector   { 
public: 
    PZFieldVariableSelector(); 
};

// ==================================
//       HLI Variable Selectors
// ==================================

class PScalarVariableSelectorHLI :
    public PScalarVariableSelector,
    public PWidgetHLIBase<VAPoR::RenderParams, std::string>
{
public:
    PScalarVariableSelectorHLI() :
        PScalarVariableSelector(),
        PWidgetHLIBase<VAPoR::RenderParams, std::string> (
            (PWidget*) this,
            &VAPoR::RenderParams::GetVariableName,
            &VAPoR::RenderParams::SetVariableName
        )
    {}
};

class PScalarVariableSelector2DHLI :
    public PVariableSelector2D,
    public PWidgetHLIBase<VAPoR::RenderParams, std::string>
{
public:
    PScalarVariableSelector2DHLI() :
        PVariableSelector2D("", "Variable Name"),
        PWidgetHLIBase<VAPoR::RenderParams, std::string> (
            (PWidget*) this,
            &VAPoR::RenderParams::GetVariableName,
            &VAPoR::RenderParams::SetVariableName
        )
    {}
};

class PScalarVariableSelector3DHLI :
    //public PScalarVariableSelector,
    public PVariableSelector3D,
    public PWidgetHLIBase<VAPoR::RenderParams, std::string>
{
public:
    PScalarVariableSelector3DHLI() :
        PVariableSelector3D("", "Variable Name"),
        PWidgetHLIBase<VAPoR::RenderParams, std::string> (
            (PWidget*) this,
            &VAPoR::RenderParams::GetVariableName,
            &VAPoR::RenderParams::SetVariableName
        )
    {}
};

class PColorMapVariableSelectorHLI :
    public PColorMapVariableSelector,
    public PWidgetHLIBase<VAPoR::RenderParams, std::string>
{
public:
    PColorMapVariableSelectorHLI() :
        PColorMapVariableSelector(),
        PWidgetHLIBase<VAPoR::RenderParams, std::string> (
            (PWidget*) this,
            &VAPoR::RenderParams::GetColorMapVariableName,
            &VAPoR::RenderParams::SetColorMapVariableName
        )
    {}
};

class PHeightVariableSelectorHLI :
    public PHeightVariableSelector,
    public PWidgetHLIBase<VAPoR::RenderParams, std::string>
{
public:
    PHeightVariableSelectorHLI() :
        PHeightVariableSelector(),
        PWidgetHLIBase<VAPoR::RenderParams, std::string> (
            (PWidget*) this,
            &VAPoR::RenderParams::GetHeightVariableName,
            &VAPoR::RenderParams::SetHeightVariableName
        )
    {}
};

class PXFieldVariableSelectorHLI :
    public PXFieldVariableSelector,
    public PWidgetHLIBase<VAPoR::RenderParams, std::string>
{
public:
    PXFieldVariableSelectorHLI() :
        PXFieldVariableSelector(),
        PWidgetHLIBase<VAPoR::RenderParams, std::string> (
            (PWidget*) this,
            &VAPoR::RenderParams::GetXFieldVariableName,
            &VAPoR::RenderParams::SetXFieldVariableName
        )
    {}
};

class PYFieldVariableSelectorHLI :
    public PYFieldVariableSelector,
    public PWidgetHLIBase<VAPoR::RenderParams, std::string>
{
public:
    PYFieldVariableSelectorHLI() :
        PYFieldVariableSelector(),
        PWidgetHLIBase<VAPoR::RenderParams, std::string> (
            (PWidget*) this,
            &VAPoR::RenderParams::GetYFieldVariableName,
            &VAPoR::RenderParams::SetYFieldVariableName
        )
    {}
};

class PZFieldVariableSelectorHLI :
    public PZFieldVariableSelector,
    public PWidgetHLIBase<VAPoR::RenderParams, std::string>
{
public:
    PZFieldVariableSelectorHLI() :
        PZFieldVariableSelector(),
        PWidgetHLIBase<VAPoR::RenderParams, std::string> (
            (PWidget*) this,
            &VAPoR::RenderParams::GetZFieldVariableName,
            &VAPoR::RenderParams::SetZFieldVariableName
        )
    {}
};
