#include "PRadioButton.h"
#include "VRadioButton.h"

PRadioButton::PRadioButton(const std::string &tag, const std::string &label) : PLineItem(tag, label, _vRadioButton = new VRadioButton)
{
    _paramValue = label;
    connect(_vRadioButton, &VRadioButton::ValueChanged, this, &PRadioButton::radioButtonStateChanged);
}

void PRadioButton::updateGUI() const
{
    if (getParamsString() == _paramValue) _vRadioButton->SetValue(true);
    else _vRadioButton->SetValue(false);
}

void PRadioButton::radioButtonStateChanged() { 
    setParamsString(_paramValue); 
}
