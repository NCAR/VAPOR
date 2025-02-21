#include "PCaptureWidget.h"
#include "PRadioButtons.h"
#include "VSection.h"
#include "PTimeRangeSelector.h"
#include "QLabel.h"
#include "VLabel.h"
#include "VHBoxWidget.h"
#include "VPushButton.h"
#include "VLineItem.h"

#include "vapor/GUIStateParams.h"
#include "vapor/AnimationParams.h"
#include "vapor/ViewpointParams.h"
#include "vapor/ControlExecutive.h"
#include "vapor/NavigationUtils.h"

#include <QHBoxLayout>
#include <QLabel>

typedef VAPoR::ViewpointParams VP;

PCaptureWidget::PCaptureWidget(VAPoR::ControlExec *ce)
    : PWidget("", _section = new VSection("Image(s)")), _ce(ce)
{
    //_currentFrameButton = new PRadioButton(AnimationParams::CaptureModeTag, "Current Frame"),
    //_rangeButton        = new PRadioButton(AnimationParams::CaptureModeTag, "Range"),
    _radioButtons       = new PRadioButtons(AnimationParams::CaptureModeTag, {"Current Frame", "Range"});
    _timeSelector       = new PTimeRangeSelector(_ce);

    VPushButton* captureButton = new VPushButton("Capture");
    connect(captureButton, SIGNAL(ButtonClicked()), this, SLOT(_capture()));

    //_section->layout()->addWidget(_currentFrameButton);
    //_section->layout()->addWidget(_rangeButton);
    _section->layout()->addWidget(_radioButtons);
    _section->layout()->addWidget(_timeSelector);
    _section->layout()->addWidget(captureButton);
}

void PCaptureWidget::updateGUI() const {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    //_currentFrameButton->Update(ap);
    //_rangeButton->Update(ap);
    _radioButtons->Update(ap);
    _timeSelector->Update(ap);
}

void PCaptureWidget::_capture() {
    std::cout << "Capture" << std::endl;
}
