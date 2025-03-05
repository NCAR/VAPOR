#pragma once

//#include "PWidget.h"
#include "PGroup.h"
#include "VContainer.h"
#include "ParamsUpdatable.h"

namespace VAPoR {
    class ControlExec;
}
class QRangeSliderTextCombo;
class VLineItem;
class QLabel;

//! \class PTimeRangeSelector
//! \brief A PWidget that maintains a TimeRangeSelector

class PTimeRangeSelector : public PWidget {
    Q_OBJECT

    ControlExec* _ce;
    QRangeSliderTextCombo* _slider;
    VLineItem* _timeStampPair;
    QLabel* _rightTimestamp;

public:
    PTimeRangeSelector(ControlExec* ce);
    void updateGUI() const override;

private slots:
    void setTimes(float start, float end);
};
