#pragma once

#include "PWidget.h"

namespace VAPoR {
class ControlExec;
}
class VLabel;


class PTotalTimestepsDisplay : public PWidget {
    VAPoR::ControlExec *_ce;
    VLabel *_label;
public:
    PTotalTimestepsDisplay(VAPoR::ControlExec *ce);
protected:
    void updateGUI() const override;
};
