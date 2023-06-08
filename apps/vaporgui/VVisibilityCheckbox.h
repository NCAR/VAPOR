#pragma once

#include "VCheckBox.h"

class VVisibilityCheckbox : public VCheckBox {
    bool _isWhite;
public:
    VVisibilityCheckbox();
protected:
    void paintEvent(QPaintEvent *e) override;
private:
    void setColor(std::string color);
    void setBlack();
    void setWhite();
};
