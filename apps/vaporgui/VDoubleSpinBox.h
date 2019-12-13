#pragma once

#include <string>

#include <QWidget>
#include <QDoubleSpinBox>

#include "VContainer.h"

//! class VDoubleSpinBox
//!
//! Wraps a QDoubleSpinBox and provides vaporgui's standard setter/getter
//! functions and signals.

class VDoubleSpinBox : public VContainer {
    Q_OBJECT

public:
    VDoubleSpinBox(double min, double max);

    void SetValue(double value);
    void SetRange(double min, double max);

    double GetValue() const;

private:
    QSpinBox *_spinBox;

public slots:
    void emitSpinBoxChanged();

signals:
    void ValueChanged(double value);
};
