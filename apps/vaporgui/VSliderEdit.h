#pragma once

#include <string>
#include "VContainer.h"

class VSlider;
class VLineEdit;

class VSliderEdit : public VContainer {
    Q_OBJECT

public:
    VSliderEdit( double min=0., double max=1., double value=0. );

    void SetIntType( bool type );

    void SetValue( double value );
    void SetRange( double min, double max );

    double GetValue() const;

private:
    VLineEdit* _lineEdit;
    VSlider*   _slider;
    double     _minValid;
    double     _maxValid;
    double     _value;
    bool       _isIntType;

public slots:
    void emitLineEditValueChanged( const std::string& value );

    void emitSliderValueChanged( double value );
    void emitSliderValueChangedIntermediate( double value );

signals:
    void ValueChanged( double value );
    void ValueChangedIntermediate( double value );
};
