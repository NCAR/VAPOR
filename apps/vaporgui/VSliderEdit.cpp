#include <cmath>
#include <iostream>

#include "vapor/VAssert.h"
#include "VSliderEdit.h"
#include "VSlider.h"
#include "VLineEdit.h"

VSliderEdit::VSliderEdit(double min, double max, double value) : VContainer(), _isIntType(false)
{
    _lineEdit = new VLineEdit();
    _slider = new VSlider();

    SetRange(min, max);
    SetValue(value);

    layout()->addWidget(_slider);
    layout()->addWidget(_lineEdit);

    connect(_lineEdit, &VLineEdit::ValueChanged, this, &VSliderEdit::_lineEditChanged);

    connect(_slider, &VSlider::ValueChanged, this, &VSliderEdit::_sliderChanged);

    connect(_slider, &VSlider::ValueChangedIntermediate, this, &VSliderEdit::_sliderChangedIntermediate);
}

double VSliderEdit::GetValue() const { return _value; }

void VSliderEdit::SetValue(double value)
{
    if (_isIntType) value = std::round(value);
    if (value < _minValid) value = _minValid;
    if (value > _maxValid) value = _maxValid;

    if (_isIntType)
        _lineEdit->SetValue(std::to_string((int)value));
    else
        _lineEdit->SetValue(std::to_string(value));
    _slider->SetValue(value);
    _value = value;
}

void VSliderEdit::SetRange(double min, double max)
{
    if (_isIntType) {
        min = round(min);
        max = round(max);
    }

    VAssert(min <= max);
    if (_value < min) _value = min;
    if (_value > max) _value = max;

    _slider->SetRange(min, max);

    _minValid = min;
    _maxValid = max;
}

void VSliderEdit::SetIntType(bool type)
{
    _isIntType = type;
    SetValue(_value);
}

void VSliderEdit::_lineEditChanged(const std::string &value)
{
    try {
        double newValue = std::stod(value);
        SetValue(newValue);
        if (_isIntType)
            emit ValueChangedInt((int)_value);
        else
            emit ValueChanged(_value);
    }
    // If we can't convert the _lineEdit text to a double,
    // then revert to the previous value.
    catch (...) {
        SetValue(_value);
    }
}

void VSliderEdit::_sliderChanged(double value)
{
    SetValue(value);
    if (_isIntType) {
        emit ValueChangedInt((int)_value);
    } else
        emit ValueChanged(_value);
}

void VSliderEdit::_sliderChangedIntermediate(double value)
{
    if (_isIntType) {
        _lineEdit->SetValue(std::to_string((int)value));
        emit ValueChangedIntIntermediate((int)value);
    } else {
        _lineEdit->SetValue(std::to_string(value));
        emit ValueChangedIntermediate(value);
    }
}
