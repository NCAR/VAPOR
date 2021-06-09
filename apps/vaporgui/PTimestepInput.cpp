#include "PTimestepInput.h"
#include "NavigationUtils.h"
#include "VIntSpinBox.h"
#include "AnimationParams.h"


PTimestepInput::PTimestepInput(VAPoR::ControlExec *ce)
: PWidget("", _input=new VIntSpinBox(0,1)), _ce(ce)
{
    connect(_input, &VIntSpinBox::ValueChanged, this, &PTimestepInput::inputChanged);
}


void PTimestepInput::updateGUI() const
{
    auto p = NavigationUtils::GetAnimationParams(_ce);
    const int start = p->GetStartTimestep();
    const int end = p->GetEndTimestep();
    const int ts = p->GetCurrentTimestep();
    
    _input->SetRange(start, end);
    _input->SetValue(ts);
}


void PTimestepInput::inputChanged(int v)
{
    NavigationUtils::SetTimestep(_ce, v);
}
