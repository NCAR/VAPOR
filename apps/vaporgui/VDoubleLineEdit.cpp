#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include "VDoubleLineEdit.h"

VDoubleLineEdit::VDoubleLineEdit( double value ) : 
    VNumericLineEdit(),
    _value( value )
{
    std::string formattedNumber = _formatValue( _value );
    _setValueString( formattedNumber );
}

void VDoubleLineEdit::SetValueDouble( double value ) {
    std::string formattedNumber = _formatValue( value );

    try {
        _value = std::stod(formattedNumber);
    } catch (const std::invalid_argument&) {
        return;
    } catch (const std::out_of_range&) {
        return;
    }

    _setValueString( formattedNumber );
}

double VDoubleLineEdit::GetValueDouble() const {
    return _value;
}

void VDoubleLineEdit::_valueChanged() {
    std::string str = _getText();

    double value;
    try {
        value = std::stod( str );
        SetValueDouble( value );
        emit ValueChanged( _value );
    } catch (const std::invalid_argument&) {
        SetValueDouble( _value );
    } catch (const std::out_of_range&) {
        SetValueDouble( _value );
    }
}

std::string VDoubleLineEdit::_formatValue( double value ) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision( _decimalDigits );
    if ( _sciNotation ) {
        stream << std::scientific;
        stream << value;
    }
    else {
        stream << value;
    }
    return stream.str();
}
