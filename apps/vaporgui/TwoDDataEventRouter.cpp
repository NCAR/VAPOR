#include "TwoDDataEventRouter.h"
#include "vapor/TwoDDataParams.h"
#include "PWidgets.h"
#include "PMetadataClasses.h"

using namespace VAPoR;

static RenderEventRouterRegistrar<TwoDDataEventRouter> registrar(TwoDDataEventRouter::GetClassType());

TwoDDataEventRouter::TwoDDataEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, TwoDDataParams::GetClassType())
{
    // clang-format off

    AddVariablesSubtab(new PGroup({
        new PSection("Variable Selection", {
            new PScalarVariableSelector,
            new PHeightVariableSelector
        }),
        new PFidelitySection,
        new POpenVariableMetadataWidget
    }));
    
    AddAppearanceSubtab(new PGroup({
        new PTFEditor,
        new PCheckbox(RenderParams::DrawInFrontTag, "Draw renderer in front")
    }));
    AddGeometrySubtab(new PGeometrySubtab);
    AddColorbarSubtab(new PAnnotationColorbarWidget);

    // clang-format on
}

string TwoDDataEventRouter::_getDescription() const
{
    return ("Displays "
            "the user's 2D data variables along the plane described by the source data "
            "file.\n\nThese 2D variables may be offset by a height variable.\n\n");
}
