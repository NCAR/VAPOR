#include "ParticleEventRouter.h"
#include "vapor/BarbParams.h"
#include "PWidgets.h"
#include "PConstantColorWidget.h"

using namespace VAPoR;
typedef BarbParams BP;

static RenderEventRouterRegistrar<ParticleEventRouter> registrar(ParticleEventRouter::GetClassType());

ParticleEventRouter::ParticleEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, BarbParams::GetClassType())
{
    // clang-format off

    AddVariablesSubtab(new PGroup({
        new PSection("Variable Selection", {
            new PScalarVariableSelector,
            new PColorMapVariableSelector,
        }),
        (new PFidelitySection)->Add(new PIntegerInput("stride", "Stride")),
        new PSection("Direction", {
            new PCheckbox("show_direction", "Show"),
            (new PDoubleSliderEdit("ns", "Scale"))->SetRange(0.0001, 10)->EnableDynamicUpdate(),
            new PVariableSelector("nx", "X"),
            new PVariableSelector("nx", "Y"),
            new PVariableSelector("nx", "Z"),
        }),
    }));
    
    AddAppearanceSubtab(new PGroup({
        new PColormapTFEditor,
    }));
    
    AddGeometrySubtab(new PGeometrySubtab);
    AddAnnotationSubtab(new PAnnotationColorbarWidget);

    // clang-format on
}

string ParticleEventRouter::_getDescription() const
{
    return ("Render particle data");
}
