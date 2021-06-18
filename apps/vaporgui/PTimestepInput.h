#pragma once

#include "PWidget.h"

namespace VAPoR {
class ControlExec;
}
class VIntSpinBox;


class PTimestepInput : public PWidget {
    VAPoR::ControlExec *_ce;
    VIntSpinBox *       _input;

public:
    PTimestepInput(VAPoR::ControlExec *ce);

protected:
    void updateGUI() const override;
    void inputChanged(int v);
};
