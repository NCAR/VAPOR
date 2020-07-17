#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include "VDoubleLineEdit.h"

VDoubleLineEdit::VDoubleLineEdit( double value, bool useMenu ) : 
    VNumericLineEdit(),
    _value( value )
{
    // Disconnect the QLineEdit's interaction with VStringLineEdit::_valueChanged(),
    // and reconnect it to VDoubleLineEdit::_valueChanged()
    disconnect( _lineEdit, SIGNAL( editingFinished() ),
        this, SLOT( _valueChanged() ) );
    connect( _lineEdit, SIGNAL( editingFinished() ),
        this, SLOT( _valueChanged() ) );

    std::string formattedNumber = _formatValue( _value );
    _lineEdit->setText( QString::fromStdString( formattedNumber ) );
    _lineEdit->setToolTip( QString::fromStdString( formattedNumber ) );
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

    SetValueString( formattedNumber );
}

double VDoubleLineEdit::GetValueDouble() const {
    return _value;
}

void VDoubleLineEdit::_valueChanged() {
    bool legalConversion;
    QString str = _lineEdit->text();
    double value = str.toDouble( &legalConversion );

    if (legalConversion) {
        SetValueDouble( value );
        emit ValueChanged( _value );
    }
    else {
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
