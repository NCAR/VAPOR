#pragma once

#include <iostream>

#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include "VAbstractLineEdit.h"

template <class T>
class VLineEditTemplate : public VAbstractLineEdit {
    public:
        virtual void SetValue( T value ) {
            _value = value;
            std::string formattedNumber = _formatValue( _value );
            _lineEdit->setText( QString::fromStdString( formattedNumber ) );
            _lineEdit->setToolTip( QString::fromStdString( formattedNumber ) );
        }
    
        virtual T GetValue() const {
            return _value;
        }

    protected:
        VLineEditTemplate( T value, bool useMenu=true )
        : VAbstractLineEdit(useMenu),
          _value( value )
        {
            std::string formattedNumber = _formatValue( _value );
            _lineEdit->setText( QString::fromStdString( formattedNumber ) );
            _lineEdit->setToolTip( QString::fromStdString( formattedNumber ) );
        }

        virtual void _valueChanged() {
            std::cout << "value changed" << std::endl;
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

        std::string _formatValue( T value ) {
            std::stringstream stream;
            stream << std::fixed << std::setprecision( _decimalDigits );
            if ( _sciNotation ) {
                stream << std::scientific;
                stream << (double)_value;
            }
            else {
                stream << _value;
            }
            std::cout << "formatted to " << stream.str() << std::endl;
            return stream.str();
        }
        
        T _value;
};

template <>
class VLineEditTemplate <std::string> : public VAbstractLineEdit {
    public:
        virtual void SetValue( const std::string& value ) {
            _value = value;
            _lineEdit->setText( QString::fromStdString( value ) );
            _lineEdit->setToolTip( QString::fromStdString( value ) );
        }

        virtual std::string GetValue() const {
            return _value;
        }

    protected:
        VLineEditTemplate( const std::string& value )
        : VAbstractLineEdit(),
          _value( value )
        {
            _lineEdit->setText( QString::fromStdString( value ) );
            _lineEdit->setToolTip( QString::fromStdString( value ) );
        }
        
        virtual void _valueChanged() {
            std::string value = _lineEdit->text().toStdString();
            if ( value != _value ) {
                _value = value;
                emit VAbstractLineEdit::ValueChanged( _value );
            }
        }

        std::string _value;
};

class VIntLineEdit : public VLineEditTemplate<int> {
    public:
        VIntLineEdit( int value, bool useMenu=true ) 
        : VLineEditTemplate<int>(value, useMenu) {}
};

class VDoubleLineEdit : public VLineEditTemplate<double> {
    public:
        VDoubleLineEdit( double value, bool useMenu=true ) 
        : VLineEditTemplate<double>(value, useMenu) {}
};

class VStringLineEdit : public VLineEditTemplate<std::string> {
    public:
        VStringLineEdit( const std::string& value ) 
        : VLineEditTemplate<std::string>(value) {}
};
