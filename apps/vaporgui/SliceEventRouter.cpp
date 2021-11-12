#include "SliceEventRouter.h"
#include <vapor/SliceParams.h>
#include "PWidgets.h"
#include "PSliceOriginSelector.h"

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
        new PSection("Slice Rotation", {
            (new PDoubleSliderEdit( SliceParams::XRotationTag, "X"))->SetRange(-90.,90.)->EnableDynamicUpdate(),
            (new PDoubleSliderEdit( SliceParams::YRotationTag, "Y"))->SetRange(-90.,90.)->EnableDynamicUpdate(),
            (new PDoubleSliderEdit( SliceParams::ZRotationTag, "Z"))->SetRange(-90.,90.)->EnableDynamicUpdate(),
        }),
        new PSection("Slice Origin", {
        new PSliceOriginSelector,
        }),
//        new PSection("Slice Origin", {
//            _xOriginSlider = (new PDoubleSliderEdit( SliceParams::XOriginTag, "X Origin"))->EnableDynamicUpdate(),
//            _yOriginSlider = (new PDoubleSliderEdit( SliceParams::YOriginTag, "Y Origin"))->EnableDynamicUpdate(),
//            _zOriginSlider = (new PDoubleSliderEdit( SliceParams::ZOriginTag, "Z Origin"))->EnableDynamicUpdate(),
//        }),
        new PGeometrySubtab,
    }));
    AddAnnotationSubtab(new PAnnotationColorbarWidget);

    // clang-format on
}

string SliceEventRouter::_getDescription() const
{
    return ("Displays an axis-aligned slice or cutting plane through"
            "a 3D variable.  Slices are sampled along the plane's axes according"
            " to a sampling rate define by the user.\n\n");
}
