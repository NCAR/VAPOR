#include "VolumeIsoEventRouter.h"
#include <vapor/VolumeIsoParams.h>
#include <vapor/VolumeOSPRay.h>
#include "PWidgets.h"
#include "PStringDropdownHLI.h"

using namespace VAPoR;
typedef VolumeIsoParams VIP;

static RenderEventRouterRegistrar<VolumeIsoEventRouter> registrar(VolumeIsoEventRouter::GetClassType());

VolumeIsoEventRouter::VolumeIsoEventRouter( QWidget *parent, ControlExec *ce) 
: RenderEventRouterGUI(ce, VolumeIsoParams::GetClassType())
{
    AddSubtab("Variables", new PGroup({
        new PSection("Variable Selection", {
            new PScalarVariableSelector,
            new PColorMapVariableSelector
        }),
        new PFidelitySection
    }));
    
    AddSubtab("Appearance", new PGroup({
        new PTFEditor(RenderParams::_variableNameTag, {PTFEditor::Histogram, PTFEditor::Opacity, PTFEditor::IsoValues}),
        new PSection("Rendering Method", {
            new PStringDropdownHLI<VIP>("Raytracing Algorithm", VIP::GetAlgorithmNames(VIP::Type::Iso), &VIP::GetAlgorithm, &VIP::SetAlgorithmByUser),
        }),
        (new PShowIf(VIP::_algorithmTag))->Equals(VolumeOSPRayIso::GetName())->Then({
            new PSection("OSPRay Parameters", {
                (new PIntegerSliderEdit("osp_spp", "Samples Per Pixel"))->SetRange(1, 10)->SetTooltip("Number of render passes. Increases fidelity but may significantly reduce performance."),
                (new PDoubleSliderEdit(VIP::OSPAmbientLightIntensity, "Ambient Light"))->EnableDynamicUpdate(),
                (new PDoubleSliderEdit(VIP::OSPDirectionalLightIntensity, "Diffuse Light"))->SetRange(0, 3)->EnableDynamicUpdate(),
            })
        })->Else({
            new PSection("Ray Tracing", {
                new PEnumDropdown(VIP::SamplingRateMultiplierTag, {"1x", "2x", "4x", "8x", "16x"}, {1, 2, 4, 8, 16}, "Sampling Rate Multiplier"),
                new PCheckbox(VIP::UseColormapVariableTag, "Color by other variable"),
                (new PColorSelector(RenderParams::_constantColorTag, "Color"))->ShowBasedOnParam(VIP::UseColormapVariableTag, false),
                (new PVariableSelector3D(RenderParams::_colorMapVariableNameTag))->ShowBasedOnParam(VIP::UseColormapVariableTag)
            }),
            (new PColormapTFEditor)->ShowBasedOnParam(VIP::UseColormapVariableTag),
            new PSection("Lighting", {
                new PCheckbox(VIP::LightingEnabledTag, "Enabled"),
                (new PDoubleSliderEdit(VIP::PhongAmbientTag,   "Ambient" ))->EnableDynamicUpdate(),
                (new PDoubleSliderEdit(VIP::PhongDiffuseTag,   "Diffuse" ))->EnableDynamicUpdate(),
                (new PDoubleSliderEdit(VIP::PhongSpecularTag,  "Specular"))->EnableDynamicUpdate(),
                (new PDoubleSliderEdit(VIP::PhongShininessTag, "Shininess"))->SetRange(1, 100)->EnableDynamicUpdate()
            })
        })
    }));
    
    AddSubtab("Geometry", new PGeometrySubtab);
    AddSubtab("Annotation", new PAnnotationColorbarWidget);
}

string VolumeIsoEventRouter::_getDescription() const
{
    return( "Displays "
           "the user's 3D data variables within a volume described by the source data "
           "file, according to color and opacity settings defined by the user.\n\n"
           "These 3D variables may be offset by a height variable.\n\n");
}
