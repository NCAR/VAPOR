#include <cmath>
#include <iostream>

#include "VSliderEdit.h"
#include "VSlider.h"
#include "VLineEdit.h"

VSliderEdit::VSliderEdit( double min, double max, double value )
: VContainer( this ),
  _isIntType( false )
{
    _lineEdit = new VLineEdit();
    _slider = new VSlider();

    SetRange( min, max );
    SetValue( value );

    layout()->addWidget(_slider);
    layout()->addWidget(_lineEdit);

    connect( _lineEdit, SIGNAL( ValueChanged( const std::string& ) ),
        this, SLOT( emitLineEditValueChanged( const std::string& ) ) );

    connect( _slider, SIGNAL( ValueChanged(double) ),
        this, SLOT( emitSliderValueChanged(double) ) );

    connect( _slider, SIGNAL( ValueChangedIntermediate(double) ),
        this, SLOT( emitSliderValueChangedIntermediate(double) ) );
}

double VSliderEdit::GetValue() const {
    return _value;
}

void VSliderEdit::SetValue( double value ) {
    if (_isIntType) value = round(value);
    if (value < _minValid) value = _minValid;
    if (value > _maxValid) value = _maxValid;

    if (_isIntType)
        _lineEdit->SetValue( std::to_string( (int)value ) );
    else
        _lineEdit->SetValue( std::to_string( value ) );
    _slider->SetValue( value );
    _value = value;
}

void VSliderEdit::SetRange( double min, double max ){
    if (_isIntType) {
        min = round(min);
        max = round(max);
    }

    if (min >= max) max = min;
    if (_value < min) _value = min;
    if (_value > max) _value = max;
   
    _slider->SetRange( min, max );
 
    _minValid = min;
    _maxValid = max;
}

void VSliderEdit::SetIntType( bool type ) {
    _isIntType = type;
    SetValue( _value );
}

void VSliderEdit::emitLineEditValueChanged( const std::string& value ) {
    try {
        double newValue = stod( value );//_lineEdit->GetValue());
        SetValue( newValue );
        emit ValueChanged( _value );
    }
    // If we can't convert the _lineEdit text to a double,
    // then revert to the previous value.
    catch (...) {
        SetValue( _value );
    }
}

void VSliderEdit::emitSliderValueChanged( double value ) {
    SetValue( value );
    emit ValueChanged( _value );
}

void VSliderEdit::emitSliderValueChangedIntermediate( double value ) {
    if (_isIntType)
        _lineEdit->SetValue( std::to_string( (int)value ) );
    else
        _lineEdit->SetValue( std::to_string( value ) );
    emit ValueChangedIntermediate( value );
}
