#include <cmath>
#include <iostream>

#include <QMenu>

#include "vapor/VAssert.h"

#include "VDoubleSliderEdit.h"
#include "VDoubleLineEdit.h"
#include "VDoubleRangeMenu.h"
#include "VSlider.h"

VDoubleSliderEdit::VDoubleSliderEdit( 
    double min, 
    double max, 
    double value,
    bool rangeChangable
) : VSliderEditInterface(),
    _value( value ),
    _rangeChangable( rangeChangable )
{
    _slider->SetRange( min, max );
    _slider->SetValue( value );
    connect( _slider, &VSlider::ValueChanged,
        this, &VDoubleSliderEdit::SetValue );
    connect( _slider, &VSlider::ValueChangedIntermediate,
        this, &VDoubleSliderEdit::_sliderChangedIntermediate );
   
    _lineEdit = new VDoubleLineEdit( value );
    _lineEdit->RemoveContextMenu();
    layout()->addWidget(_lineEdit);
    connect( _lineEdit, SIGNAL( ValueChanged( double ) ),
        this, SLOT( SetValue( double ) ) );
    
    _makeContextMenu();
}

void VDoubleSliderEdit::AllowUserRange(bool allowed)
{
    _rangeChangable = allowed;
    _menu->AllowUserRange(allowed);
}

void VDoubleSliderEdit::_makeContextMenu() {
    _menu = new VDoubleRangeMenu(
        this,
        _lineEdit->GetSciNotation(),
        _lineEdit->GetNumDigits(),
        _slider->GetMinimum(),
        _slider->GetMaximum(),
        _rangeChangable
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
    if (value == _lineEdit->GetSciNotation()) {
        return;
    }
    _lineEdit->SetSciNotation( value );
    emit FormatChanged();
}

int VDoubleSliderEdit::GetNumDigits() const {
    return _lineEdit->GetNumDigits();
}

void VDoubleSliderEdit::SetNumDigits( int digits ) {
    if (digits == _lineEdit->GetNumDigits()) {
        return;
    }
    _lineEdit->SetNumDigits( digits );
    emit FormatChanged();
}

double VDoubleSliderEdit::GetValue() const {
    return _value;
}

void VDoubleSliderEdit::SetValue( double value ) {

    // If 'value' is illegal, reset _lineEdit's text and return
    //
    
    if ( value == _value                ||
         value <  _slider->GetMinimum() ||
         value >  _slider->GetMaximum()
    ) {
        _lineEdit->SetValueDouble( _value );
        return;
    }

    /*double min = _slider->GetMinimum();
    if (value < min) {
        if ( _rangeChangable ) {
            _slider->SetMinimum( value );
            _menu->SetMinimum( value );
        }
        else {
            value = min;
        }
    }

    double max = _slider->GetMaximum();
    if (value > max) {
        if ( _rangeChangable ) {
            _slider->SetMaximum( value );
            _menu->SetMaximum( value );
        }
        else {
            value = max;
        }
    }*/
    _value = value;

    //dynamic_cast<VDoubleLineEdit*>(_lineEdit)->SetValueDouble( _value );
    _lineEdit->SetValueDouble( _value );
    _slider->SetValue( _value );
    /*_lineEdit->blockSignals( true );
    dynamic_cast<VDoubleLineEdit*>(_lineEdit)->SetValueDouble( _value );
    _lineEdit->blockSignals( false );

    _slider->blockSignals(   true );
    _slider->SetValue( _value );
    _slider->blockSignals(   false );*/

    if ( QObject::sender() != nullptr ) {
        emit ValueChanged( _value );
    }
}

double VDoubleSliderEdit::GetMinimum() const {
    return _slider->GetMinimum();;
}

void VDoubleSliderEdit::SetMinimum( double min ) {
    if (min == _slider->GetMinimum() ||
        min >= _slider->GetMaximum() ||
        min > _value
    ) {
        //std::cout << "set min too high" << min << " " << _value << " " << _slider->GetMinimum() << std::endl;
        //_menu->blockSignals( true );
        _menu->SetMinimum( _slider->GetMinimum() );
        //_menu->blockSignals( false );
        return;
    }

    /*if ( min > _value ) {
        _value = min;
        _lineEdit->SetValueDouble( min );
    }*/
   
    _slider->SetMinimum( min );
   
    _menu->SetMinimum( min );
    /*if ( min >= _slider->GetMaximum() ) {
        _menu->SetMaximum( min );
    }*/
    
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
    if (max == _slider->GetMaximum() ||
        max <= _slider->GetMinimum() ||
        max < _value
    ) {
        _menu->SetMaximum( _slider->GetMaximum() );
        return;
    }

    /*if ( max < _value ) {
        _value = max;
        _lineEdit->SetValueDouble( max );
    }*/
    
    _slider->SetMaximum( max );

    _menu->SetMaximum( max );
    //if ( max <= _slider->GetMinimum() ) {
    //    _menu->SetMinimum( max );
    //}

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
    dynamic_cast<VDoubleLineEdit*>(_lineEdit)->SetValueDouble( value );
    emit ValueChangedIntermediate( value );
}
