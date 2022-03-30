#include "PAnnotationColorbarWidget.h"
#include "PWidgets.h"
#include "PCornerSelector.h"
#include <vapor/ColorbarPbase.h>
#include <vapor/RenderParams.h>

using VAPoR::ColorbarPbase;
using VAPoR::RenderParams;
typedef ColorbarPbase CBP;

PAnnotationColorbarWidget::PAnnotationColorbarWidget() : PWidget("", _pSection = new PSection("Colorbar Settings"))
{
    // clang-format off
    _pSection->Add({
        new PColorbarCornerSelector,
        new PCheckbox(CBP::_colorbarEnabledTag, "Show Colorbar"),
        (new PDoubleSliderEdit(CBP::_colorbarPositionXTag, "X Position"))->EnableDynamicUpdate(),
        (new PDoubleSliderEdit(CBP::_colorbarPositionYTag, "Y Position"))->EnableDynamicUpdate(),
        (new PDoubleSliderEdit(CBP::_colorbarSizeYTag, "Height"))->EnableDynamicUpdate(),
        (new PDoubleSliderEdit(CBP::_colorbarSizeXTag, "Scale"))->EnableDynamicUpdate(),
        (new PIntegerSliderEdit(CBP::_colorbarNumTicksTag, "Ticks"))->SetRange(2, 20)->EnableDynamicUpdate(),
        new PStringInput(CBP::_colorbarTitleTag, "Title"),
        new PCheckbox(CBP::UseScientificNotationTag, "Scientific Notation"),
        (new PIntegerInput(CBP::_colorbarNumDigitsTag, "Significant Figures"))->SetRange(1, 7),
#ifdef MANUAL_FONT_CONTROL
        new PCheckbox("manual_font", "Manual Font Size"),
        (new PShowIf("manual_font"))->Equals(true)->Then(new PSubGroup({
            (new PIntegerInput(CBP::_colorbarFontSizeTag, "Font Size"))->SetRange(6, 80),
        })),
#endif
    });
    // clang-format on
}

void PAnnotationColorbarWidget::updateGUI() const
{
    RenderParams * rp = getParams<RenderParams>();
    ColorbarPbase *cbp = rp->GetColorbarPbase();
    _pSection->Update(cbp, getParamsMgr(), getDataMgr());
}
