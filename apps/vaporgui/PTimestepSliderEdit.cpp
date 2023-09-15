#include "PTimestepSliderEdit.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>
#include "VIntSliderEdit.h"
#include <vapor/AnimationParams.h>
#include <vapor/NavigationUtils.h>

// ===============================
//       PTimestepSliderEdit
// ===============================

PTimestepSliderEdit::PTimestepSliderEdit(VAPoR::ControlExec *ce) : PLineItem("", "Current Timestep", _sliderEdit = new VIntSliderEdit(0, 1, 0, false))
{
    _ce = ce;
    connect(_sliderEdit, &VIntSliderEdit::ValueChanged, this, &PTimestepSliderEdit::valueChanged);
}

void PTimestepSliderEdit::updateGUI() const
{
    auto      p = NavigationUtils::GetAnimationParams(_ce);
    const int start = p->GetStartTimestep();
    const int end = p->GetEndTimestep();
    const int ts = p->GetCurrentTimestep();

    _sliderEdit->SetMinimum(start);
    _sliderEdit->SetMaximum(end);
    _sliderEdit->SetValue(ts);
}

void PTimestepSliderEdit::valueChanged(int v) { NavigationUtils::SetTimestep(_ce, v); }
