#include "PTimeRangeSelector.h"
#include "vapor/ControlExecutive.h"
#include "vapor/AnimationParams.h"
#include "QRangeSliderTextCombo.h"
#include "VLabelPair.h"

#include <cmath>

PTimeRangeSelector::PTimeRangeSelector(ControlExec* ce) 
    : PWidget("", 
        new VGroup({
            _slider = new QRangeSliderTextCombo(),
            _timeStampPair = new VLabelPair(),
        })
      ),
      _ce(ce)
{
    _slider->SetNumDigits(0);
    connect(_slider, SIGNAL(ValueChanged(float, float)), this, SLOT(setTimes(float, float)));
}

void PTimeRangeSelector::updateGUI() const {
    _slider->SetRange(0, _ce->GetDataStatus()->GetTimeCoordinates().size()-1);

    AnimationParams* ap = dynamic_cast<AnimationParams*>(getParams());
    size_t start = ap->GetValueLong(AnimationParams::CaptureStartTag, ap->GetStartTimestep());
    size_t end = ap->GetValueLong(AnimationParams::CaptureEndTag, ap->GetEndTimestep());
    _slider->SetValue(start, end);

    std::vector<std::string> timeCoords = _ce->GetDataStatus()->GetTimeCoordsFormatted();
    _timeStampPair->SetLeftText(timeCoords[start]);
    _timeStampPair->SetRightText(timeCoords[end]);
}

void PTimeRangeSelector::setTimes(float start, float end) {
    start = std::round(start);
    end = std::round(end);
    _slider->SetValue(start, end);

    AnimationParams* ap = dynamic_cast<AnimationParams*>(getParams());
    ap->SetValueLong(AnimationParams::CaptureStartTag, "Set value for capturing imagery start time", start);
    ap->SetValueLong(AnimationParams::CaptureEndTag, "Set value for capturing imagery end time", end);
}
