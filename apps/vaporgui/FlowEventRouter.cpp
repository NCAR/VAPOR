#include "FlowEventRouter.h"
#include "vapor/FlowParams.h"
#include "PWidgets.h"
#include "PFlowRakeRegionSelector.h"
#include "PMultiVarSelector.h"

using namespace VAPoR;
typedef FlowParams FP;

static RenderEventRouterRegistrar<FlowEventRouter> registrar(FlowEventRouter::GetClassType());

FlowEventRouter::FlowEventRouter(QWidget *parent, ControlExec *ce)
: RenderEventRouterGUI(ce, FlowParams::GetClassType())
{
    AddSubtab("Variables", new PGroup({
        new PSection("Variable Selection", {
            new PDimensionSelector,
            new PXFieldVariableSelector,
            new PYFieldVariableSelector,
            (new PZFieldVariableSelector)->OnlyShowForDim(3),
            new PColorMapVariableSelector,
        }),
        new PFidelitySection
    }));
    
    _seedingTab =
    AddSubtab("Seeding", new PGroup({
        new PSection("Flow Integration Settings", {
            new PEnumDropdown(FP::_isSteadyTag, {"Streamlines", "Pathlines"}, {true, false}, "Flow Type"),
            (new PShowIf(FP::_isSteadyTag))->Equals(true)->Then({
                new PEnumDropdown(FP::_flowDirectionTag, {"Forward", "Backward", "Bi-Directional"}, {(int)FlowDir::FORWARD, (int)FlowDir::BACKWARD, (int)FlowDir::BI_DIR}, "Flow Direction"),
                (new PIntegerSliderEdit(FP::_steadyNumOfStepsTag, "Integration Steps"))->SetRange(0, 5000),
            })->Else({
                _pathlineLengthSlider = new PIntegerSliderEdit(FP::_pastNumOfTimeSteps, "Pathline Length"),
                _pathlineInjectionSlider = new PIntegerSliderEdit(FP::_seedInjInterval, "Injection Interval"),
            }),
            new PDoubleInput(FP::_velocityMultiplierTag, "Vector Field Multiplier"),
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
                (new PIntegerSliderEdit(FP::_randomNumOfSeedsTag, "Seed count"))->SetRange(1, 500),
            }),
            (new PShowIf(FP::_seedGenModeTag))->Equals((int)FlowSeedMode::RANDOM_BIAS)->Then({
                (new PIntegerSliderEdit(FP::_randomNumOfSeedsTag, "Seed count"))->SetRange(1, 500),
                (new PDoubleSliderEdit(FP::_rakeBiasStrength, "Bias weight"))->SetRange(-10, 10),
                new PVariableSelector(FP::_rakeBiasVariable)
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
        new PSection("Rake Region", {
            new PFlowRakeRegionSelector1D(0),
            new PFlowRakeRegionSelector1D(1),
            new PFlowRakeRegionSelector1D(2),
        }),
        
        new PSection("Write Flowlines to File", {
            new PFileSaveSelector(FP::_flowlineOutputFilenameTag, "Target file"),
            (new PButton("Write to file", [](ParamsBase *p){p->SetValueLong(FP::_needFlowlineOutputTag, "", true);}))->DisableUndo(),
            new PLabel("Specify variables to sample and output along the flowlines"),
            new PMultiVarSelector(FP::_flowOutputMoreVariablesTag)
        }),
    }));
    
    AddSubtab("Appearance", new PGroup({
        (new PTFEditor(RenderParams::_colorMapVariableNameTag))->ShowOpacityBasedOnParam("NULL", 1),
        new PSection("Appearance", {
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
#ifndef NDEBUG
        (new PSection("Debug", {
            new PCheckbox("old_render", "Old Render Code (Regressing Testing)")
        }))->SetTooltip("Only accessible in debug build."),
#endif
    }));

	AddSubtab("Geometry", new PGeometrySubtab);
    AddSubtab("Annotation", new PAnnotationColorbarWidget);
    
    connect(this, &QTabWidget::currentChanged, this, &FlowEventRouter::tabChanged);
}

void FlowEventRouter::_updateTab(){
    RenderEventRouterGUI::_updateTab();
    
    int numTS = GetActiveDataMgr()->GetNumTimeSteps();
    _pathlineLengthSlider   ->SetRange(0, std::max(1, numTS-1));
    _pathlineInjectionSlider->SetRange(0, numTS);
    
    syncOpenTabWithGUIStateParams();
}

string FlowEventRouter::_getDescription() const
{
	return "Computes and displays steady or unsteady flow trajectories.\n";
}

void FlowEventRouter::tabChanged(int i)
{
    GUIStateParams *gp = (GUIStateParams *)_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());
    bool setToSeedingTab = widget(i) == _seedingTab;
    gp->SetFlowSeedTabActive(setToSeedingTab);
}

void FlowEventRouter::syncOpenTabWithGUIStateParams()
{
    GUIStateParams *gp = (GUIStateParams *)_controlExec->GetParamsMgr()->GetParams(GUIStateParams::GetClassType());
    if (gp->IsFlowSeedTabActive()) {
        if (currentWidget() != _seedingTab) {
            blockSignals(true);
            setCurrentWidget(_seedingTab);
            blockSignals(false);
        }
    } else {
        if (currentWidget() == _seedingTab) {
            blockSignals(true);
            setCurrentIndex(1);
            blockSignals(false);
        }
    }
}
