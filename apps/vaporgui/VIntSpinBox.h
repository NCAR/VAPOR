#pragma once

#include <string>

#include <QWidget>
#include <QSpinBox>

#include "VHBoxWidget.h"

//! class VIntSpinBox
//!
//! Wraps a QSpinBox and provides vaporgui's standard setter/getter functions
//! and signals.

class VIntSpinBox : public VHBoxWidget {
    Q_OBJECT

public:
    VIntSpinBox(int min, int max);

    void SetValue(int value);
    void SetRange(int min, int max);

    int GetValue() const;

private:
    QSpinBox *_spinBox;

public slots:
    void emitSpinBoxChanged(int value);
    void emitSpinBoxFinished();

signals:
    void ValueChanged(int value);
};
