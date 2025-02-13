#include "PTimeRangeSelector.h"
#include "QRangeSliderTextCombo.h"
#include "VLineItem.h"
#include "QLabel.h"

#include "vapor/ControlExecutive.h"
#include "vapor/AnimationParams.h"

#include <cmath>

PTimeRangeSelector::PTimeRangeSelector(ControlExec* ce) 
    : PWidget("", 
        new VGroup({
            _slider = new QRangeSliderTextCombo(),
            _timeStampPair = new VLineItem("left", _rightTimestamp = new QLabel("right"))
        })
      ),
      _ce(ce)
{
    
    _rightTimestamp->setAlignment(Qt::AlignRight);
    _slider->SetNumDigits(0);
    connect(_slider, SIGNAL(ValueChanged(float, float)), this, SLOT(setTimes(float, float)));
}

void PTimeRangeSelector::updateGUI() const {
    AnimationParams* ap = dynamic_cast<AnimationParams*>(getParams());
    size_t start = ap->GetStartTimestep();
    size_t end = ap->GetEndTimestep();
    _slider->SetValue(start, end);
    _slider->SetRange(0, _ce->GetDataStatus()->GetTimeCoordinates().size()-1);

    std::vector<std::string> timeCoords = _ce->GetDataStatus()->GetTimeCoordsFormatted();
    _timeStampPair->SetLabelText(timeCoords[start]);
    _rightTimestamp->setText(QString::fromStdString(timeCoords[end]));
}

void PTimeRangeSelector::setTimes(float start, float end) {
    start = std::round(start);
    end = std::round(end);
    _slider->SetValue(start, end);

    AnimationParams* ap = dynamic_cast<AnimationParams*>(getParams());
    ap->SetStartTimestep(start);
    ap->SetEndTimestep(end);
}
