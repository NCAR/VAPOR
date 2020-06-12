#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include <QMenu>

#include "VActions.h"
#include "VLineEdit2.h"
#include "ErrorReporter.h"

VIntLineEdit::VIntLineEdit( int value ) : 
    VContainer(), 
    _lineEdit( new QLineEdit ),
    _value( value ) 
{
    connect( _lineEdit, &QLineEdit::editingFinished, this, &VIntLineEdit::emitChange );
    layout()->addWidget( _lineEdit );
    SetValue( _value );
}

void VIntLineEdit::emitChange() {
    bool legalInt;
    auto str = _lineEdit->text();

    double value = str.toDouble( &legalInt );
    
    if ( legalInt ) {
        _value = (int)value;
        emit ValueChanged( _value );
    }
    else {
        _lineEdit->setText( QString::number( _value ) );
    }
}

void VIntLineEdit::SetValue( int value ) {
    _value = value;
    _lineEdit->setText( QString::number( _value ) );
}

int VIntLineEdit::GetValue() const {
    return _value;
}

VDoubleLineEdit::VDoubleLineEdit( double value ) : 
    VContainer(), 
    _lineEdit( new QLineEdit ),
    _value( value ) 
{
    connect( _lineEdit, &QLineEdit::editingFinished, this, &VDoubleLineEdit::emitChange );
    layout()->addWidget( _lineEdit );
    SetValue( _value );
}

void VDoubleLineEdit::emitChange() {
    bool legalDouble;
    auto str = _lineEdit->text();

    double value = str.toDouble( &legalDouble );
    
    if ( legalDouble ) {
        _value = value;
        emit ValueChanged( _value );
    }
    else {
        _lineEdit->setText( QString::number( _value ) );
    }
}

void VDoubleLineEdit::SetValue( double value ) {
    _value = value;
    _lineEdit->setText( QString::number( _value ) );
}

double VDoubleLineEdit::GetValue() const {
    return _value;
}

VStringLineEdit::VStringLineEdit( const std::string& value ) : 
    VContainer(), 
    _lineEdit( new QLineEdit ),
    _value( value ) 
{
    connect( _lineEdit, &QLineEdit::editingFinished, this, &VStringLineEdit::emitChange );
    layout()->addWidget( _lineEdit );
    SetValue( _value );
}

void VStringLineEdit::emitChange() {
    _value = _lineEdit->text().toStdString();
    emit ValueChanged( _value );
}

void VStringLineEdit::SetValue( const std::string& value ) {
    _value = value;
    _lineEdit->setText( QString::fromStdString( _value ) );
}

std::string VStringLineEdit::GetValue() const {
    return _value;
}
