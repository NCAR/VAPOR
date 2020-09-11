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
    bool         requireDataMgr() const override { return true; }

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
    bool         requireDataMgr() const override { return true; }

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
    bool         requireDataMgr() const override { return true; }

private slots:
    void dropdownIndexChanged(int i);
};
