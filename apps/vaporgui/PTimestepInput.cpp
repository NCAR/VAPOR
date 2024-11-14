#include "PTimestepInput.h"
#include <vapor/NavigationUtils.h>
#include "VIntSpinBox.h"
#include <vapor/AnimationParams.h>


PTimestepInput::PTimestepInput(VAPoR::ControlExec *ce)
: PWidget("", _input = new VIntSpinBox(0, 1)), _ce(ce)
{
    connect(_input, &VIntSpinBox::ValueChangedIntermediate, this, &PTimestepInput::inputChanged);
    _input->setMinimumWidth(40);
}


void PTimestepInput::updateGUI() const
{
    auto      p = NavigationUtils::GetAnimationParams(_ce);
    const int start = p->GetStartTimestep();
    const int end = p->GetEndTimestep();
    const int ts = p->GetCurrentTimestep();

//    int end = _ce->GetDataStatus()->GetTimeCoordinates().size() - 1;

    _input->SetRange(start, end);
    _input->SetValue(ts);
}


void PTimestepInput::inputChanged(int v) {
    NavigationUtils::SetTimestep(_ce, v);
}
