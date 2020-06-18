#include <iostream>
#include <cmath>

#include <vapor/VAssert.h>
#include "VSlider.h"

#define NUM_STEPS 100

VSlider::VSlider( double min, double max )
: VContainer(),
    _min(0.0),
    _max(0.0),
    _stepSize(1.0)
{
    _slider = new QSlider;
    _slider->setOrientation( Qt::Horizontal );
    _slider->setMinimum( 0 );
    _slider->setMaximum( NUM_STEPS );
    SetRange( min, max );
    SetValue( (max-min) / 2 );
    layout()->addWidget(_slider);

    connect( _slider, &QSlider::sliderMoved,
        this, &VSlider::_sliderChangedIntermediate );

    connect( _slider, &QSlider::sliderReleased,
        this, &VSlider::_sliderChanged );
}

void VSlider::SetValue( double value ) {
    if ( _stepSize <=0 )
        return;

    if (value > _max) value = _max;
    if (value < _min) value = _min;

    value = (value-_min)/_stepSize;
    _slider->blockSignals(true);
    _slider->setValue( value );
    _slider->blockSignals(false);
}

double VSlider::GetMinimum() const {
    return _min;
}

void VSlider::SetMinimum( double min ) {
    SetRange( min, _max );
}

double VSlider::GetMaximum() const {
    return _max;
}

void VSlider::SetMaximum( double max ) {
    SetRange( _min, max );
}

void VSlider::SetRange( double min, double max ) {
    VAssert( min <= max );

    _stepSize = ( max - min ) / NUM_STEPS;

    _min = min;
    _max = max;
    SetValue( GetValue() );
}

double VSlider::GetValue() const {
    int sliderVal = _slider->value();

    // Return min/max values if the slider is at the end.
    // note - Qt does not move the sliders to positions 0, 1, 99, or 100 until
    // the mouse is released.  
    if (sliderVal <= 2)                             // positions 0, 1 and 2
        return _min;
    if (sliderVal >= NUM_STEPS-2)                   // positions 98, 99, and 100
        return _max;

    double value = _stepSize * _slider->value() + _min;
    return value;
}

void VSlider::_sliderChanged() {
    double value = GetValue();
    emit ValueChanged( value );
}

void VSlider::_sliderChangedIntermediate( int position ) {
    double value = GetValue();
    emit ValueChangedIntermediate( value );
}
