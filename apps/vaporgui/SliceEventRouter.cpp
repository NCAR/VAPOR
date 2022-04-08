#include "SliceEventRouter.h"
#include <vapor/SliceParams.h>
#include "PWidgets.h"
#include "PSliceController.h"

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
            (new PDoubleSliderEdit(RenderParams::SampleRateTag, "N Samples"))->SetRange(32, 2000)
        })
    }));
    
    AddGeometrySubtab(new PGroup({
        new PSliceController,
        new PGeometrySubtab,
    }));
    AddAnnotationSubtab(new PAnnotationColorbarWidget);

    // clang-format on
}

string SliceEventRouter::_getDescription() const
{
    return ("Displays a slice or cutting plane through"
            "a 3D variable.  Slices are sampled along the plane's axes according"
            " to a sampling rate define by the user.\n\n");
}
