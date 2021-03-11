#include "BarbEventRouter.h"
#include "vapor/BarbParams.h"
#include "PWidgets.h"
#include "PConstantColorWidget.h"

using namespace VAPoR;
typedef BarbParams BP;

static RenderEventRouterRegistrar<BarbEventRouter> registrar(BarbEventRouter::GetClassType());

BarbEventRouter::BarbEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, BarbParams::GetClassType())
{
    // clang-format off

    AddVariablesSubtab(new PGroup({
        new PSection("Variable Selection", {
            new PDimensionSelector,
            new PXFieldVariableSelector,
            new PYFieldVariableSelector,
            new PZFieldVariableSelector,
            new PHeightVariableSelector,
            new PColorMapVariableSelector,
        }),
        new PFidelitySection
    }));
    
    AddAppearanceSubtab(new PGroup({
        new PColormapTFEditor,
        new PSection("Barbs", {
            (new PIntegerSliderEdit(BP::_xBarbsCountTag, "X Barbs"))->SetRange(1, 50),
            (new PIntegerSliderEdit(BP::_yBarbsCountTag, "Y Barbs"))->SetRange(1, 50),
            (new PIntegerSliderEdit(BP::_zBarbsCountTag, "Z Barbs"))->SetRange(1, 50),
            (new PDoubleSliderEdit(BP::_lengthScaleTag, "Length Scale"))->SetRange(0.01, 4)->EnableDynamicUpdate(),
            (new PDoubleSliderEdit(BP::_thicknessScaleTag, "Thickness Scale"))->SetRange(0.01, 4)->EnableDynamicUpdate(),
            new PConstantColorWidget
        }),
    }));
    
    AddGeometrySubtab(new PGeometrySubtab);
    AddAnnotationSubtab(new PAnnotationColorbarWidget);

    // clang-format on
}

string BarbEventRouter::_getDescription() const
{
    return ("Displays an "
            "array of arrows with the users domain, with custom dimensions that are "
            "defined by the user in the X, Y, and Z axes.  The arrows represent a vector "
            "whos direction is determined by up to three user-defined variables.\n\nBarbs "
            "can have a constant color applied to them, or they may be colored according "
            "to an additional user-defined variable.\n\n");
}
