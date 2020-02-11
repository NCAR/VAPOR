#pragma once

#include "PWidget.h"

class VSection;
class PGroup;

class PSection : public PWidget {
    Q_OBJECT
    
    VSection *_vsection;
    PGroup *_pgroup;
    
public:
    PSection(const std::string &label="");
    PSection *Add(PWidget *pw);

protected:
    void updateGUI() const override;
};
