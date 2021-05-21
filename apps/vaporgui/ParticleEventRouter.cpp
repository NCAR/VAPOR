#include "ParticleEventRouter.h"
#include "vapor/BarbParams.h"
#include "PWidgets.h"
#include "PConstantColorWidget.h"

using namespace VAPoR;

static RenderEventRouterRegistrar<ParticleEventRouter> registrar(ParticleEventRouter::GetClassType());

ParticleEventRouter::ParticleEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, BarbParams::GetClassType())
{
    // clang-format off

    AddVariablesSubtab(new PGroup({
        new PSection("Variable Selection", {
            new PScalarVariableSelector,
            new PXFieldVariableSelector,
            new PYFieldVariableSelector,
            new PZFieldVariableSelector,
        }),
        new PSection("Data Fidelity", {
            (new PIntegerInput(ParticleParams::StrideTag, "Stride"))->SetRange(1, 1000)
        }),
    }));
    
    AddAppearanceSubtab(new PGroup({
        new PTFEditor,
        new PSection("Direction", {
            new PCheckbox(ParticleParams::ShowDirectionTag, "Show"),
            (new PDoubleSliderEdit(ParticleParams::DirectionScaleTag, "Scale"))->SetRange(0.0001, 10)->EnableDynamicUpdate(),
            new PXFieldVariableSelector,
            new PYFieldVariableSelector,
            new PZFieldVariableSelector,
        }),
    }));
    
    AddGeometrySubtab(new PGeometrySubtab);
    AddAnnotationSubtab(new PAnnotationColorbarWidget);

    // clang-format on
}

string ParticleEventRouter::_getDescription() const { return ("Render particle data"); }
