#include <cmath>
#include <iostream>

#include <QMenu>

#include "vapor/VAssert.h"

#include "VSliderEdit.h"
#include "VSlider.h"
#include "VLineEdit.h"
#include "VActions.h"
#include "VActions.h"

VSliderEdit::VSliderEdit( 
    double min, 
    double max, 
    double value,
    bool intType 
) : VContainer(),
  _minValid( min ),
  _maxValid( max ),
  _value( value ),
  _isIntType( intType ),
  _scientific( false ),
  _decDigits( 0 ),
  _menu( nullptr ),
  _minIntAction( nullptr ),
  _maxIntAction( nullptr ),
  _minDoubleAction( nullptr ),
  _maxDoubleAction( nullptr ),
  _decimalAction( nullptr ),
  _scientificAction( nullptr )
{
    _lineEdit = new VLineEdit();
    _slider = new VSlider();

    SetRange( min, max );
    SetValue( value );

    layout()->addWidget(_slider);
    layout()->addWidget(_lineEdit);

    setContextMenuPolicy( Qt::CustomContextMenu );
    _lineEdit->setContextMenuPolicy( Qt::NoContextMenu );

    setFrameStyle(QFrame::Panel | QFrame::Raised );

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &VSliderEdit::customContextMenuRequested,
        this, &VSliderEdit::ShowContextMenu );

    connect( _lineEdit, SIGNAL( ValueChanged( std::string ) ),
        this, SLOT( _lineEditChanged( std::string ) ) );

    connect( _slider, &VSlider::ValueChanged,
        this, &VSliderEdit::_sliderChanged );
    connect( _slider, &VSlider::ValueChangedIntermediate,
        this, &VSliderEdit::_sliderChangedIntermediate );

    MakeContextMenu();
}

VSliderEdit::VSliderEdit(
    bool intType
) : VSliderEdit( 0, 1, 0, intType )
{}

double VSliderEdit::GetValue() const {
    return _value;
}

void VSliderEdit::SetValue( double value ) {
    if (_isIntType) value = std::round(value);
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
    
    if ( min > max ) {
        min = max;
    }
    else if ( max < min ) {
        min = max;
    }

    if (_value < min) _value = min;
    if (_value > max) _value = max;
    
    _minValid = min;
    _maxValid = max;

    _slider->SetRange( min, max );
    _slider->SetValue( _value );
 
    if ( _minIntAction != nullptr )
        _minIntAction->SetValue( _minValid );
    if ( _maxIntAction != nullptr )
        _maxIntAction->SetValue( _maxValid );
    if ( _minDoubleAction != nullptr )
        _minDoubleAction->SetValue( _minValid );
    if ( _maxDoubleAction != nullptr )
        _maxDoubleAction->SetValue( _maxValid );
}

void VSliderEdit::SetMinimum( double min ) {
    _minValid = min;

    if( _maxValid < _minValid ) {
        _maxValid = min;
    }

    SetRange( _minValid, _maxValid );
}

void VSliderEdit::SetMaximum( double max ) {
    _maxValid = max;

    if( _minValid > max ) {
        _minValid = max;
    }

    SetRange( _minValid, _maxValid );
}

void VSliderEdit::SetScientific( bool sci ) {
    _scientific = sci;
    SetValue( _value );
}

void VSliderEdit::SetNumDigits( int digits ) {
    _decDigits = digits;
    SetValue( _value );
}

double VSliderEdit::GetMinimum() const {
    return _minValid;
}

double VSliderEdit::GetMaximum() const {
    return _maxValid;
}

bool VSliderEdit::GetScientific() const {
    return _scientific;
}

int VSliderEdit::GetNumDigits() const {
    return _decDigits;
}

void VSliderEdit::_lineEditChanged( const std::string& value ) {
    try {
        double newValue = std::stod( value );
        SetValue( newValue );
        if (_isIntType)
            emit ValueChanged( (int)_value );
        else 
            emit ValueChanged( _value );
    }
    // If we can't convert the _lineEdit text to a double,
    // then revert to the previous value.
    catch (...) {
        SetValue( _value );
    }
}

void VSliderEdit::_sliderChanged( double value ) {
    SetValue( value );
    if (_isIntType) {
        emit ValueChanged( (int)_value );
    }
    else
        emit ValueChanged( _value );
}

void VSliderEdit::_sliderChangedIntermediate( double value ) {
    if (_isIntType) {
        _lineEdit->SetValue( std::to_string( (int)value ) );
        emit ValueChangedIntermediate( (int)value );
    }
    else {
        _lineEdit->SetValue( std::to_string( value ) );
        emit ValueChangedIntermediate( value );
    }
}

void VSliderEdit::ShowContextMenu( const QPoint& pos ) {
    QPoint globalPos = mapToGlobal(pos);
    _menu->exec(globalPos);
}

void VSliderEdit::MakeContextMenu() {
    _menu = new QMenu;

    _minIntAction = new VIntLineEditAction( "Minimum slider value", _minValid );
    connect( _minIntAction, &VIntLineEditAction::ValueChanged,
        this, &VSliderEdit::SetMinimum);
    
    _maxIntAction = new VIntLineEditAction( "Maximum slider value", _maxValid );
    connect( _maxIntAction, &VIntLineEditAction::ValueChanged,
        this, &VSliderEdit::SetMaximum);

    _minDoubleAction = new VDoubleLineEditAction( "Minimum slider value", _minValid );
    connect( _minDoubleAction, &VDoubleLineEditAction::ValueChanged,
        this, &VSliderEdit::SetMinimum);
    
    _maxDoubleAction = new VDoubleLineEditAction( "Maximum slider value", _maxValid );
    connect( _maxDoubleAction, &VDoubleLineEditAction::ValueChanged,
        this, &VSliderEdit::SetMaximum);

    _decimalAction = new VSpinBoxAction( "Decimal digits", _decDigits);
    connect( _decimalAction, &VSpinBoxAction::editingFinished,
        this, &VSliderEdit::_decimalDigitsChanged );

    _scientificAction = new VCheckBoxAction( "Scientific", _scientific);
    connect( _scientificAction, &VCheckBoxAction::clicked,
        this, &VSliderEdit::_scientificClicked );

    if ( _isIntType ) {
        _menu->addAction( _minIntAction );
        _menu->addAction( _maxIntAction );
    }
    else {
        _menu->addAction( _minDoubleAction );
        _menu->addAction( _maxDoubleAction );
        _menu->addAction( _decimalAction );
    }
    _menu->addAction( _scientificAction );
}

void VSliderEdit::_decimalDigitsChanged( int value ) {
    _decDigits = value;
    SetValue( _value );
    emit FormatChanged();
}

void VSliderEdit::_scientificClicked( bool value ) {
    _scientific = value;
    SetValue( _value );
    emit FormatChanged();
}

void VSliderEdit::_minRangeChanged( double value ) {
    _minValid = value;
    if ( _minValid > _maxValid ) {
        _maxValid = _minValid;
    }
    if ( _value < _minValid ) {
        SetValue( _minValid );
    }
    emit FormatChanged();
}

void VSliderEdit::_maxRangeChanged( double value ) {
    _maxValid = value;
    if ( _maxValid < _minValid ) {
        _minValid = _maxValid;
    }
    if ( _value > _maxValid ) {
        SetValue( _maxValid );
    }
    emit FormatChanged();
}
