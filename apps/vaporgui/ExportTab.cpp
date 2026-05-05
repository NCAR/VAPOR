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
#include "CaptureController.h"
#include <vapor/AnimationParams.h>
#include <QLayout>

ExportTab::ExportTab(ControlExec *ce, CaptureController *captureController) : _ce(ce)
{
    _pg = new PGroup({
        new PCaptureWidget(_ce, captureController),
        new POutputResolutionSection(_ce),
        new PCameraControlsSection(_ce),
        _movingDomainSection = new PMovingDomainSettings(_ce),
        new PGroup({
            new PSection("Animation", {
                new PTimestepSliderEdit(_ce),
                new PTotalTimestepsDisplay(_ce),
            })
        })
    });

    layout()->addWidget(_pg);
}

void ExportTab::Update()
{
    if (_pg) _pg->Update(_ce->GetParams<AnimationParams>());
}
