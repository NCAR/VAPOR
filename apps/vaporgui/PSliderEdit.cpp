#include "PSliderEdit.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>
#include "VSliderEdit.h"

PSliderEdit::PSliderEdit(const std::string &tag, const std::string &label) : PLineItem(tag, label, _sliderEdit = new VSliderEdit) {}

PSliderEdit *PSliderEdit::SetRange(double min, double max)
{
    _sliderEdit->SetRange(min, max);
    return this;
}

PDoubleSliderEdit::PDoubleSliderEdit(const std::string &tag, const std::string &label) : PSliderEdit(tag, label)
{
    connect(_sliderEdit, &VSliderEdit::ValueChanged, this, &PDoubleSliderEdit::valueChanged);
    connect(_sliderEdit, &VSliderEdit::ValueChangedIntermediate, this, &PDoubleSliderEdit::valueChangedIntermediate);
}

void PDoubleSliderEdit::updateGUI() const
{
    double value = getParamsDouble();
    _sliderEdit->SetValue(value);
}

void PDoubleSliderEdit::valueChanged(double v) { setParamsDouble(v); }

void PDoubleSliderEdit::valueChangedIntermediate(double v) { dynamicSetParamsDouble(v); }

PIntegerSliderEdit::PIntegerSliderEdit(const std::string &tag, const std::string &label) : PSliderEdit(tag, label)
{
    _sliderEdit->SetIntType(true);
    connect(_sliderEdit, &VSliderEdit::ValueChangedInt, this, &PIntegerSliderEdit::valueChanged);
    connect(_sliderEdit, &VSliderEdit::ValueChangedIntIntermediate, this, &PIntegerSliderEdit::valueChangedIntermediate);
}

void PIntegerSliderEdit::updateGUI() const
{
    int value = getParamsLong();
    _sliderEdit->SetValue(value);
}

void PIntegerSliderEdit::valueChanged(int v) { setParamsLong(v); }

void PIntegerSliderEdit::valueChangedIntermediate(int v) { dynamicSetParamsLong(v); }
