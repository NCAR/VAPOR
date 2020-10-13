#pragma once

#include "PLineItem.h"

class VComboBox;

namespace S {

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

};

