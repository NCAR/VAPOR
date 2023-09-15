#pragma once

#include "PLineItem.h"

class VComboBox;

class PDimensionSelector : public PLineItem {
    VComboBox *_vComboBox;

public:
    PDimensionSelector();

protected:
    virtual void updateGUI() const override;
    bool         requireDataMgr() const override { return true; }

private:
    void dropdownTextChanged(std::string text);
};
