#pragma once

#include "PLineItem.h"
#include "VaporWidgetsFwd.h"

class PDoubleInput : public PLineItem {
    Q_OBJECT

    VDoubleInput *_doubleInput;

public:
    PDoubleInput(const std::string &tag, const std::string &label = "");

protected:
    void updateGUI() const override;

private slots:
    void doubleInputValueChanged(double v);
};
