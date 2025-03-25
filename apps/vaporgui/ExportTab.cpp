#include "ExportTab.h"
#include "PCheckbox.h"
#include "PIntegerInput.h"
#include "PSliderEdit.h"
#include "PCameraControlsSection.h"
#include "POutputResolutionSection.h"
#include "PTimestepSliderEdit.h"
#include "PTotalTimestepsDisplay.h"
#include "PCaptureWidget.h"
#include "PMovingDomainSettings.h"
#include <vapor/AnimationParams.h>
#include <QLayout>

ExportTab::ExportTab(ControlExec *ce, MainForm *mf) : _ce(ce)
{
    _pg = new PGroup({
        new PCaptureWidget(_ce, mf),
        new POutputResolutionSection(_ce),
        new PCameraControlsSection(_ce),
        _movingDomainSection = new PMovingDomainSettings(_ce),
        new PGroup({
            new PSection("Animation", {
                new PTimestepSliderEdit(_ce),
                new PTotalTimestepsDisplay(_ce),
                new PCheckbox(AnimationParams::_repeatTag, "Loop Animation Playback"),
                (new PIntegerInput(AnimationParams::_stepSizeTag, "Animation Play Step Size"))->SetRange(1, 10),
                (new PDoubleSliderEdit(AnimationParams::_maxRateTag, "Max Animation Frames Per Second"))->SetRange(1, 60),
            })
        })
    });

    layout()->addWidget(_pg);
}

void ExportTab::Update()
{
    if (_pg) _pg->Update(_ce->GetParams<AnimationParams>());
}
