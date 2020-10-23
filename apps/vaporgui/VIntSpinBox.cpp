#include "VIntSpinBox.h"

VIntSpinBox::VIntSpinBox(int min, int max) : VHBoxWidget()
{
    _spinBox = new QSpinBox;
    SetRange(min, max);
    SetValue(min);
    layout()->addWidget(_spinBox);

    connect(_spinBox, &QSpinBox::editingFinished, this, &VIntSpinBox::emitSpinBoxFinished);

    // QSpinBox overloads valueChanged.  This makes the function pointer
    // based syntax for signal and slot connection ugly, so defer to
    // SIGNAL/SLOT connections with data type specificaiton.
    //
    // More info here: https://doc.qt.io/qt-5/qspinbox.html#valueChanged
    connect(_spinBox, SIGNAL(valueChanged(int)), this, SLOT(emitSpinBoxChanged(int)));
}

void VIntSpinBox::SetValue(int value)
{
    _spinBox->blockSignals(true);
    _spinBox->setValue(value);
    _spinBox->blockSignals(false);
}

void VIntSpinBox::SetRange(int min, int max)
{
    blockSignals(true);
    _spinBox->setRange(min, max);
    blockSignals(false);
}

int VIntSpinBox::GetValue() const { return _spinBox->value(); }

void VIntSpinBox::emitSpinBoxFinished() { emit ValueChanged(GetValue()); }

void VIntSpinBox::emitSpinBoxChanged(int value) { emit ValueChanged(value); }
