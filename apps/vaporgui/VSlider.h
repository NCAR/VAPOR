#pragma once

#include <string>

#include <QWidget>
#include <QSlider>

#include "VContainer.h"

class VSlider : public VContainer {
    Q_OBJECT

public:
    VSlider( double min=0, double max=1 );

    void SetValue( double value );
    void SetRange( double min, double max );
    void SetIntType( bool isInt );

    double GetValue() const;

private:
    void _adjustValue( double& value ) const;

    QSlider* _slider;
    double _minValid;
    double _maxValid;
    double _stepSize;
    bool _isInt;

public slots:
    void emitSliderChanged();
    void emitSliderChangedIntermediate( int position );

signals:
    void ValueChanged( double value );
    void ValueChangedIntermediate( double value );
};
