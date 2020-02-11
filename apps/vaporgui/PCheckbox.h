#pragma once

#include "PLineItem.h"

class VCheckBox;

class PCheckbox : public PLineItem {
    Q_OBJECT
    
    VCheckBox *_qcheckbox;
    
public:
    PCheckbox(const std::string &tag, const std::string &label="");

protected:
    void updateGUI() const override;
    
private slots:
    void checkboxStateChanged(bool on);
};
