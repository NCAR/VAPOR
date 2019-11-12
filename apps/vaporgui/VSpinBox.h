#pragma once

#include <string>

#include <QWidget>
#include <QSpinBox>

#include "VContainer.h"

class VSpinBox2 : public VContainer {
    Q_OBJECT

public:
    VSpinBox2( int min, int max);

    void SetValue( int value );
    void SetRange( int min, int max );

    int GetValue() const;

private:
    QSpinBox* _spinBox;

public slots:
    void emitSpinBoxChanged( int value ) {};
    void emitSpinBoxChanged();

signals:
    void ValueChanged( int value );
};
