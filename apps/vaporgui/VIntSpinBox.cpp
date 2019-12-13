#include "VIntSpinBox.h"

VIntSpinBox::VIntSpinBox(int min, int max) : VContainer()
{
    _spinBox = new QSpinBox;
    SetRange(min, max);
    SetValue(min);
    layout()->addWidget(_spinBox);

    connect(_spinBox, &QSpinBox::editingFinished, this, &VIntSpinBox::emitSpinBoxChanged);
}

void VIntSpinBox::SetValue(int value)
{
    _spinBox->blockSignals(true);
    _spinBox->setValue(value);
    _spinBox->blockSignals(false);
}

void VIntSpinBox::SetRange(int min, int max) { _spinBox->setRange(min, max); }

int VIntSpinBox::GetValue() const { return _spinBox->value(); }

void VIntSpinBox::emitSpinBoxChanged() { emit ValueChanged(GetValue()); }
