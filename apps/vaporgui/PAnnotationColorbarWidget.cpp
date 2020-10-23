#include "PAnnotationColorbarWidget.h"
#include "PSection.h"
#include "PCheckbox.h"
#include "PStringInput.h"
#include "PSliderEdit.h"
#include "PColorSelector.h"
#include <vapor/ColorbarPbase.h>
#include <vapor/RenderParams.h>

using VAPoR::ColorbarPbase;
using VAPoR::RenderParams;
typedef ColorbarPbase CBP;

PAnnotationColorbarWidget::PAnnotationColorbarWidget()
: PWidget("", _pSection = new PSection("Colorbar Settings", {
                                                                new PCheckbox(CBP::_colorbarEnabledTag, "Enabled"),
                                                                new PStringInput(CBP::_colorbarTitleTag, "Title"),
                                                                (new PDoubleSliderEdit(CBP::_colorbarPositionXTag, "X Position"))->EnableDynamicUpdate(),
                                                                (new PDoubleSliderEdit(CBP::_colorbarPositionYTag, "Y Position"))->EnableDynamicUpdate(),
                                                                (new PDoubleSliderEdit(CBP::_colorbarSizeXTag, "X Size"))->EnableDynamicUpdate(),
                                                                (new PDoubleSliderEdit(CBP::_colorbarSizeYTag, "Y Size"))->EnableDynamicUpdate(),
                                                                (new PIntegerSliderEdit(CBP::_colorbarNumDigitsTag, "Num. Decimals"))->SetRange(1, 8)->EnableDynamicUpdate(),
                                                                (new PIntegerSliderEdit(CBP::_colorbarNumTicksTag, "Num. Ticks"))->SetRange(1, 20)->EnableDynamicUpdate(),
                                                                (new PIntegerSliderEdit(CBP::_colorbarFontSizeTag, "Font Size"))->SetRange(1, 100)->EnableDynamicUpdate(),
                                                                new PColorSelector(CBP::_colorbarBackColorTag, "Background Color"),
                                                            }))
{
}

void PAnnotationColorbarWidget::updateGUI() const
{
    RenderParams * rp = getParams<RenderParams>();
    ColorbarPbase *cbp = rp->GetColorbarPbase();
    _pSection->Update(cbp, getParamsMgr(), getDataMgr());
}
