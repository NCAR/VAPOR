#pragma once

#include <QLineEdit>

#include "VContainer.h"

class VNumericFormatMenu;
class VCheckBoxAction;
class VSpinBoxAction;

class VAbstractLineEdit : public VContainer {
    Q_OBJECT

protected:
    explicit VAbstractLineEdit( bool useNumericMenu = false);

public slots:
    virtual void SetValue( int value );
    virtual void SetValue( double value );
    virtual void SetValue( const std::string& value );

    int GetNumDigits() const;
    void SetNumDigits( int digits );

    bool GetSciNotation() const;
    void SetSciNotation( bool sciNotation );

public slots:
    virtual void _valueChanged() = 0;
    void _setDecimalDigits( int digits ); 
    void _setSciNotation( bool sciNotation );

protected:
    QLineEdit*          _lineEdit;
    VNumericFormatMenu* _menu;
    VCheckBoxAction*    _sciNotationAction;
    VSpinBoxAction*     _decimalAction;

    bool _sciNotation;
    int  _decimalDigits;

    void _showMenu( const QPoint& pos );

signals:
    void ValueChanged( int value );
    void ValueChanged( double value );
    void ValueChanged( const std::string& value );
    void DecimalDigitsChanged( int decimalDigits );
    void SciNotationChanged( bool sciNotation );
};

/*
template <class T>
class VLineEditTemplate : public VAbstractLineEdit {

    public:
        T GetValue() const { return _value; }

        virtual void SetValue( T value ) { 
            _value = value;
            std::string formattedNumber = _formatValue( _value );
            _lineEdit->setText( QString::fromStdString( formattedNumber ) );
            _lineEdit->setToolTip( QString::fromStdString( formattedNumber ) );
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
            return stream.str();
        }
        
        T _value;

};

template <>
class VLineEditTemplate <std::string> : public VAbstractLineEdit {
    protected:
        VLineEditTemplate( const std::string& value ) 
        : VAbstractLineEdit(),
          _value( value )
        {
            _lineEdit->setText( QString::fromStdString( value ) );
            _lineEdit->setToolTip( QString::fromStdString( value ) );
        }
        
        virtual void SetValue( const std::string& value ) { 
            _value = value; 
            _lineEdit->setText( QString::fromStdString( value ) );
            _lineEdit->setToolTip( QString::fromStdString( value ) );
        }

        std::string GetValue() const { return _value; }

    //protected:
        std::string _value;
        virtual void _valueChanged() { 
            std::string value = _lineEdit->text().toStdString();
            if ( value != _value ) {
                _value = value;
                emit VAbstractLineEdit::ValueChanged( _value ); 
            }
        }
};

class VIntLineEdit3 : public VLineEditTemplate<int> {
    public:
        VIntLineEdit3( int value, bool useMenu=true ) 
        : VLineEditTemplate<int>(value, useMenu) {}
};

class VDoubleLineEdit3 : public VLineEditTemplate<double> {
    public:
        VDoubleLineEdit3( double value, bool useMenu=true ) 
        : VLineEditTemplate<double>(value, useMenu) {}
};

class VStringLineEdit3 : public VLineEditTemplate<std::string> {
    public:
        VStringLineEdit3( const std::string& value ) 
        : VLineEditTemplate<std::string>(value) {}
};*/
