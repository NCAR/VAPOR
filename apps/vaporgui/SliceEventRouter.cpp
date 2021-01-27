#include "SliceEventRouter.h"
#include <vapor/SliceParams.h>
#include "PWidgets.h"
#include "PSliceSampleLocationSelector.h"

using namespace VAPoR;

static RenderEventRouterRegistrar<SliceEventRouter> registrar(SliceEventRouter::GetClassType());

SliceEventRouter::SliceEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, SliceParams::GetClassType())
{
    // clang-format off

    AddSubtab("Variables", new PGroup({
        new PSection("Variable Selection", {
            new PScalarVariableSelector
        }),
        new PFidelitySection
    }));
    
    AddSubtab("Appearance", new PGroup({
        new PTFEditor,
        new PSection("Slice", {
            new POrientationSelector,
            new PSliceSampleLocationSelector,
            (new PDoubleSliderEdit(SliceParams::_sampleRateTag, "N Samples"))->SetRange(32, 2000)
        })
    }));
    
    AddSubtab("Geometry", new PGeometrySubtab);
    AddSubtab("Annotation", new PAnnotationColorbarWidget);

    // clang-format on
}

string SliceEventRouter::_getDescription() const
{
    return ("Displays an axis-aligned slice or cutting plane through"
            "a 3D variable.  Slices are sampled along the plane's axes according"
            " to a sampling rate define by the user.\n\n");
}
