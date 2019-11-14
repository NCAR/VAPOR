#pragma once

#include <string>
#include "VContainer.h"

class VSlider2;
class VLineEdit2;

class VSliderEdit : public VContainer {
    Q_OBJECT

public:
    VSliderEdit( double min=0., double max=1., double value=0. );

    void SetIntType( bool type );

    void SetValue( double value );
    void SetRange( double min, double max );

    double GetValue() const;

private:
    VLineEdit2* _lineEdit;
    VSlider2*   _slider;
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
