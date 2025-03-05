#include "POutputResolutionSection.h"
#include "PWidgets.h"
#include <vapor/ViewpointParams.h>
#include <vapor/ControlExecutive.h>
#include <vapor/NavigationUtils.h>


typedef VAPoR::ViewpointParams VP;


// clang-format off
POutputResolutionSection::POutputResolutionSection(VAPoR::ControlExec *ce)
: PWidgetWrapper(
    new PSection("Output Resolution", {
        new PCheckbox(VP::UseCustomFramebufferTag, "Use Custom Output Size"),
        (new PIntegerInput(VP::CustomFramebufferWidthTag,  "Output Width (px)"))->SetRange(1, 16384)->EnableBasedOnParam(VP::UseCustomFramebufferTag),
        (new PIntegerInput(VP::CustomFramebufferHeightTag, "Output Height (px)"))->SetRange(1, 16384)->EnableBasedOnParam(VP::UseCustomFramebufferTag),
    }
)), _ce(ce) {}
// clang-format on


VAPoR::ParamsBase *POutputResolutionSection::getWrappedParams() const { return NavigationUtils::GetActiveViewpointParams(_ce); }
