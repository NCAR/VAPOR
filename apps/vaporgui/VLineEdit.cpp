#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include <QMenu>

#include "VActions.h"
#include "VLineEdit.h"
#include "ErrorReporter.h"

VLineEdit::VLineEdit( const std::string& value )
: VContainer(),
  _value( value ),
  _scientific( false ),
  _menuEnabled( false ),
  _decDigits( 5 )
{
    _lineEdit = new QLineEdit;
    SetValue( _value );
    layout()->addWidget(_lineEdit);

    _lineEdit->setContextMenuPolicy( Qt::NoContextMenu );
    connect( _lineEdit, &QLineEdit::editingFinished,
        this, &VLineEdit::emitLineEditChanged);
}

void VLineEdit::UseDoubleMenu() {
    _menuEnabled = true;

    _lineEdit->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( _lineEdit, &QLineEdit::customContextMenuRequested,
        this, &VLineEdit::ShowContextMenu );
}

void VLineEdit::SetValue( int value ) {
    std::stringstream stream;
    if (_scientific) {
        stream << std::scientific;
    }
    stream << value << std::endl;
    _value = stream.str();
    
    _lineEdit->blockSignals(true);
    _lineEdit->setText( QString::fromStdString(_value) );
    _lineEdit->blockSignals(false);
}

void VLineEdit::SetValue( double value ) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision( _decDigits );
    if (_scientific) {
        stream << std::scientific;
    }
    stream << value << std::endl;
    _value = stream.str();
    
    _lineEdit->blockSignals(true);
    _lineEdit->setText( QString::fromStdString(_value) );
    _lineEdit->setText( QString::fromStdString("foobar") );
    _lineEdit->blockSignals(false);
}

void VLineEdit::SetValue( const std::string& value ) {
    _value = value;

    _lineEdit->blockSignals(true);
    _lineEdit->setText( QString::fromStdString(_value) );
    _lineEdit->blockSignals(false);
}

std::string VLineEdit::GetValue() const {
    return _value;    
}

void VLineEdit::SetReadOnly(bool b) {
    _lineEdit->setReadOnly(b);
}

void VLineEdit::emitLineEditChanged() {
    std::string value = _lineEdit->text().toStdString();
    SetValue( value );
    //emit ValueChanged( _value );
    //emit ValueChanged( stod( _value ) );
    //emit ValueChanged( stoi( _value ) );
}

void VLineEdit::ShowContextMenu( const QPoint& pos ) {
    if ( !_menuEnabled )
        return;

    QMenu menu;
    
    VSpinBoxAction* decimalAction = new VSpinBoxAction( "Decimal digits", _decDigits);
    connect( decimalAction, &VSpinBoxAction::editingFinished,
        this, &VLineEdit::_decimalDigitsChanged );
    menu.addAction(decimalAction);

    VCheckBoxAction* checkBoxAction = new VCheckBoxAction( "Scientific", _scientific);
    connect( checkBoxAction, &VCheckBoxAction::clicked,
        this, &VLineEdit::SetScientific );
    menu.addAction(checkBoxAction);

    QPoint globalPos = _lineEdit->mapToGlobal(pos);
    menu.exec(globalPos);
}

void VLineEdit::_decimalDigitsChanged( int value ) {
    _decDigits = value;
    SetValue( stod( _value ) );
}

void VLineEdit::SetScientific( bool value ) {
    _scientific = value;
    SetValue( stod( _value ) );
}
