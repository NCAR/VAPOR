#include <cmath>
#include <iostream>

#include "vapor/VAssert.h"

#include "VIntSliderEdit.h"
#include "VLineEdit3.h"
#include "VIntRangeMenu.h"
#include "VSlider.h"

VIntSliderEdit::VIntSliderEdit( int min, int max, int value )
    : VAbstractSliderEdit(),
      _value( value )
{
    _slider = new VSlider( min, max );
    layout()->addWidget(_slider);
    connect( _slider, &VSlider::ValueChanged,
        this, &VIntSliderEdit::SetValue );
    connect( _slider, &VSlider::ValueChangedIntermediate,
        this, &VIntSliderEdit::_sliderChangedIntermediate );
   
    _lineEdit = new VIntLineEdit( _value, false );
    layout()->addWidget(_lineEdit);
    _lineEdit->setContextMenuPolicy( Qt::NoContextMenu );
    connect( _lineEdit, &VIntLineEdit::ValueChanged,
        this, &VIntSliderEdit::SetValue );
    
    _makeContextMenu();
}

void VIntSliderEdit::_makeContextMenu() {
    _menu = new VIntRangeMenu(
        this,
        _lineEdit->GetSciNotation(),
        _lineEdit->GetNumDigits(),
        _slider->GetMinimum(),
        _slider->GetMaximum()
    );
    connect( _menu, &VNumericFormatMenu::DecimalDigitsChanged,
        this, &VIntSliderEdit::SetNumDigits );
    connect( _menu, &VNumericFormatMenu::SciNotationChanged,
        this, &VIntSliderEdit::SetSciNotation );
    connect( _menu, &VIntRangeMenu::MinChanged,
        this, &VIntSliderEdit::SetMinimum );
    connect( _menu, &VIntRangeMenu::MaxChanged,
        this, &VIntSliderEdit::SetMaximum );
}

bool VIntSliderEdit::GetSciNotation() const {
    return _lineEdit->GetSciNotation();
}

void VIntSliderEdit::SetSciNotation( bool value ) {
    _lineEdit->SetSciNotation( value );
    emit FormatChanged();
}

int VIntSliderEdit::GetNumDigits() const {
    return _lineEdit->GetNumDigits();
}

void VIntSliderEdit::SetNumDigits( int digits ) {
    _lineEdit->SetNumDigits( digits );
    emit FormatChanged();
}

int VIntSliderEdit::GetValue() const {
    return _value;
}

void VIntSliderEdit::SetValue( int value ) {
    int min = _slider->GetMinimum();
    if (value < min) value = min;

    int max = _slider->GetMaximum();
    if (value > max) value = max;
    _value = value;

    _lineEdit->blockSignals( true );
    dynamic_cast<VIntLineEdit*>(_lineEdit)->SetValue( _value );
    _lineEdit->blockSignals( false );

    _slider->blockSignals(   true );
    _slider->SetValue( _value );
    _slider->blockSignals(   false );

    if ( QObject::sender() != nullptr ) {
        emit ValueChanged( _value );
    }
}

int VIntSliderEdit::GetMinimum() const {
    return _slider->GetMinimum();;
}

void VIntSliderEdit::SetMinimum( int min ) {
    if ( min > _value ) {
        _value = min;
        _lineEdit->SetValue( min );
    }
   
    _slider->SetMinimum( min );
   
    _menu->SetMinRange( min );
    if ( min >= _slider->GetMaximum() ) {
        _menu->SetMaxRange( min );
    }
    
    // If sender() is a nullptr, then this fuction is being called from Update().
    // Don't emit anythong.  Otherwise, emit our signal.
    if ( QObject::sender() != nullptr ) {
        emit MinimumChanged( min );
    }
}

int VIntSliderEdit::GetMaximum() const {
    return _slider->GetMaximum();
}

void VIntSliderEdit::SetMaximum( int max ) {
    if ( max < _value ) {
        _value = max;
        _lineEdit->SetValue( max );
    }
    
    _slider->SetMaximum( max );

    _menu->SetMaxRange( max );
    if ( max <= _slider->GetMinimum() ) {
        _menu->SetMinRange( max );
    }

    // If sender() is a nullptr, then this fuction is being called from Update().
    // Don't emit anythong.  Otherwise, emit our signal.
    if ( QObject::sender() != nullptr ) {
        emit MaximumChanged( max );
    }
}

void VIntSliderEdit::ShowContextMenu( const QPoint& pos ) {
    QPoint globalPos = mapToGlobal(pos);
    _menu->exec(globalPos);
}

void VIntSliderEdit::_sliderChangedIntermediate( int value ) {
    dynamic_cast<VIntLineEdit*>(_lineEdit)->SetValue( value );
    emit ValueChangedIntermediate( value );
}
