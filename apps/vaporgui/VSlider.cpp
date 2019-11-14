#include <iostream>
#include <cmath>
#include "VSlider.h"

#define NUM_STEPS 100

VSlider2::VSlider2( double min, double max )
: VContainer( this ),
  _isInt( false )
{
    _slider = new QSlider;
    _slider->setOrientation( Qt::Horizontal );
    _slider->setMinimum( 0 );
    _slider->setMaximum( NUM_STEPS );
    SetRange( min, max );
    SetValue( (max-min) / 2 );
    layout()->addWidget(_slider);

    connect( _slider, SIGNAL( sliderMoved( int ) ),
        this, SLOT( emitSliderChangedIntermediate( int ) ) );
    
    connect( _slider, SIGNAL( sliderReleased() ),
        this, SLOT( emitSliderChanged() ) );
}

void VSlider2::SetValue( double value ) {
    if ( value > _maxValid)
        value = _maxValid;
    if ( value < _minValid)
        value = _minValid;

    if ( _isInt ) {
        // Nudge value to nearest whole number
        value = round( value );
        // Offset our value by _minValid
        value = value - _minValid;
        // Nudge again, to the nearest increment supported by the slider's discretization
        value = round( value * NUM_STEPS / ( _maxValid - _minValid ) );
    }
    else {
        value = ( value - _minValid ) / _stepSize;
    }

    _slider->blockSignals(true);
    _slider->setValue( value );
    _slider->blockSignals(false);
}

void VSlider2::SetRange( double min, double max ) {
    if ( _isInt ) {
        min = round( min );
        max = round( max );
    }

    if ( min > max ) min = max;
    if ( max < min ) max = min;

    _stepSize = ( max - min ) / NUM_STEPS;
    _minValid = min;
    _maxValid = max;
}

double VSlider2::GetValue() const {
    double value = _stepSize * _slider->value() + _minValid;
    
    if (_isInt) 
        value = round(value);

    return value;
}

void VSlider2::SetIntType( bool isInt ) {
    _isInt = isInt;
}

void VSlider2::emitSliderChanged() {
    double value = GetValue();

    // Nudge the current value to nearest whole number if we are of nt type,
    // which is done in the SetValue() method.
    if (_isInt) {
        SetValue( value );
    }

    emit ValueChanged( value );
}

void VSlider2::emitSliderChangedIntermediate( int position ) {
    double value = GetValue();
    emit ValueChangedIntermediate( value );
}
