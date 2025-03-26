#pragma once

#include "PGroup.h"
#include "VContainer.h"
#include "ParamsUpdatable.h"
#include "VHBoxWidget.h"

namespace VAPoR {
    class ControlExec;
}
class QRangeSliderTextCombo;
class VLabelPair;
class VLineItem;

//! \class PTimeRangeSelector
//! \brief A PWidget that maintains a TimeRangeSelector

class PTimeRangeSelector : public PWidget {
    Q_OBJECT

    ControlExec* _ce;
    QRangeSliderTextCombo* _slider;
    VLabelPair* _timeStampPair;

public:
    PTimeRangeSelector(ControlExec* ce);
    void updateGUI() const override;

private slots:
    void setTimes(float start, float end);
};
