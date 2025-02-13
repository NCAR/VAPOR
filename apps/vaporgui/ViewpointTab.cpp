#include "ViewpointTab.h"
#define INCLUDE_DEPRECATED_LEGACY_VECTOR_MATH
#include <vapor/LegacyVectorMath.h>
#include <vapor/GUIStateParams.h>
#include <QHBoxLayout>
#include "PWidgets.h"
#include "PProjectionStringSection.h"
#include "PCameraControlsSection.h"
#include "POutputResolutionSection.h"

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


ViewpointTab::ViewpointTab(ControlExec *ce) : _controlExec(ce)
{
    PProjectionStringSection *proj;
    _pg = new PGroup({
        new PCameraControlsSection(_controlExec),
        _movingDomainSection = new PMovingDomainSettings(ce),
        new POutputResolutionSection(_controlExec),
        proj = new PProjectionStringSection(_controlExec),
    });

    QVBoxLayout *l = new QVBoxLayout;
    l->setContentsMargins(0, 0, 0, 0);
    setLayout(l);
    layout()->addWidget(_pg);
    l->addStretch();
}

void ViewpointTab::Update()
{
    auto vp = NavigationUtils::GetActiveViewpointParams(_controlExec);
    if (!(isEnabled() && vp))
        return;

    if (_pg) _pg->Update(vp);
}
