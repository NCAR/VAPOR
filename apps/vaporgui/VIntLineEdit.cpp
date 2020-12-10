#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include <ErrorReporter.h>

#include "VIntLineEdit.h"

VIntLineEdit::VIntLineEdit( int value ) : 
    VNumericLineEdit(),
    _value( value )
{
    std::string formattedNumber = _formatValue( _value );
    SetValueString( formattedNumber );
}

void VIntLineEdit::SetValueInt( int value ) {
    std::string formattedNumber = _formatValue( value );

    try {
        _value = (int)std::stod(formattedNumber);
    } catch (const std::invalid_argument&) {
        return;
    } catch (const std::out_of_range&) {
        return;
    }

    SetValueString( formattedNumber );
}

int VIntLineEdit::GetValueInt() const {
    return _value;
}

void VIntLineEdit::_valueChanged() {
    std::string str = _getText();

    int value;
    try {
        double dValue = std::stod( str );
       
        if ( _checkOverflow( dValue ) ) {
            SetValueInt( _value );
            return;
        }
        
        value = (int)std::stod( str );

        // If value changed, update and emit, otherwiese revert to old value
        if ( value != _value ) {
            SetValueInt( value );
            emit ValueChanged( _value );
        }
        else {
            SetValueInt( _value );
        }
    } catch (const std::invalid_argument&) {
        SetValueInt( _value );
    } catch (const std::out_of_range&) {
        SetValueInt( _value );
    }
}

std::string VIntLineEdit::_formatValue( int value ) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision( _decimalDigits );
    if ( _sciNotation ) {
        stream << std::scientific;
        stream << (double)value;
    }
    else {
        stream << value;
    }
    return stream.str();
}

bool VIntLineEdit::_checkOverflow( double value ) {
    std::stringstream svalue;
    svalue << std::fixed << std::setprecision(0) << value;
    std::string error = "Value " + svalue.str() + " exceeds ";

    if ( value < INT_MIN ) {
        error += " minimum integer limit (" + std::to_string(INT_MIN) + ").";
        MSG_ERR( error );
        return true;
    }
    if ( value > INT_MAX ) {
        error += " maximum integer limit (" + std::to_string(INT_MAX) + ").";
        MSG_ERR( error );
        return true;
    }

    return false;
}
