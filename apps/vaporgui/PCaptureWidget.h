#pragma once

#include "PWidget.h"

namespace VAPoR {
class ControlExec;
}

class VSection;
class PRadioButton;
//class TimeRangeSelector;
class PTimeRangeSelector;
class QLabel;
class VLabel;

class PCaptureWidget : public PWidget {
    ControlExec *_ce;
    VSection *_section;
    PRadioButton *_currentFrameButton, *_rangeButton;
    PTimeRangeSelector *_slider;
    VLabel *_startTime, *_endTime;

public:
    PCaptureWidget(VAPoR::ControlExec *ce);
    //void Update() override;

protected:
    virtual void updateGUI() const;
};
