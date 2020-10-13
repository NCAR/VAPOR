#pragma once

#include "PWidget.h"

class PSection;

class PAnnotationColorbarWidget : public PWidget {
    PSection *_pSection;
public:
    PAnnotationColorbarWidget();
protected:
    void updateGUI() const override;
};
