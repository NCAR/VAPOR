#include "VRadioButton.h"
#include <QRadioButton>

VRadioButton::VRadioButton(bool checked) : VHBoxWidget()
{
    _radioButton = new QRadioButton;
    SetValue(checked);
    layout()->addWidget(_radioButton);

    connect(_radioButton, &QRadioButton::toggled, this, &VRadioButton::emitRadioButtonChanged);
}

void VRadioButton::SetValue(bool checked)
{
    _radioButton->blockSignals(true);
    _radioButton->setChecked(checked);
    _radioButton->blockSignals(false);
}

bool VRadioButton::GetValue() const { return _radioButton->isChecked(); }

void VRadioButton::emitRadioButtonChanged(bool checked) { emit ValueChanged(checked); }
