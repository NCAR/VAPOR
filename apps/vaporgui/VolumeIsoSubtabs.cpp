#include "VolumeIsoSubtabs.h"
#include "VolumeSubtabs.h"
#include "PGroup.h"
#include "PSection.h"
#include "PTFEditor.h"
#include "PCheckbox.h"
#include "PStringDropdownHLI.h"
#include "PEnumDropdown.h"
#include "PSliderEdit.h"
#include "PColorSelector.h"
#include "PVariableWidgets.h"
#include "PFidelitySection.h"

using namespace VAPoR;

VolumeIsoVariablesSubtab::VolumeIsoVariablesSubtab(QWidget* parent) {
    setLayout( new QVBoxLayout );
    ((QVBoxLayout*)layout())->insertWidget(1, _pg = new PGroup);
    PSection *vars = new PSection("Variable Selection");
    vars->Add(new PScalarVariableSelectorHLI);
    vars->Add(new PColorMapVariableSelectorHLI);
    _pg->Add(vars);
    _pg->Add(new PFidelitySection);
}

void VolumeIsoVariablesSubtab::Update(
    VAPoR::DataMgr *dataMgr,
    VAPoR::ParamsMgr *paramsMgr,
    VAPoR::RenderParams *rParams
) {
    _pg->Update(rParams, paramsMgr, dataMgr);
}

VolumeIsoAppearanceSubtab::VolumeIsoAppearanceSubtab(QWidget* parent) 
{
    setupUi(this);
    
    _pg = new PGroup;
    
    verticalLayout->insertWidget(0, _pg);
    _pg->Add((new PTFEditor(RenderParams::_variableNameTag, {PTFEditor::Histogram, PTFEditor::IsoValues}, "Transfer Function")));
    _pg->Add((new PTFEditor(RenderParams::_colorMapVariableNameTag, {PTFEditor::Histogram, PTFEditor::Colormap}, "Colormap Transfer Function"))->ShowBasedOnParam(VolumeParams::UseColormapVariableTag));
    
    PSection *rtp = new PSection("Isosurface Parameters");
    _pg->Add(rtp);
    rtp->Add(new PStringDropdownHLI<VolumeIsoParams>("Raytracing Algorithm", VolumeIsoParams::GetAlgorithmNames(VolumeParams::Type::Iso), &VolumeIsoParams::GetAlgorithm, &VolumeIsoParams::SetAlgorithmByUser));
    rtp->Add(new PEnumDropdown(VolumeIsoParams::SamplingRateMultiplierTag, {"1x", "2x", "4x", "8x", "16x"}, {1, 2, 4, 8, 16}, "Sampling Rate Multiplier"));
    rtp->Add(new PCheckbox(VolumeIsoParams::UseColormapVariableTag, "Color by other variable"));
    rtp->Add((new PColorSelector(RenderParams::_constantColorTag, "Color"))->ShowBasedOnParam(VolumeIsoParams::UseColormapVariableTag, false));
    rtp->Add((new PVariableSelector3D(RenderParams::_colorMapVariableNameTag))->ShowBasedOnParam(VolumeIsoParams::UseColormapVariableTag));

    PSection *lp = new PSection("Lighting Parameters");
    _pg->Add(lp);
    lp->Add(new PCheckbox(VolumeIsoParams::LightingEnabledTag));
    lp->Add((new PDoubleSliderEdit(VolumeIsoParams::PhongAmbientTag,   "Ambient" ))->EnableDynamicUpdate());
    lp->Add((new PDoubleSliderEdit(VolumeIsoParams::PhongDiffuseTag,   "Diffuse" ))->EnableDynamicUpdate());
    lp->Add((new PDoubleSliderEdit(VolumeIsoParams::PhongSpecularTag,  "Specular"))->EnableDynamicUpdate());
    lp->Add((new PDoubleSliderEdit(VolumeIsoParams::PhongShininessTag, "Specular"))->SetRange(1, 100)->EnableDynamicUpdate());
}


void VolumeIsoAppearanceSubtab::Update( VAPoR::DataMgr      *dataMgr,
                                         VAPoR::ParamsMgr    *paramsMgr,
                                         VAPoR::RenderParams *params ) 
{
    _pg->Update(params, paramsMgr, dataMgr);
}
