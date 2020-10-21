#include "VolumeEventRouter.h"
#include "vapor/VolumeParams.h"
#include "PWidgets.h"
#include "PStringDropdownHLI.h"

using namespace VAPoR;
typedef VolumeParams VP;

static RenderEventRouterRegistrar<VolumeEventRouter> registrar(VolumeEventRouter::GetClassType());

VolumeEventRouter::VolumeEventRouter(QWidget *parent, ControlExec *ce)
: RenderEventRouterGUI(ce, VolumeParams::GetClassType())
{
    AddSubtab("Variables", new PGroup({
        new PSection("Variable Selection", {
            new S::PScalarVariableSelector,
            new S::PColorMapVariableSelector
        }),
        new PFidelitySection
    }));
    
    AddSubtab("Appearance", new PGroup({
        (new PTFEditor)->ShowColormapBasedOnParam(VP::UseColormapVariableTag, false),
        new PSection("Ray Tracing", {
            new PStringDropdownHLI<VP>("Raytracing Algorithm", VP::GetAlgorithmNames(VP::Type::DVR), &VP::GetAlgorithm, &VP::SetAlgorithmByUser),
            new PEnumDropdown(VP::SamplingRateMultiplierTag, {"1x", "2x", "4x", "8x", "16x"}, {1, 2, 4, 8, 16}, "Sampling Rate Multiplier"),
            (new PDoubleSliderEdit(VP::VolumeDensityTag, "Volume Density"))->EnableDynamicUpdate()->SetTooltip("Changes the overall density or 'opacity' of the volume allowing for finer tuning of the transfer function."),
            new PCheckbox(VP::UseColormapVariableTag, "Color by other variable"),
            (new S::PVariableSelector3D(RenderParams::_colorMapVariableNameTag))->ShowBasedOnParam(VP::UseColormapVariableTag)
        }),
        (new PColormapTFEditor)->ShowBasedOnParam(VolumeParams::UseColormapVariableTag),
        new PSection("Lighting", {
            new PCheckbox(VolumeParams::LightingEnabledTag, "Enabled"),
            (new PDoubleSliderEdit(VP::PhongAmbientTag,   "Ambient" ))->EnableDynamicUpdate(),
            (new PDoubleSliderEdit(VP::PhongDiffuseTag,   "Diffuse" ))->EnableDynamicUpdate(),
            (new PDoubleSliderEdit(VP::PhongSpecularTag,  "Specular"))->EnableDynamicUpdate(),
            (new PDoubleSliderEdit(VP::PhongShininessTag, "Shininess"))->SetRange(1, 100)->EnableDynamicUpdate()
        })
    }));
    
    AddSubtab("Geometry", new PGeometrySubtab);
    AddSubtab("Annotation", new PAnnotationColorbarWidget);
}

string VolumeEventRouter::_getDescription() const
{
    return( "Displays "
           "the user's 3D data variables within a volume described by the source data "
           "file, according to color and opacity settings defined by the user.\n\n"
           "These 3D variables may be offset by a height variable.\n\n");
}
