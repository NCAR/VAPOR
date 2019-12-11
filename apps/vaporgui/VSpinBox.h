#pragma once

#include <string>

#include <QWidget>
#include <QSpinBox>

#include "VContainer.h"

//! class VSpinBox
//!
//! Wraps a QSpinBox and provides vaporgui's standard setter/getter functions
//! and signals.

class VSpinBox : public VContainer {
    Q_OBJECT

public:
    VSpinBox( int min, int max);

    void SetValue( int value );
    void SetRange( int min, int max );

    int GetValue() const;

private:
    QSpinBox* _spinBox;

public slots:
    void emitSpinBoxChanged();

signals:
    void ValueChanged( int value );
};
