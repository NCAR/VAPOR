#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include <QMenu>

#include "VActions.h"
#include "VLineEdit2.h"
#include "ErrorReporter.h"

VLineEdit2::VLineEdit2() : 
    VContainer(), 
    _lineEdit( new QLineEdit )
{
    connect( _lineEdit, &QLineEdit::editingFinished, 
        this, &VLineEdit2::_emitChange );
    layout()->addWidget( _lineEdit );
}


VStringLineEdit::VStringLineEdit( const std::string& value ) :
    VLineEdit2()
{
    _value = value;
    _lineEdit->setText( QString::fromStdString( _value ) );
}

void VStringLineEdit::SetValue( const std::string& value ) {
    if ( QObject::sender() == _lineEdit ) {
        _value = _lineEdit->text().toStdString();
        emit ValueChanged( _value );
    }

    else {
        _value = value;
        _lineEdit->setText( QString::fromStdString( value ) );
    }
}

std::string VStringLineEdit::GetValue() const {
    return _value;
}

void VStringLineEdit::_emitChange() {
    _value = _lineEdit->text().toStdString();
    emit ValueChanged( _value );
}

VNumericLineEdit::VNumericLineEdit( bool useMenu ) : 
    VLineEdit2( ), 
    _sciNotation( false ),
    _decimalDigits( 6 )
{
    layout()->addWidget( _lineEdit );
    _lineEdit->setContextMenuPolicy( Qt::NoContextMenu );

    _menu = new QMenu(this);
    _sciNotationAction = new VCheckBoxAction( "Scientific notation", _sciNotation );
    connect( _sciNotationAction, &VCheckBoxAction::clicked,
        this, &VIntLineEdit::SetSciNotation );
    _menu->addAction( _sciNotationAction );

    _decimalAction = new VSpinBoxAction( "Decimal digits", _decimalDigits);
    connect( _decimalAction, &VSpinBoxAction::editingFinished,
        this, &VNumericLineEdit::_decimalDigitsChanged );
    _menu->addAction(_decimalAction);

    if ( useMenu ) { 
        _lineEdit->setContextMenuPolicy( Qt::CustomContextMenu );
        connect( _lineEdit, &QLineEdit::customContextMenuRequested,
            this, &VNumericLineEdit::_showMenu );
    }
}

bool VNumericLineEdit::GetSciNotation() const {
    return _sciNotation;
}

void VNumericLineEdit::SetSciNotation( bool sciNotation ) {
    _sciNotation = sciNotation ;
    Reformat();
    emit FormatChanged();
}

int VNumericLineEdit::GetNumDigits() const {
    return _decimalDigits;
}

void VNumericLineEdit::SetNumDigits( int digits ) {
    _decimalDigits = digits;
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
