#include "PMovingDomainSettings.h"
#include "PCheckbox.h"
#include <vapor/ControlExecutive.h>
#include <vapor/GUIStateParams.h>

PMovingDomainSettings::PMovingDomainSettings(ControlExec *ce)
    : PSection("Moving Domain", {
        (new PCheckbox(GUIStateParams::MovingDomainTrackCameraTag, "Track Camera"))->SetTooltip("Camera should follow the moving domain"),
        (new PCheckbox(GUIStateParams::MovingDomainTrackRenderRegionsTag, "Track Rendered Regions"))->SetTooltip("Renderer regions should be relative to the moving domain"),
    }), _ce(ce) 
{
    SetTooltip("Control behaviors related to moving domains. Disabled if dataset does not contain a moving domain.");
}

bool PMovingDomainSettings::isEnabled() const {
    for (auto name : _ce->GetDataStatus()->GetDataMgrNames())
        if (_ce->GetDataStatus()->GetDataMgr(name)->HasMovingDomain())
            return true;
    return false;
}
