#include "VolumeSubtabs.h"
#include "TFEditorVolume.h"
#include "QSliderEdit.h"
#include "PGroup.h"
#include "PSection.h"
#include "PTFEditor.h"
#include "PCheckbox.h"
#include "PStringDropdownHLI.h"
#include "PEnumDropdown.h"
#include "PSliderEdit.h"
#include "PVariableSelector.h"

using namespace VAPoR;

void VolumeVariablesSubtab::Update(DataMgr *dataMgr, ParamsMgr *paramsMgr, RenderParams *params)
{
    VolumeParams *vp = dynamic_cast<VolumeParams *>(params);
    _volumeParams = vp;
    VAssert(vp);

    _variablesWidget->Update(dataMgr, paramsMgr, params);
}

VolumeAppearanceSubtab::VolumeAppearanceSubtab(QWidget *parent)
{
    setupUi(this);

    verticalLayout->insertWidget(0, _pg = new PGroup);
    _pg->Add((new PTFEditor(RenderParams::_variableNameTag, {PTFEditor::Default}, "Transfer Function"))->ShowColormapBasedOnParam(VolumeParams::UseColormapVariableTag, false));
    _pg->Add(
        (new PTFEditor(RenderParams::_colorMapVariableNameTag, {PTFEditor::Histogram, PTFEditor::Colormap}, "Colormap Transfer Function"))->ShowBasedOnParam(VolumeParams::UseColormapVariableTag));

    PSection *rtp = new PSection("Volume Parameters");
    _pg->Add(rtp);
    rtp->Add(new PStringDropdownHLI<VolumeParams>("Raytracing Algorithm", VolumeParams::GetAlgorithmNames(VolumeParams::Type::DVR), &VolumeParams::GetAlgorithm, &VolumeParams::SetAlgorithmByUser));
    rtp->Add(new PEnumDropdown(VolumeParams::SamplingRateMultiplierTag, {"1x", "2x", "4x", "8x", "16x"}, {1, 2, 4, 8, 16}, "Sampling Rate Multiplier"));
    rtp->Add((new PDoubleSliderEdit(VolumeParams::VolumeDensityTag, "Volume Density"))
                 ->EnableDynamicUpdate()
                 ->SetTooltip("Changes the overall density or 'opacity' of the volume allowing for finer tuning of the transfer function."));
    rtp->Add(new PCheckbox(VolumeParams::UseColormapVariableTag, "Color by other variable"));
    rtp->Add((new PVariableSelector3D(RenderParams::_colorMapVariableNameTag))->ShowBasedOnParam(VolumeParams::UseColormapVariableTag));

    PSection *lp = new PSection("Lighting Parameters");
    _pg->Add(lp);
    lp->Add(new PCheckbox(VolumeParams::LightingEnabledTag));
    lp->Add((new PDoubleSliderEdit(VolumeParams::PhongAmbientTag, "Ambient"))->EnableDynamicUpdate());
    lp->Add((new PDoubleSliderEdit(VolumeParams::PhongDiffuseTag, "Diffuse"))->EnableDynamicUpdate());
    lp->Add((new PDoubleSliderEdit(VolumeParams::PhongSpecularTag, "Specular"))->EnableDynamicUpdate());
    lp->Add((new PDoubleSliderEdit(VolumeParams::PhongShininessTag, "Specular"))->SetRange(1, 100)->EnableDynamicUpdate());
}

void VolumeAppearanceSubtab::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) { _pg->Update(rParams, paramsMgr, dataMgr); }
