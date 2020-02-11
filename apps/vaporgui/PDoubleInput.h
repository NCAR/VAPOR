#pragma once

#include "PLineItem.h"
//#include "VaporWidgetsFwd.h"

class VLineEdit;

class PDoubleInput : public PLineItem {
    Q_OBJECT
    
    VLineEdit *_doubleInput;
    
public:
    PDoubleInput(const std::string &tag, const std::string &label="");

protected:
    void updateGUI() const override;
    
private slots:
    void doubleInputValueChanged(const std::string &v);
};
