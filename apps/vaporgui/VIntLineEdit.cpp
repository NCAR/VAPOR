#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include <VIntLineEdit.h>

VIntLineEdit::VIntLineEdit( int value ) : 
    VNumericLineEdit(),
    _value( value )
{
    std::string formattedNumber = _formatValue( _value );
    _setValueString( formattedNumber );
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

    _setValueString( formattedNumber );
}

int VIntLineEdit::GetValueInt() const {
    return _value;
}

void VIntLineEdit::_valueChanged() {
    std::string str = _getText();

    int value;
    try {
        value = (int)std::stod( str );
        SetValueInt( value );
        emit ValueChanged( _value );
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
