#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include <VDoubleLineEdit.h>

VDoubleLineEdit::VDoubleLineEdit( double value, bool useMenu ) : 
    VAbstractLineEdit(useMenu),
    _value( value )
{
    std::string formattedNumber = _formatValue( _value );
    _lineEdit->setText( QString::fromStdString( formattedNumber ) );
    _lineEdit->setToolTip( QString::fromStdString( formattedNumber ) );
}

void VDoubleLineEdit::SetValue( double value ) {
    _value = value;
    std::string formattedNumber = _formatValue( _value );
    _lineEdit->setText( QString::fromStdString( formattedNumber ) );
    _lineEdit->setToolTip( QString::fromStdString( formattedNumber ) );
}

double VDoubleLineEdit::GetValue() const {
    return _value;
}

void VDoubleLineEdit::_valueChanged() {
    bool legalConversion;
    QString str = _lineEdit->text();

    double value = str.toDouble( &legalConversion );

    if (legalConversion) {
        SetValue( value );
        emit VAbstractLineEdit::ValueChanged( _value );
    }
    else {
        SetValue( _value );
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
