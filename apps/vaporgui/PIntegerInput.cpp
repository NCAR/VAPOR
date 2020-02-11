#include "PIntegerInput.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>
#include "VIntSpinBox.h"

PIntegerInput::PIntegerInput(const std::string &tag, const std::string &label)
: PLineItem(tag, label, _spinbox = new VIntSpinBox(INT_MIN, INT_MAX))
{
    connect(_spinbox, &VIntSpinBox::ValueChanged, this, &PIntegerInput::spinboxValueChanged);
}

PIntegerInput *PIntegerInput::SetRange(int min, int max)
{
    _spinbox->SetRange(min, max);
    return this;
}

void PIntegerInput::updateGUI() const
{
    int value = getParams()->GetValueLong(GetTag(), 0);
    _spinbox->SetValue(value);
}

void PIntegerInput::spinboxValueChanged(int i)
{
    getParams()->SetValueLong(GetTag(), "", i);
}
