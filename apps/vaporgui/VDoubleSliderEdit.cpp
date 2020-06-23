#include <cmath>
#include <iostream>

#include <QMenu>

#include "vapor/VAssert.h"

#include "VDoubleSliderEdit.h"
#include "VLineEditTemplate.h"
#include "VDoubleRangeMenu.h"
#include "VSlider.h"

VDoubleSliderEdit::VDoubleSliderEdit( double min, double max, double value )
    : VSliderEditInterface(),
      _value( value )
{
    _slider = new VSlider( min, max );
    layout()->addWidget(_slider);
    connect( _slider, &VSlider::ValueChanged,
        this, &VDoubleSliderEdit::SetValue );
    connect( _slider, &VSlider::ValueChangedIntermediate,
        this, &VDoubleSliderEdit::_sliderChangedIntermediate );
   
    _lineEdit = new VDoubleLineEdit( _value, false );
    layout()->addWidget(_lineEdit);
    _lineEdit->setContextMenuPolicy( Qt::NoContextMenu );
    connect( _lineEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( SetValue( double ) ) );
    
    _makeContextMenu();
}

void VDoubleSliderEdit::_makeContextMenu() {
    _menu = new VDoubleRangeMenu(
        this,
        _lineEdit->GetSciNotation(),
        _lineEdit->GetNumDigits(),
        _slider->GetMinimum(),
        _slider->GetMaximum()
    );
    connect( _menu, &VNumericFormatMenu::DecimalDigitsChanged,
        this, &VDoubleSliderEdit::SetNumDigits );
    connect( _menu, &VNumericFormatMenu::SciNotationChanged,
        this, &VDoubleSliderEdit::SetSciNotation );
    connect( _menu, &VDoubleRangeMenu::MinChanged,
        this, &VDoubleSliderEdit::SetMinimum );
    connect( _menu, &VDoubleRangeMenu::MaxChanged,
        this, &VDoubleSliderEdit::SetMaximum );
}

bool VDoubleSliderEdit::GetSciNotation() const {
    return _lineEdit->GetSciNotation();
}

void VDoubleSliderEdit::SetSciNotation( bool value ) {
    _lineEdit->SetSciNotation( value );
    emit FormatChanged();
}

int VDoubleSliderEdit::GetNumDigits() const {
    return _lineEdit->GetNumDigits();
}

void VDoubleSliderEdit::SetNumDigits( int digits ) {
    _lineEdit->SetNumDigits( digits );
    emit FormatChanged();
}

double VDoubleSliderEdit::GetValue() const {
    return _value;
}

void VDoubleSliderEdit::SetValue( double value ) {
    double min = _slider->GetMinimum();
    if (value < min) value = min;

    double max = _slider->GetMaximum();
    if (value > max) value = max;
    _value = value;

    _lineEdit->blockSignals( true );
    dynamic_cast<VDoubleLineEdit*>(_lineEdit)->SetValue( _value );
    _lineEdit->blockSignals( false );

    _slider->blockSignals(   true );
    _slider->SetValue( _value );
    _slider->blockSignals(   false );

    if ( QObject::sender() != nullptr ) {
        emit ValueChanged( _value );
    }
}

double VDoubleSliderEdit::GetMinimum() const {
    return _slider->GetMinimum();;
}

void VDoubleSliderEdit::SetMinimum( double min ) {
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

double VDoubleSliderEdit::GetMaximum() const {
    return _slider->GetMaximum();
}

void VDoubleSliderEdit::SetMaximum( double max ) {
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

void VDoubleSliderEdit::ShowContextMenu( const QPoint& pos ) {
    QPoint globalPos = mapToGlobal(pos);
    _menu->exec(globalPos);
}

void VDoubleSliderEdit::_sliderChangedIntermediate( double value ) {
    dynamic_cast<VDoubleLineEdit*>(_lineEdit)->SetValue( value );
    emit ValueChangedIntermediate( value );
}
