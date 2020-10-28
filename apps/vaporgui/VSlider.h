#pragma once

#include <string>

#include <QWidget>
#include <QSlider>

#include "VHBoxWidget.h"

//! class VSlider
//!
//! Wraps a QSlider and provides vaporgui's standard setter/getter fucntions
//! and signals.  This class also provides range setting for the slider values.

class VSlider : public VHBoxWidget {
    Q_OBJECT

public:
    VSlider(double min = 0, double max = 1);

    double GetValue() const;
    void   SetValue(double value);

    double GetMinimum() const;
    void   SetMinimum(double min);

    double GetMaximum() const;
    void   SetMaximum(double max);

    void SetRange(double min, double max);

private:
    QSlider *_slider;
    double   _min;
    double   _max;
    double   _stepSize;

private slots:
    void _sliderChanged();
    void _sliderChangedIntermediate(int position);

signals:
    void ValueChanged(double value);
    void ValueChangedIntermediate(double value);
};

class ScrollWheelEater : public QObject {
    Q_OBJECT

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};
