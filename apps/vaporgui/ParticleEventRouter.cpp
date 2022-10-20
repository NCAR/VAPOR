#include "ParticleEventRouter.h"
#include "vapor/BarbParams.h"
#include "PWidgets.h"
#include "PConstantColorWidget.h"
#include "PCheckbox.h"

using namespace VAPoR;

static RenderEventRouterRegistrar<ParticleEventRouter> registrar(ParticleEventRouter::GetClassType());

ParticleEventRouter::ParticleEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, BarbParams::GetClassType())
{
    // clang-format off

    AddVariablesSubtab(new PGroup({
        new PSection("Variable Selection", {
            (new PScalarVariableSelector)->ShowParticleVars(),
            (new PXFieldVariableSelector)->ShowParticleVars(),
            (new PYFieldVariableSelector)->ShowParticleVars(),
            (new PZFieldVariableSelector)->ShowParticleVars(),
        }),
        new PSection("Data Fidelity", {
            (new PIntegerInput(ParticleParams::StrideTag, "Stride"))->SetRange(1, 1000)
        }),
    }));

    AddAppearanceSubtab(new PGroup({
        new PTFEditor,
            new PSection("Particles", {
                (new PDoubleSliderEdit(ParticleParams::RenderRadiusScalarTag, "Radius"))->SetRange(0.5, 25)->AllowUserRange(true)->EnableDynamicUpdate()->EnableBasedOnParam(ParticleParams::RenderLegacyTag, false),
                new PCheckbox(ParticleParams::ShowDirectionTag, "Show direction"),
                    (new PDoubleSliderEdit(ParticleParams::DirectionScaleTag, "Length scale"))->SetRange(0.0001, 10)->AllowUserRange(true)->EnableDynamicUpdate()->EnableBasedOnParam(ParticleParams::ShowDirectionTag),
                    (new PXFieldVariableSelector)->ShowParticleVars()->EnableBasedOnParam(ParticleParams::ShowDirectionTag),
                    (new PYFieldVariableSelector)->ShowParticleVars()->EnableBasedOnParam(ParticleParams::ShowDirectionTag),
                    (new PZFieldVariableSelector)->ShowParticleVars()->EnableBasedOnParam(ParticleParams::ShowDirectionTag),
            }),
            new PSection("Lighting", {
                (new PCheckbox(ParticleParams::LightingEnabledTag,"Enable Lighting"))->EnableBasedOnParam(ParticleParams::RenderLegacyTag, false),
                (new PDoubleSliderEdit(ParticleParams::PhongAmbientTag,   "Ambient"  ))->EnableDynamicUpdate()->EnableBasedOnParam(ParticleParams::RenderLegacyTag, false),
                (new PDoubleSliderEdit(ParticleParams::PhongDiffuseTag,   "Diffuse"  ))->EnableDynamicUpdate()->EnableBasedOnParam(ParticleParams::RenderLegacyTag, false),
                (new PDoubleSliderEdit(ParticleParams::PhongSpecularTag,  "Specular" ))->EnableDynamicUpdate()->EnableBasedOnParam(ParticleParams::RenderLegacyTag, false),
                (new PDoubleSliderEdit(ParticleParams::PhongShininessTag, "Shininess"))->EnableDynamicUpdate()->EnableBasedOnParam(ParticleParams::RenderLegacyTag, false)
            }),
            new PSection("Legacy code", {
                new PCheckbox(ParticleParams::RenderLegacyTag,"Enable legacy renderer")
            }),
    }));
    
    AddGeometrySubtab(new PGeometrySubtab);
    AddAnnotationSubtab(new PAnnotationColorbarWidget);

    // clang-format on
}

string ParticleEventRouter::_getDescription() const { return ("Render particle data"); }
