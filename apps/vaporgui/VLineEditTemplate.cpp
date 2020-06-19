#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include "VLineEditTemplate.h"

template <class T>
VLineEditTemplate<T>::VLineEditTemplate( T value, bool useMenu ) 
: VAbstractLineEdit(useMenu),
  _value( value )
{
    std::string formattedNumber = _formatValue( _value );
    _lineEdit->setText( QString::fromStdString( formattedNumber ) );
    _lineEdit->setToolTip( QString::fromStdString( formattedNumber ) );
}

template <class T>
T VLineEditTemplate<T>::GetValue() const { return _value; }

template <class T>
void VLineEditTemplate<T>::SetValue( T value ) { 
    _value = value;
    std::string formattedNumber = _formatValue( _value );
    _lineEdit->setText( QString::fromStdString( formattedNumber ) );
    _lineEdit->setToolTip( QString::fromStdString( formattedNumber ) );
}

template <class T>
void VLineEditTemplate<T>::_valueChanged() { 
    bool legalConversion;
    QString str = _lineEdit->text();
    double value = str.toDouble( &legalConversion );
    if (legalConversion) {
        SetValue( (T)value );
        emit VAbstractLineEdit::ValueChanged( _value ); 
    }
    else {
        SetValue( _value );
    }
}

template <class T>
std::string VLineEditTemplate<T>::_formatValue( T value ) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision( _decimalDigits );
    if ( _sciNotation ) {
        stream << std::scientific;
        stream << (double)_value;
    }
    else {
        stream << _value;
    }
    return stream.str();
}

VLineEditTemplate<std::string>::VLineEditTemplate( const std::string& value ) 
: VAbstractLineEdit(),
  _value( value )
{
    _lineEdit->setText( QString::fromStdString( value ) );
    _lineEdit->setToolTip( QString::fromStdString( value ) );
}
        
void VLineEditTemplate<std::string>::SetValue( const std::string& value ) { 
    _value = value; 
    _lineEdit->setText( QString::fromStdString( value ) );
    _lineEdit->setToolTip( QString::fromStdString( value ) );
}

std::string VLineEditTemplate<std::string>::GetValue() const { 
    return _value; 
}

void VLineEditTemplate<std::string>::_valueChanged() { 
    std::string value = _lineEdit->text().toStdString();
    if ( value != _value ) {
        _value = value;
        emit VAbstractLineEdit::ValueChanged( _value ); 
    }
}
