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
        new PCheckbox(ParticleParams::Render3DTag,"Enable 3D geometry"),
        (new PShowIf(ParticleParams::Render3DTag))->Equals(true)->Then({
            new PSection("Particles", {
                (new PDoubleSliderEdit(ParticleParams::RenderRadiusScalarTag, "Radius"))->SetRange(0.5, 25)->AllowUserRange(true)->EnableDynamicUpdate(),
                new PCheckbox(ParticleParams::ShowDirectionTag, "Show direction"),
                (new PShowIf(ParticleParams::ShowDirectionTag))->Equals(true)->Then({
                    (new PDoubleSliderEdit(ParticleParams::DirectionScaleTag, "Length scale"))->SetRange(0.0001, 10)->AllowUserRange(true)->EnableDynamicUpdate(),
                    (new PXFieldVariableSelector)->ShowParticleVars(),
                    (new PYFieldVariableSelector)->ShowParticleVars(),
                    (new PZFieldVariableSelector)->ShowParticleVars(),
                }),
            }),
            new PCheckbox(ParticleParams::LightingEnabledTag,"Enable Lighting"),
            (new PShowIf(ParticleParams::LightingEnabledTag))->Equals(true)->Then({
                new PSection("Lighting", {
                    (new PDoubleSliderEdit(ParticleParams::PhongAmbientTag,   "Ambient"  ))->EnableDynamicUpdate(),
                    (new PDoubleSliderEdit(ParticleParams::PhongDiffuseTag,   "Diffuse"  ))->EnableDynamicUpdate(),
                    (new PDoubleSliderEdit(ParticleParams::PhongSpecularTag,  "Specular" ))->EnableDynamicUpdate(),
                    (new PDoubleSliderEdit(ParticleParams::PhongShininessTag, "Shininess"))->EnableDynamicUpdate()
                })
            }),
        }),
    }));
    
    AddGeometrySubtab(new PGeometrySubtab);
    AddAnnotationSubtab(new PAnnotationColorbarWidget);

    // clang-format on
}

string ParticleEventRouter::_getDescription() const { return ("Render particle data"); }
