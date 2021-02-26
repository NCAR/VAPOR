#include <QLineEdit>
#include "VIntSpinBox.h"
#include <iostream>
VIntSpinBox::VIntSpinBox(int min, int max) : VHBoxWidget(), _value(min)
{
    _spinBox = new QSpinBox;
    SetRange(min, max);
    SetValue(min);
    layout()->addWidget(_spinBox);

    // Emit when the spinbox loses focus, or when return is pressed
    // Note: when opening a context menu with right click, a QSpinBox will emit the editingFinished signal,
    // due to the QLineEdit receiving focus upon click, and losing focus upon opening the menu.
    // In the VIntSpinBox's slot, we must therefore check if the currently held value has changed before emitting.
    connect(_spinBox, &QSpinBox::editingFinished, this, &VIntSpinBox::emitValueChanged);

    // QSpinBox overloads valueChanged.  This makes the function pointer
    // based syntax for signal and slot connection ugly, so defer to
    // SIGNAL/SLOT connections with data type specificaiton.
    //
    // More info here: https://doc.qt.io/qt-5/qspinbox.html#valueChanged
    connect(_spinBox, SIGNAL(valueChanged(int)), this, SLOT(emitValueChangedIntermediate(int)));
}

void VIntSpinBox::SetValue(int value)
{
    _value = value;
    _spinBox->blockSignals(true);
    _spinBox->setValue(_value);
    _spinBox->blockSignals(false);
}

void VIntSpinBox::SetRange(int min, int max)
{
    blockSignals(true);
    _spinBox->setRange(min, max);
    blockSignals(false);
}

int VIntSpinBox::GetValue() const { return _spinBox->value(); }

void VIntSpinBox::emitValueChanged()
{
    int value = GetValue();
    if (value != _value) {
        _value = value;
        emit ValueChanged(_value);
    }
}

void VIntSpinBox::emitValueChangedIntermediate(int value) { emit ValueChangedIntermediate(value); }
