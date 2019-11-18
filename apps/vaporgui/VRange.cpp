#include "VRange.h"

VRange::VRange( QWidget* parent, float min, float max, const std::string& minLabel,
                const std::string& maxLabel )
      : QWidget( parent )
{
    _layout    = new QVBoxLayout(this);

    _minSlider = new VSlider( this, minLabel, min, max );
    _maxSlider = new VSlider( this, maxLabel, min, max );
    connect( _minSlider, SIGNAL( _valueChanged() ), this, SLOT( _respondMinSlider() ) );
    connect( _maxSlider, SIGNAL( _valueChanged() ), this, SLOT( _respondMaxSlider() ) );

    _layout->addWidget( _minSlider );
    _layout->addWidget( _maxSlider );
}

VRange::~VRange() {}

void
VRange::SetRange( float min, float max )
{
    VAssert( max >= min );
    _minSlider->SetRange( min, max );
    _maxSlider->SetRange( min, max );
}

void
VRange::SetCurrentValLow( float low )
{
    /* _minSlider will only respond if low is within a valid range. */
    _minSlider->SetCurrentValue( low );
    _adjustMaxToMin();
}

void
VRange::SetCurrentValHigh( float high )
{
    /* _maxSlider will only respond if high is within a valid range. */
    _maxSlider->SetCurrentValue( high );
    _adjustMinToMax();
}

void
VRange::GetCurrentValRange( float& low, float& high ) const
{
    low  = _minSlider->GetCurrentValue();
    high = _maxSlider->GetCurrentValue();
}

void
VRange::_adjustMaxToMin()
{
    float low  = _minSlider->GetCurrentValue();
    float high = _maxSlider->GetCurrentValue();
    if( low > high )
        _maxSlider->SetCurrentValue( low );
}

void
VRange::_adjustMinToMax()
{
    float high = _maxSlider->GetCurrentValue();
    float min  = _minSlider->GetCurrentValue();
    if( high < min  )
        _minSlider->SetCurrentValue( high );
}

void
VRange::_respondMinSlider()
{
    _adjustMaxToMin();
    emit _rangeChanged();
}

void
VRange::_respondMaxSlider()
{
    _adjustMinToMax();
    emit _rangeChanged();
}
