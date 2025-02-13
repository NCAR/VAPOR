#include "PCaptureWidget.h"
#include "PRadioButton.h"
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
    _currentFrameButton = new PRadioButton(AnimationParams::CaptureModeTag, "Current Frame"),
    _rangeButton        = new PRadioButton(AnimationParams::CaptureModeTag, "Range"),
    _timeSelector       = new PTimeRangeSelector(_ce);

    _section->layout()->addWidget(_currentFrameButton);
    _section->layout()->addWidget(_rangeButton);
    _section->layout()->addWidget(_timeSelector);
}

void PCaptureWidget::updateGUI() const {
    AnimationParams* ap = (AnimationParams*)_ce->GetParamsMgr()->GetParams(AnimationParams::GetClassType());
    _currentFrameButton->Update(ap);
    _rangeButton->Update(ap);
    _timeSelector->Update(ap);
}

//VAPoR::ParamsBase *PCaptureWidget::getWrappedParams() const { return NavigationUtils::GetActiveViewpointParams(_ce); }
