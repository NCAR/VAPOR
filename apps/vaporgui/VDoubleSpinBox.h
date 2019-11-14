#pragma once

#include <string>

#include <QWidget>
#include <QDoubleSpinBox>

#include "VContainer.h"

class VDoubleSpinBox2 : public VContainer {
    Q_OBJECT

public:
    VDoubleSpinBox2( double min, double max);

    void SetValue( double value );
    void SetRange( double min, double max );

    double GetValue() const;

private:
    QSpinBox* _spinBox;

public slots:
    void emitSpinBoxChanged();

signals:
    void ValueChanged( double value );
};
