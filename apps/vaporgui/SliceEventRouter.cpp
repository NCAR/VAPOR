#include "SliceEventRouter.h"
#include <vapor/SliceParams.h>
#include "PWidgets.h"
#include "PSliceSampleLocationSelector.h"

using namespace VAPoR;

static RenderEventRouterRegistrar<SliceEventRouter> registrar(SliceEventRouter::GetClassType());

SliceEventRouter::SliceEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, SliceParams::GetClassType())
{
    // clang-format off

    AddVariablesSubtab(new PGroup({
        new PSection("Variable Selection", {
            new PScalarVariableSelector
        }),
        new PFidelitySection
    }));
    
    AddAppearanceSubtab(new PGroup({
        new PTFEditor,
        new PSection("Slice", {
            (new PDoubleSliderEdit(SliceParams::_sampleRateTag, "N Samples"))->SetRange(32, 2000)
        })
    }));
    
    AddGeometrySubtab(new PGroup({
        new PSection("Slice", {
            (new PDoubleSliderEdit( SliceParams::XRotationTag, "X Rotation"))->SetRange(-90.,90.),
            (new PDoubleSliderEdit( SliceParams::YRotationTag, "Y Rotation"))->SetRange(-90.,90.),
            (new PDoubleSliderEdit( SliceParams::ZRotationTag, "Z Rotation"))->SetRange(-90.,90.),
            new POrientationSelector,
            new PSliceSampleLocationSelector,
        }),
        new PGeometrySubtab,
    }));
    AddAnnotationSubtab(new PAnnotationColorbarWidget);

    // clang-format on
}

void SliceEventRouter::updateTab() {
    ParamsBase *p = GetActiveParams();
    double xr = p->GetValueDouble( SliceParams::XRotationTag, 0. );
    double yr = p->GetValueDouble( SliceParams::YRotationTag, 0. );
    double zr = p->GetValueDouble( SliceParams::ZRotationTag, 0. );

    std::cout << "R: " << xr << " " << yr << " " << zr << std::endl;
    
    RenderEventRouter::updateTab();
}

string SliceEventRouter::_getDescription() const
{
    return ("Displays an axis-aligned slice or cutting plane through"
            "a 3D variable.  Slices are sampled along the plane's axes according"
            " to a sampling rate define by the user.\n\n");
}
