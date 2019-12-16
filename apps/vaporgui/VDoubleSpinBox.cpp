#include "VDoubleSpinBox.h"

VDoubleSpinBox::VDoubleSpinBox(double min, double max) : VContainer()
{
    _spinBox = new QDoubleSpinBox;
    SetRange(min, max);
    SetValue(min);
    layout()->addWidget(_spinBox);

    connect( _spinBox, &QDoubleSpinBox::editingFinished,
        this, &VDoubleSpinBox::emitSpinBoxChanged;
}

void VDoubleSpinBox::SetValue(double value)
{
    _spinBox->blockSignals(true);
    _spinBox->setValue(value);
    _spinBox->blockSignals(false);
}

void VDoubleSpinBox::SetRange(double min, double max) { _spinBox->setRange(min, max); }

double VDoubleSpinBox::GetValue() const { return _spinBox->value(); }

void VDoubleSpinBox::emitSpinBoxChanged() { emit ValueChanged(GetValue()); }
