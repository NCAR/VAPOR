#include "ExportTab.h"
#define INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
#include <vapor/LegacyVectorMath.h>
#include <vapor/GUIStateParams.h>
#include <vapor/AnimationParams.h>
#include <QHBoxLayout>
#include "PWidgets.h"
#include "PProjectionStringSection.h"
#include "PCameraControlsSection.h"
#include "POutputResolutionSection.h"
#include "PTimestepSliderEdit.h"
#include "PTotalTimestepsDisplay.h"
#include "PCaptureWidget.h"

using namespace VAPoR;


class PMovingDomainSettings : public PSection {
    ControlExec *_ce;
public:
    PMovingDomainSettings(ControlExec *ce)
    : PSection("Moving Domain", {
        (new PCheckbox(GUIStateParams::MovingDomainTrackCameraTag, "Track Camera"))->SetTooltip("Camera should follow the moving domain"),
        (new PCheckbox(GUIStateParams::MovingDomainTrackRenderRegionsTag, "Track Rendered Regions"))->SetTooltip("Renderer regions should be relative to the moving domain"),
    }), _ce(ce) {
        SetTooltip("Control behaviors related to moving domains. Disabled if dataset does not contain a moving domain.");
    }
protected:
    bool isEnabled() const override {
        for (auto name : _ce->GetDataStatus()->GetDataMgrNames())
            if (_ce->GetDataStatus()->GetDataMgr(name)->HasMovingDomain())
                return true;
        return false;
    }
};


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

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);
    layout()->addWidget(_pg);
    l->addStretch();
}

void ExportTab::Update()
{
    //auto vp = NavigationUtils::GetActiveViewpointParams(_controlExec);
    //if (!(isEnabled() && vp))
    //    return;

    if (_pg) _pg->Update(_ce->GetParams<AnimationParams>());
    //if (_pg) _pg->Update();
}
