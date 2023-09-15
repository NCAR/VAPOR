#pragma once

#include "PLineItem.h"

class VIntSliderEdit;

namespace VAPoR {
class ControlExec;
};

//! \class PTimestepSliderEdit
//! Creates a timestep slider input

class PTimestepSliderEdit : public PLineItem {
    VIntSliderEdit *    _sliderEdit;
    VAPoR::ControlExec *_ce;

public:
    PTimestepSliderEdit(VAPoR::ControlExec *ce);

protected:
    void updateGUI() const override;

private:
    void valueChanged(int v);
};
