#include "TwoDDataEventRouter.h"
#include "vapor/TwoDDataParams.h"
#include "PWidgets.h"

using namespace VAPoR;

static RenderEventRouterRegistrar<TwoDDataEventRouter> registrar(TwoDDataEventRouter::GetClassType());

TwoDDataEventRouter::TwoDDataEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, TwoDDataParams::GetClassType())
{
    // clang-format off

    AddSubtab("Variables", new PGroup({
        new PSection("Variable Selection", {
            new PScalarVariableSelector,
            new PHeightVariableSelector
        }),
        new PFidelitySection
    }));
    
    AddSubtab("Appearance", (new PTFEditor));
    AddSubtab("Geometry", new PGeometrySubtab);
    AddSubtab("Annotation", new PAnnotationColorbarWidget);

    // clang-format on
}

string TwoDDataEventRouter::_getDescription() const
{
    return ("Displays "
            "the user's 2D data variables along the plane described by the source data "
            "file.\n\nThese 2D variables may be offset by a height variable.\n\n");
}
