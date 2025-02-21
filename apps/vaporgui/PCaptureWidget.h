#pragma once

#include "PWidget.h"

namespace VAPoR {
class ControlExec;
}

class VSection;
class PRadioButtons;
//class TimeRangeSelector;
class PTimeRangeSelector;
class QLabel;
class VLabel;

class PCaptureWidget : public PWidget {
    Q_OBJECT

    ControlExec *_ce;
    VSection *_section;
    //PRadioButton *_currentFrameButton, *_rangeButton;
    PRadioButtons *_radioButtons;
    PTimeRangeSelector *_timeSelector;
    VLabel *_startTime, *_endTime;

public:
    PCaptureWidget(VAPoR::ControlExec *ce);
    //void Update() override;

protected:
    virtual void updateGUI() const;

private slots:
    void _capture();
};
