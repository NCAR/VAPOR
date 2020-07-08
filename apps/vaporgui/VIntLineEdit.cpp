#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include <VIntLineEdit.h>

VIntLineEdit::VIntLineEdit( int value, bool useMenu ) : 
    VAbstractLineEdit(useMenu),
    _value( value )
{
    std::string formattedNumber = _formatValue( _value );
    _lineEdit->setText( QString::fromStdString( formattedNumber ) );
    _lineEdit->setToolTip( QString::fromStdString( formattedNumber ) );
}

void VIntLineEdit::SetValue( int value ) {
    _value = value;
    std::string formattedNumber = _formatValue( _value );
    _lineEdit->setText( QString::fromStdString( formattedNumber ) );
    _lineEdit->setToolTip( QString::fromStdString( formattedNumber ) );
}

int VIntLineEdit::GetValue() const {
    return _value;
}

void VIntLineEdit::_valueChanged() {
    bool legalConversion;
    QString str = _lineEdit->text();

    // QString does not convert integer values that have scientific notation,
    // so convert the string to a double, then cast to int :(
    int value = (int)str.toDouble( &legalConversion );

    if (legalConversion) {
        SetValue( value );
        emit VAbstractLineEdit::ValueChanged( _value );
    }
    else {
        SetValue( _value );
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
