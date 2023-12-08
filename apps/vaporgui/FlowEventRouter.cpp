#include "FlowEventRouter.h"
#include "vapor/FlowParams.h"
#include "PWidgets.h"
#include "PFlowRakeRegionSelector.h"
#include "PFlowIntegrationRegionSelector.h"
#include "PMultiVarSelector.h"
#include "PConstantColorWidget.h"
#include "PMetadataClasses.h"
#include "PSliderEditHLI.h"

using namespace VAPoR;
typedef FlowParams FP;

static RenderEventRouterRegistrar<FlowEventRouter> registrar(FlowEventRouter::GetClassType());
const string                                       FlowEventRouter::SeedingTabName = "Seeding";
const string                                       FlowEventRouter::IntegrationTabName = "Integration";

FlowEventRouter::FlowEventRouter(QWidget *parent, ControlExec *ce) : RenderEventRouterGUI(ce, FlowParams::GetClassType())
{
    // clang-format off

    AddVariablesSubtab(new PGroup({
        new PSection("Variable Selection", {
            new PDimensionSelector,
            new PXFieldVariableSelector,
            new PYFieldVariableSelector,
            (new PZFieldVariableSelector)->OnlyShowForDim(3),
            new PColorMapVariableSelector,
        }),
        new PFidelitySection,
        new POpenVariableMetadataWidget
    }));
    
    _seedingTab =
    AddSubtab(SeedingTabName, new PGroup({
        new PSection("Flow Integration Settings", {
            new PEnumDropdown(FP::_isSteadyTag, {"Streamlines", "Pathlines"}, {true, false}, "Flow Type"),
            (new PShowIf(FP::_isSteadyTag))->Equals(true)->Then({
                new PEnumDropdown(FP::_flowDirectionTag, {"Forward", "Backward", "Bi-Directional"}, {(int)FlowDir::FORWARD, (int)FlowDir::BACKWARD, (int)FlowDir::BI_DIR}, "Flow Direction"),
                (new PIntegerInput(FP::_steadyNumOfStepsTag, "Integration Steps"))->SetRange(0, 10000),
            })->Else({
                _pathlineLengthSlider = new PIntegerSliderEdit(FP::_pastNumOfTimeSteps, "Pathline Length"),
                _pathlineInjectionSlider = new PIntegerSliderEdit(FP::_seedInjInterval, "Injection Interval"),
            }),
            new PCheckbox(FP::_xPeriodicTag, "X axis periodicity"),
            new PCheckbox(FP::_yPeriodicTag, "Y axis periodicity"),
            new PCheckbox(FP::_zPeriodicTag, "Z axis periodicity"),
        }),
        new PSection("Seed Distribution Settings", {
            new PEnumDropdown(FP::_seedGenModeTag, {"Gridded", "Random", "Random w/ Bias", "List of seeds"}, {(int)FlowSeedMode::UNIFORM, (int)FlowSeedMode::RANDOM, (int)FlowSeedMode::RANDOM_BIAS, (int)FlowSeedMode::LIST}, "Seed distribution type"),
            (new PShowIf(FP::_seedGenModeTag))->Equals((int)FlowSeedMode::UNIFORM)->Then({
                (new PIntegerSliderEdit(FP::_xGridNumOfSeedsTag, "X axis seeds"))->SetRange(1, 50),
                (new PIntegerSliderEdit(FP::_yGridNumOfSeedsTag, "Y axis seeds"))->SetRange(1, 50),
                (new PIntegerSliderEdit(FP::_zGridNumOfSeedsTag, "Z axis seeds"))->SetRange(1, 50),
            }),
            (new PShowIf(FP::_seedGenModeTag))->Equals((int)FlowSeedMode::RANDOM)->Then({
                (new PIntegerSliderEdit(FP::_randomNumOfSeedsTag, "Seed count"))->SetRange(1, 2500)
                 ->AllowUserRange(true),
            }),
            (new PShowIf(FP::_seedGenModeTag))->Equals((int)FlowSeedMode::RANDOM_BIAS)->Then({
                (new PIntegerSliderEdit(FP::_randomNumOfSeedsTag, "Seed count"))->SetRange(1, 2500)
                     ->AllowUserRange(true),
                (new PIntegerSliderEdit(FP::_rakeBiasStrength, "Bias weight"))->SetRange(-10000, 10000)
                     ->AllowUserRange(true),
                new PVariableSelector(FP::_rakeBiasVariable, "Bias Variable")
            }),
            (new PShowIf(FP::_seedGenModeTag))->Equals((int)FlowSeedMode::LIST)->Then({
                (new PFileOpenSelector(FP::_seedInputFilenameTag, "List of seeds file"))->SetTooltip(
                    "Seed injection points within your domain may be defined in this text file with the following definition: \n"
                    "1. Each line specifies the location of one seed. \n"
                    "2. Empty lines and lines starting with a pound sign is ignored. \n"
                    "3. Seed locations are specified using comma separated X,Y,Z coordinates. \n"
                    "4. A line with less than 3 comma separated values is invalid, which also invalidates the entire file. \n"
                    "5. A line can have more than 3 comma separated values, with additional values being ignored. \n"
                    "6. X, Y, Z coordinates use the same unit of the dataset's spatial domain. \n"
                    "    Note: lat-lon coordinates may be converted to meters via a map projection. \n"
                    "Finally, the listOfSeeds.txt demo file provides a starting point to specify your own seeds."
                )
            }),
        }),
        (new PShowIf(FP::_seedGenModeTag))->Not()->Equals((int)FlowSeedMode::LIST)->Then({
            new PSection("Rake Region", {
                new PFlowRakeRegionSelector1D(0),
                new PFlowRakeRegionSelector1D(1),
                new PFlowRakeRegionSelector1D(2),
            }),
            (new PSection("Rake Center", {
                (_xRakeCenterSlider = new PDoubleSliderEditHLI<FlowParams>("X", &FlowParams::GetXRakeCenter, &FlowParams::SetXRakeCenter))->AllowDynamicUpdate(),
                (_yRakeCenterSlider = new PDoubleSliderEditHLI<FlowParams>("Y", &FlowParams::GetYRakeCenter, &FlowParams::SetYRakeCenter))->AllowDynamicUpdate(),
                (_zRakeCenterSlider = new PDoubleSliderEditHLI<FlowParams>("Z", &FlowParams::GetZRakeCenter, &FlowParams::SetZRakeCenter))->AllowDynamicUpdate(),
            }))->SetTooltip("Controls the location of the Rake's Center.  To control\n the range of values that the Rake Center can traverse, adjust\n the Flow Renderer's Region in the Geometry tab"),
        }),
        
        new PSection("Write Flowlines to File", {
            new PFileSaveSelector(FP::_flowlineOutputFilenameTag, "Target file"),
            (new PButton("Write to file", [](ParamsBase *p){p->SetValueLong(FP::_needFlowlineOutputTag, "", true);}))->DisableUndo(),
            new PLabel("Specify variables to sample and output along the flowlines"),
            new PMultiVarSelector(FP::_flowOutputMoreVariablesTag)
        }),

        new PSection("Advanced Options", {
            (new PDoubleInput(FP::_velocityMultiplierTag, "Vector Field Multiplier"))->SetTooltip( "Apply a multiplier to the velocity field."),
            (new PDoubleInput(FP::_firstStepSizeMultiplierTag, "First Step Size Multiplier"))->SetTooltip( "Apply a multiplier to the auto-calculated first step size. Very occasionally a value bigger than 1.0 is needed here."),
            (new PCheckbox(FP::_fixedAdvectionStepTag, "Use Fixed Advection Steps"))->SetTooltip( "The user may provide an advection step size, so that VAPOR disables dynamic step size adjustments and always uses the fixed step size."),
            (new PShowIf(FP::_fixedAdvectionStepTag)->Then(new PDoubleInput(FP:_fixedAdvectionSizeTag, "Fixed Advection Step Size"))->SetTooltip( "Use this specific value as the fixed advection step size.")),
        }),
    }));
    
    AddAppearanceSubtab(new PGroup({
        (new PTFEditor(RenderParams::_colorMapVariableNameTag)),
        new PSection("Appearance", {
            new PConstantColorWidget,
            new PEnumDropdown(FP::RenderTypeTag, {"Tubes", "Samples", "KLGWTH"}, {FP::RenderTypeStream, FP::RenderTypeSamples, FP::RenderTypeDensity}, "Render Type"),
            (new PShowIf(FP::RenderTypeTag))->Equals(FP::RenderTypeStream)->Then({
                new PCheckbox(FP::RenderGeom3DTag, "3D Geometry"),
                (new PDoubleSliderEdit(FP::RenderRadiusScalarTag, "Radius Scalar"))->SetRange(0.1, 5)->EnableDynamicUpdate(),
                new PCheckbox(FP::RenderShowStreamDirTag, "Show Stream Direction"),
                (new PSubGroup({(new PIntegerSliderEdit(FP::RenderGlyphStrideTag, "Every N Samples"))->SetRange(1, 20)->EnableDynamicUpdate()}))->ShowBasedOnParam(FP::RenderShowStreamDirTag),
            }),
            (new PShowIf(FP::RenderTypeTag))->Equals(FP::RenderTypeSamples)->Then({
                new PEnumDropdown(FP::RenderGlyphTypeTag, {"Circle", "Arrow"}, {FP::GlpyhTypeSphere, FP::GlpyhTypeArrow}, "Glyph Type"),
                new PCheckbox(FP::RenderGeom3DTag, "3D Geometry"),
                (new PDoubleSliderEdit(FP::RenderRadiusScalarTag, "Radius Scalar"))->SetRange(0.1, 5)->EnableDynamicUpdate(),
                (new PIntegerSliderEdit(FP::RenderGlyphStrideTag, "Every N Samples"))->SetRange(1, 20)->EnableDynamicUpdate(),
                new PCheckbox(FP::RenderGlyphOnlyLeadingTag, "Only Show Leading Sample"),
            }),
            (new PShowIf(FP::RenderTypeTag))->Equals(FP::RenderTypeDensity)->Then({
                new PLabel("May not render correctly with other renderers"),
                (new PDoubleSliderEdit(FP::RenderRadiusScalarTag, "Radius Scalar"))->SetRange(0.1, 5)->EnableDynamicUpdate(),
                (new PDoubleSliderEdit(FlowParams::RenderDensityFalloffTag, "Density Falloff"))->SetRange(0.5, 10)->EnableDynamicUpdate()->SetTooltip("The exponential factor at which the intensity falls off along the width of the line"),
                (new PDoubleSliderEdit(FlowParams::RenderDensityToneMappingTag, "Tone Mapping"))->SetRange(0, 1)->EnableDynamicUpdate()->SetTooltip("The overall color intensity of the line"),
                (new PCheckbox("Invert"))->SetTooltip("For rendering on light backgrounds"),
            }),
            (new PShowIf(FP::RenderTypeTag))->Not()->Equals(FP::RenderTypeSamples)->Then({
                new PCheckbox(FP::RenderFadeTailTag, "Fade Flow Tails"),
                (new PShowIf(FP::RenderFadeTailTag))->Equals(true)->Then(new PSubGroup({
                    (new PIntegerSliderEdit(FP::RenderFadeTailStartTag, "Fade Start Sample"))->SetRange(0, 100)->EnableDynamicUpdate()->SetTooltip("How far behind leading sample fade begins."),
                    (new PIntegerSliderEdit(FP::RenderFadeTailLengthTag, "Fade Over N Samples"))->SetRange(1, 100)->EnableDynamicUpdate()->SetTooltip("Number of samples from opaque to transparent."),
                    (new PIntegerSliderEdit(FP::RenderFadeTailStopTag, "Animate Steady"))->SetRange(0, 200)->EnableDynamicUpdate()->SetTooltip("Temporary solution for animating steady flow particles."),
                })),
            }),
        }),
        (new PShowIf(FP::RenderGeom3DTag))->Then(new PSection("Lighting", {
            (new PDoubleSliderEdit(FP::PhongAmbientTag,   "Ambient"  ))->EnableDynamicUpdate(),
            (new PDoubleSliderEdit(FP::PhongDiffuseTag,   "Diffuse"  ))->EnableDynamicUpdate(),
            (new PDoubleSliderEdit(FP::PhongSpecularTag,  "Specular" ))->EnableDynamicUpdate(),
            (new PDoubleSliderEdit(FP::PhongShininessTag, "Shininess"))->EnableDynamicUpdate(),
        })),
#ifndef NDEBUG
        (new PSection("Debug", {
            new PCheckbox("old_render", "Old Render Code (Regressing Testing)")
        }))->SetTooltip("Only accessible in debug build."),
#endif
    }));
    
    AddSubtab(IntegrationTabName, new PGroup({
        new PSection("Integration Settings", {
            new PCheckbox(FP::_doIntegrationTag, "Integrate particle values along trajectory"),
            new PCheckbox(FP::_integrationSetAllToFinalValueTag, "Set entire stream value to integrated total"),
            (new PVariableSelector(RenderParams::_colorMapVariableNameTag, "Scalar to Integrate"))->EnableBasedOnParam(FP::_doIntegrationTag),
            (new PDoubleInput(FP::_integrationScalarTag, "Integration distance scale"))->EnableBasedOnParam(FP::_doIntegrationTag),
        }),
        (new PSection("Integration Region", {
            new PFlowIntegrationRegionSelector1D(0),
            new PFlowIntegrationRegionSelector1D(1),
            new PFlowIntegrationRegionSelector1D(2),
        }))->EnableBasedOnParam(FP::_doIntegrationTag),
    }));

    AddGeometrySubtab(new PGeometrySubtab);
    AddAnnotationSubtab(new PAnnotationColorbarWidget);

    // clang-format on
}

void FlowEventRouter::_updateTab()
{
    RenderEventRouterGUI::_updateTab();

    int numTS = GetActiveDataMgr()->GetNumTimeSteps();
    _pathlineLengthSlider->SetRange(0, std::max(1, numTS - 1));
    _pathlineInjectionSlider->SetRange(0, numTS);

    VAPoR::CoordType min, max;
    FlowParams *        fp = dynamic_cast<FlowParams *>(GetActiveParams());
    fp->GetBox()->GetExtents(min, max);
    double xCenter = fp->GetXRakeCenter();
    double yCenter = fp->GetYRakeCenter();
    double zCenter = fp->GetZRakeCenter();

    if (xCenter > max[0]) fp->SetXRakeCenter(max[0]);
    if (xCenter < min[0]) fp->SetXRakeCenter(min[0]);
    _xRakeCenterSlider->SetRange(min[0], max[0]);

    if (yCenter > max[1]) fp->SetYRakeCenter(max[1]);
    if (yCenter < min[1]) fp->SetYRakeCenter(min[1]);
    _yRakeCenterSlider->SetRange(min[1], max[1]);

    if (zCenter > max[2]) fp->SetZRakeCenter(max[2]);
    if (zCenter < min[2]) fp->SetZRakeCenter(min[2]);
    _zRakeCenterSlider->SetRange(min[2], max[2]);
}

string FlowEventRouter::_getDescription() const { return "Computes and displays steady or unsteady flow trajectories.\n"; }
