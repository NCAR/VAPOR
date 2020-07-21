#pragma once

#include "vapor/RenderParams.h"

#include "PSection.h"
#include "PLineItem.h"
#include "PWidgetHLI.h"

class VComboBox;

class PFidelitySection : public PSection {
public:
    PFidelitySection();
};

class PQuickFidelitySelector : public PLineItem {
    Q_OBJECT
    VComboBox *_vComboBox;
public:
    PQuickFidelitySelector();

protected:
    virtual void updateGUI() const override;
    bool requireDataMgr() const override { return true; }
    
private slots:
    void dropdownTextChanged(std::string text);
};

class PLODSelector : public PLineItem {
    Q_OBJECT
    VComboBox *_vComboBox;
public:
    PLODSelector();

protected:
    virtual void updateGUI() const override;
    bool requireDataMgr() const override { return true; }
    
private slots:
    void dropdownIndexChanged(int i);
};

class PRefinementSelector : public PLineItem {
    Q_OBJECT
    VComboBox *_vComboBox;
public:
    PRefinementSelector();

protected:
    virtual void updateGUI() const override;
    bool requireDataMgr() const override { return true; }
    
private slots:
    void dropdownIndexChanged(int i);
};

class PLODSelectorHLI :
    public PLODSelector,
    public PWidgetHLIBase<VAPoR::RenderParams, double>
{
public:
    PLODSelectorHLI() :
        PLODSelector(),
        PWidgetHLIBase<VAPoR::RenderParams, double> (
            (PWidget*) this,
            &VAPoR::RenderParams::GetCompressionLevel,
            &VAPoR::RenderParams::SetCompressionLevel
        )
    {}
};

class PRefinementSelectorHLI :
    public PRefinementSelector,
    public PWidgetHLIBase<VAPoR::RenderParams, double>
{
public:
    PRefinementSelectorHLI() :
        PRefinementSelector(),
        PWidgetHLIBase<VAPoR::RenderParams, double> (
            (PWidget*) this,
            &VAPoR::RenderParams::GetRefinementLevel,
            &VAPoR::RenderParams::SetRefinementLevel
        )
    {}
};
