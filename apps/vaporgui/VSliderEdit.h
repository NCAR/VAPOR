#pragma once

#include <string>
#include "VContainer.h"

class VSlider;
class VLineEdit;

//! class VSliderEdit
//!
//! Wraps a VSlider and a VLineEdit for selecting a numeric value within a
//! defined range.  Allows for integer and double value types.  This class
//! also provides vaporgui's standard setter/getter functions and signals.
class VSliderEdit : public VContainer {
    Q_OBJECT

public:
    VSliderEdit(double min = 0., double max = 1., double value = 0.);

    void SetIntType(bool type);

    void SetValue(double value);
    void SetRange(double min, double max);

    double GetValue() const;

private:
    VLineEdit *_lineEdit;
    VSlider *  _slider;
    double     _minValid;
    double     _maxValid;
    double     _value;
    bool       _isIntType;

private slots:
    void _lineEditChanged(const std::string &value);

    void _sliderChanged(double value);
    void _sliderChangedIntermediate(double value);

signals:
    void ValueChanged(double value);
    void ValueChangedInt(int value);

    void ValueChangedIntermediate(double value);
    void ValueChangedIntIntermediate(int value);
};
