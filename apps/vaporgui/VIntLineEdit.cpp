#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include <VIntLineEdit.h>

VIntLineEdit::VIntLineEdit( int value, bool useMenu ) : 
    VNumericLineEdit(),
    _value( value )
{
    // Disconnect the QLineEdit's interaction with VStringLineEdit::_valueChanged(),
    // and reconnect it to VIntLineEdit::_valueChanged()
    disconnect( _lineEdit, SIGNAL( editingFinished() ),
        this, SLOT( _valueChanged() ) );
    connect( _lineEdit, SIGNAL( editingFinished() ),
        this, SLOT( _valueChanged() ) );

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
        value = (int)std::stod( str );
        SetValueInt( value );
        emit ValueChanged( _value );
    } catch (const std::invalid_argument&) {
        SetValueInt( _value );
    } catch (const std::out_of_range&) {
        SetValueInt( _value );
    }

    /*bool legalConversion;
    QString str = _lineEdit->text();
    // QString does not convert integer values that have scientific notation,
    // so convert the string to a double, then cast to int :(
    int value = (int)str.toDouble( &legalConversion );

    if (legalConversion) {
        SetValueInt( value );
        emit ValueChanged( _value );
    }
    else {
        SetValueInt( _value );
    }*/
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
