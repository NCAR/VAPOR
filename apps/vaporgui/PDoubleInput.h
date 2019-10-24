#pragma once

#include "PWidget.h"
#include "VaporWidgetsFwd.h"

class PDoubleInput : public PWidget {
    Q_OBJECT
    
    VDoubleInput *_doubleInput;
    
public:
    PDoubleInput(const std::string &tag, const std::string &label="");

protected:
    void update() const;
    
private slots:
    void doubleInputValueChanged(double v);
};
