#pragma once

#include <string>

#include <QWidget>
#include <QSlider>

#include "VContainer.h"

//! class VSlider
//!
//! Wraps a QSlider and provides vaporgui's standard setter/getter fucntions
//! and signals.  This class also provides range setting for the slider values.

class VSlider : public VContainer {
    Q_OBJECT

public:
    VSlider(double min = 0, double max = 1);

    void SetValue(double value);
    void SetRange(double min, double max);

    double GetValue() const;

private:
    QSlider *_slider;
    double   _minValid;
    double   _maxValid;
    double   _stepSize;

private slots:
    void _sliderChanged();
    void _sliderChangedIntermediate(int position);

signals:
    void ValueChanged(double value);
    void ValueChangedIntermediate(double value);
};
