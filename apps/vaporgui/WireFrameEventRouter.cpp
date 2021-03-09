#include "WireFrameEventRouter.h"
#include "vapor/WireFrameParams.h"
#include "PWidgets.h"
#include "PConstantColorWidget.h"

using namespace VAPoR;

static RenderEventRouterRegistrar<WireFrameEventRouter> registrar(WireFrameEventRouter::GetClassType());

WireFrameEventRouter::WireFrameEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, WireFrameParams::GetClassType())
{
    // clang-format off

    AddVariablesSubtab(new PGroup({
        new PSection("Variable Selection", {
            new PDimensionSelector,
            new PScalarVariableSelector,
            new PHeightVariableSelector
        }),
        new PFidelitySection
    }));
    
    AddAppearanceSubtab(new PGroup({
        (new PTFEditor)->ShowOpacityBasedOnParam("NULL", 1),
        new PSection("Appearance", {
            new PConstantColorWidget
        })
    }));
    
    AddGeometrySubtab(new PGeometrySubtab);
    AddAnnotationSubtab(new PAnnotationColorbarWidget);

    // clang-format on
}

string WireFrameEventRouter::_getDescription() const { return ("Displays a wireframe of the mesh for the selected variable"); }
