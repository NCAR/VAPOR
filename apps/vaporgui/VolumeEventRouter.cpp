#include "VolumeEventRouter.h"
#include "vapor/VolumeParams.h"
#include <vapor/VolumeOSPRay.h>
#include "PWidgets.h"
#include "PStringDropdownHLI.h"

using namespace VAPoR;
typedef VolumeParams VP;

static RenderEventRouterRegistrar<VolumeEventRouter> registrar(VolumeEventRouter::GetClassType());

VolumeEventRouter::VolumeEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, VolumeParams::GetClassType())
{
    // clang-format off

    AddVariablesSubtab(new PGroup({
        new PSection("Variable Selection", {
            new PScalarVariableSelector,
            new PColorMapVariableSelector
        }),
        new PFidelitySection
    }));
    
    AddAppearanceSubtab(new PGroup({
        (new PTFEditor)->ShowColormapBasedOnParam(VP::UseColormapVariableTag, false),
        new PSection("Rendering Method", {
            new PStringDropdownHLI<VP>("Raytracing Algorithm", VP::GetAlgorithmNames(VP::Type::DVR), &VP::GetAlgorithm, &VP::SetAlgorithmByUser),
        }),
        (new PShowIf(VP::_algorithmTag))->Equals(VolumeOSPRay::GetName())->Then({
            new PSection("OSPRay Parameters", {
                (new PDoubleSliderEdit(VolumeParams::OSPDensity, "Density"))->SetRange(0, 3)->EnableDynamicUpdate()->SetTooltip("Volume density (aka opacity)."),
                (new PIntegerSliderEdit("osp_spp", "Samples Per Pixel"))->SetRange(1, 10)->SetTooltip("Number of render passes. Increases fidelity but may significantly reduce performance."),
                (new PDoubleInput(VolumeParams::OSPSampleRateScalar, "Volume Sample Rate Scalar"))->EnableBasedOnParam("osp_usePT", false)->SetTooltip("Scales the sampling rate along the ray throughout the volume. Increasing may significantly reduce performance."), })
        })->Else({
            new PSection("Ray Tracing", {
                new PEnumDropdown(VP::SamplingRateMultiplierTag, {"1x", "2x", "4x", "8x", "16x"}, {1, 2, 4, 8, 16}, "Sampling Rate Multiplier"),
                (new PDoubleSliderEdit(VP::VolumeDensityTag, "Volume Density"))->EnableDynamicUpdate()->SetTooltip("Changes the overall density or 'opacity' of the volume allowing for finer tuning of the transfer function."),
                new PCheckbox(VP::UseColormapVariableTag, "Color by other variable"),
            }),
            (new PColormapTFEditor)->ShowBasedOnParam(VolumeParams::UseColormapVariableTag),
            new PSection("Lighting", {
                new PCheckbox(VolumeParams::LightingEnabledTag, "Enabled"),
                (new PDoubleSliderEdit(VP::PhongAmbientTag,   "Ambient"  ))->EnableDynamicUpdate(),
                (new PDoubleSliderEdit(VP::PhongDiffuseTag,   "Diffuse"  ))->EnableDynamicUpdate(),
                (new PDoubleSliderEdit(VP::PhongSpecularTag,  "Specular" ))->EnableDynamicUpdate(),
                (new PDoubleSliderEdit(VP::PhongShininessTag, "Shininess"))->EnableDynamicUpdate()
            })
        })
    }));
    
    AddGeometrySubtab(new PGeometrySubtab);
    AddAnnotationSubtab(new PAnnotationColorbarWidget);

    // clang-format on
}

string VolumeEventRouter::_getDescription() const
{
    return ("Displays "
            "the user's 3D data variables within a volume described by the source data "
            "file, according to color and opacity settings defined by the user.\n\n"
            "These 3D variables may be offset by a height variable.\n\n");
}
