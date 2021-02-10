#include "WireFrameEventRouter.h"
#include "vapor/WireFrameParams.h"
#include "PWidgets.h"

using namespace VAPoR;

static RenderEventRouterRegistrar<WireFrameEventRouter> registrar(WireFrameEventRouter::GetClassType());

WireFrameEventRouter::WireFrameEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, WireFrameParams::GetClassType())
{
    // clang-format off

    AddSubtab("Variables", new PGroup({
        new PSection("Variable Selection", {
            new PDimensionSelector,
            new PScalarVariableSelector,
            new PHeightVariableSelector
        }),
        new PFidelitySection
    }));
    
    AddSubtab("Appearance", (new PTFEditor)->ShowOpacityBasedOnParam("NULL", 1));
    AddSubtab("Geometry", new PGeometrySubtab);
    AddSubtab("Annotation", new PAnnotationColorbarWidget);

    // clang-format on
}

string WireFrameEventRouter::_getDescription() const { return ("Displays a wireframe of the mesh for the selected variable"); }
