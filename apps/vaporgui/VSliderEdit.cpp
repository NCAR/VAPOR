#include <cmath>
#include <iostream>

#include <QMenu>

#include "vapor/VAssert.h"

#include "VSliderEdit.h"
#include "VSlider.h"
#include "VLineEdit.h"
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
  _decDigits( 0 )
{
    _lineEdit = new VLineEdit();
    //_lineEdit->SetIsDouble( true );
    _slider = new VSlider();

    SetRange( min, max );
    SetValue( value );

    layout()->addWidget(_slider);
    layout()->addWidget(_lineEdit);

    setContextMenuPolicy( Qt::CustomContextMenu );
    _lineEdit->setContextMenuPolicy( Qt::NoContextMenu );
    //_slider->setContextMenuPolicy( Qt::CustomContextMenu );

    setFrameStyle(QFrame::Panel | QFrame::Raised );

    setContextMenuPolicy( Qt::CustomContextMenu );
    connect( this, &VSliderEdit::customContextMenuRequested,
        this, &VSliderEdit::ShowContextMenu );

    connect( _lineEdit, &VLineEdit::ValueChanged,
        this, &VSliderEdit::_lineEditChanged );

    connect( _slider, &VSlider::ValueChanged,
        this, &VSliderEdit::_sliderChanged );
    connect( _slider, &VSlider::ValueChangedIntermediate,
        this, &VSliderEdit::_sliderChangedIntermediate );
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

    VAssert(min <= max);
    if (_value < min) _value = min;
    if (_value > max) _value = max;
   
    _slider->SetRange( min, max );
 
    _minValid = min;
    _maxValid = max;
}

//void VSliderEdit::SetIntType( bool type ) {
//    _isIntType = type;
//    SetValue( _value );
//}

void VSliderEdit::_lineEditChanged( const std::string& value ) {
    try {
        double newValue = std::stod( value );
        SetValue( newValue );
        if (_isIntType)
            emit ValueChangedInt( (int)_value );
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
        emit ValueChangedInt( (int)_value );
    }
    else
        emit ValueChanged( _value );
}

void VSliderEdit::_sliderChangedIntermediate( double value ) {
    if (_isIntType) {
        _lineEdit->SetValue( std::to_string( (int)value ) );
        emit ValueChangedIntIntermediate( (int)value );
    }
    else {
        _lineEdit->SetValue( std::to_string( value ) );
        emit ValueChangedIntermediate( value );
    }
}

void VSliderEdit::ShowContextMenu( const QPoint& pos ) {
    std::cout << "VSliderEdit ShowContextMenu" << std::endl;
    QMenu menu;

    VSpinBoxAction* decimalAction = new VSpinBoxAction(tr("Decimal digits"), _decDigits);
    connect( decimalAction, &VSpinBoxAction::editingFinished,
        this, &VSliderEdit::_decimalDigitsChanged );
    menu.addAction(decimalAction);

    VCheckBoxAction* checkBoxAction = new VCheckBoxAction(tr("Scientific"), _scientific);
    connect( checkBoxAction, &VCheckBoxAction::clicked,
        this, &VSliderEdit::_scientificClicked );
    menu.addAction(checkBoxAction);

    QPoint globalPos = mapToGlobal(pos);
    menu.exec(globalPos);
}

void VSliderEdit::_decimalDigitsChanged( int value ) {
    _decDigits = value;
    SetValue( _value );
}

void VSliderEdit::_scientificClicked( bool value ) {
    _scientific = value;
    SetValue( _value );
}

void VSliderEdit::_minRangeChanged( double value ) {
    _minValid = value;
    if ( _value < _minValid ) {
        SetValue( _minValid );
    }
}

void VSliderEdit::_maxRangeChanged( double value ) {
    _maxValid = value;
    if ( _value > _maxValid ) {
        SetValue( _maxValid );
    }
}
