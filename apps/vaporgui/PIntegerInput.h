#pragma once

#include "PLineItem.h"
//#include "VaporWidgetsFwd.h"

class VIntSpinBox;

class PIntegerInput : public PLineItem {
    Q_OBJECT
    
    VIntSpinBox *_spinbox;
    
public:
    PIntegerInput(const std::string &tag, const std::string &label="");
    PIntegerInput *SetRange(int min, int max);

protected:
    void updateGUI() const override;
    
private slots:
    void spinboxValueChanged(int i);
};
