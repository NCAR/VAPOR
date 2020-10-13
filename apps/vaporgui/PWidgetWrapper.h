#pragma once

#include "PWidget.h"

class PWidgetWrapper : public PWidget {
    PWidget *_child;
public:
    PWidgetWrapper(PWidget *p);
    PWidgetWrapper(std::string tag, PWidget *p);
protected:
    void updateGUI() const override;
};
