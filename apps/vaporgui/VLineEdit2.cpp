#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include <QMenu>

#include "VActions.h"
#include "VLineEdit2.h"
#include "ErrorReporter.h"

VLineEdit2::VLineEdit2( const std::string& value ) : 
    VContainer(), 
    _lineEdit( new QLineEdit )
{
    connect( _lineEdit, &QLineEdit::editingFinished, this, &VLineEdit2::_emitChange );
    layout()->addWidget( _lineEdit );
}

void VLineEdit2::_emitChange() {
    std::string value = _lineEdit->text().toStdString();
    emit ValueChanged( value );
}

void VLineEdit2::SetValue( const std::string& value ) {
    _lineEdit->setText( QString::fromStdString( value ) );
}

std::string VLineEdit2::GetValue() const {
    return _lineEdit->text().toStdString();
}

VNumericLineEdit::VNumericLineEdit( bool useMenu ) : 
    VLineEdit2( "" ), 
    //_lineEdit( new QLineEdit ),
    _sciNotation( false ),
    _decimalDigits( 6 )
{
    layout()->addWidget( _lineEdit );
    _lineEdit->setContextMenuPolicy( Qt::NoContextMenu );

    _menu = new QMenu(this);
    _sciNotationAction = new VCheckBoxAction( "Scientific notation", _sciNotation );
    std::cout << "sci " << _sciNotationAction << std::endl;
    connect( _sciNotationAction, &VCheckBoxAction::clicked,
        this, &VIntLineEdit::SetSciNotation );
    _menu->addAction( _sciNotationAction );

    _decimalAction = new VSpinBoxAction( "Decimal digits", _decimalDigits);
    std::cout << "dec " << _decimalAction << std::endl;
    connect( _decimalAction, &VSpinBoxAction::editingFinished,
        this, &VNumericLineEdit::_decimalDigitsChanged );
    _menu->addAction(_decimalAction);

    if ( useMenu ) { 
        _lineEdit->setContextMenuPolicy( Qt::CustomContextMenu );
        connect( _lineEdit, &QLineEdit::customContextMenuRequested,
            this, &VNumericLineEdit::_showMenu );
    }
}

void VNumericLineEdit::SetNumDigits( int digits ) {
    _decimalDigits = digits;
    Reformat();
    emit FormatChanged();
}

void VNumericLineEdit::SetSciNotation( bool sciNotation ) {
    _sciNotation = sciNotation ;
    Reformat();
    emit FormatChanged();
}

void VNumericLineEdit::_decimalDigitsChanged( int digits ) {
    _decimalDigits = digits;
    Reformat();
    emit FormatChanged();
}

void VNumericLineEdit::_showMenu( const QPoint& pos ) {
    QPoint globalPos = mapToGlobal(pos);
    _menu->exec(globalPos);
}

VIntLineEdit::VIntLineEdit( int value, bool useMenu ) : 
    VNumericLineEdit( useMenu ),
    _value( value )
{
    std::cout << "   " << _decimalAction << std::endl;
    std::cout << "   " << _sciNotationAction << std::endl;

    connect( _lineEdit, &QLineEdit::editingFinished, this, &VIntLineEdit::_emitChange );
    SetValue( _value );

    connect( _sciNotationAction, &VCheckBoxAction::clicked,
        this, &VIntLineEdit::_enableDecimalPrecision );
}

void VIntLineEdit::_enableDecimalPrecision( bool enabled ) {
    _decimalAction->setEnabled( enabled );
}

void VIntLineEdit::_emitChange() {
    bool legalInt;
    QString str = _lineEdit->text();

    double value = str.toDouble( &legalInt );
    
    if ( legalInt ) {  // Conversion successful, update _value
        SetValue( (int)value );
        emit ValueChanged( _value );
    }
    else {            // Conversion failed, reset to previous value
        SetValue( _value );
    }
}

void VIntLineEdit::Reformat() {
    QString qValue = QString::number( _value );
    if ( _sciNotation ) {
        std::stringstream stream;
        stream << std::fixed << std::setprecision( _decimalDigits );
        stream << std::scientific;
        stream << (float)_value << std::endl;
        qValue = QString::fromStdString( stream.str() );
    }
    _lineEdit->setText( qValue );
}

void VIntLineEdit::SetValue( int value ) {
    _value = value;
    Reformat();
}

int VIntLineEdit::GetValue() const {
    return _value;
}

VDoubleLineEdit::VDoubleLineEdit( double value, bool useMenu ) : 
    VNumericLineEdit( useMenu ), 
    _value( value )
{
    connect( _lineEdit, &QLineEdit::editingFinished, this, &VDoubleLineEdit::_emitChange );
    SetValue( _value );
}

double VDoubleLineEdit::GetValue() const {
    return _value;
}

void VDoubleLineEdit::SetValue( double value ) {
    _value = value;
    Reformat();
}

void VDoubleLineEdit::_emitChange() {
    bool legalDouble;
    QString str = _lineEdit->text();

    double value = str.toDouble( &legalDouble );
    
    if ( legalDouble ) {       // Conversion successful, update _value and emit
        SetValue( value );
        emit ValueChanged( _value );
    }
    else {                     // Conversion failed, reinstate previous _value
        SetValue( _value );
    }
}

void VDoubleLineEdit::Reformat() {
    std::stringstream stream;
    stream << std::fixed << std::setprecision( _decimalDigits );
    if (_sciNotation ) {
        stream << std::scientific;
    }
    stream << _value << std::endl;

    QString qValue = QString::fromStdString( stream.str() );
    _lineEdit->setText( qValue );
}

