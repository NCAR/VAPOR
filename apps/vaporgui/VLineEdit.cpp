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
  _decDigits( 10 )
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

    //_lineEdit->setContextMenuPolicy( Qt::CustomContextMenu );
    //connect( _lineEdit, &QLineEdit::customContextMenuRequested,
    //    this, &VLineEdit::ShowContextMenu );
}

void VLineEdit::SetValue( double value ) {
    std::stringstream stream;
    if (_menuEnabled) {
        stream << std::fixed << std::setprecision( _decDigits );
        if (_scientific)
            stream << std::scientific;
    }
    stream << value << std::endl;
    _value = stream.str();
    
    _lineEdit->blockSignals(true);
    _lineEdit->setText( QString::fromStdString(_value) );
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

void VLineEdit::SetIsDouble( bool isDouble ) {
    UseDoubleMenu();
}

void VLineEdit::SetReadOnly(bool b)
{
    _lineEdit->setReadOnly(b);
}

void VLineEdit::emitLineEditChanged() {
    std::string value = _lineEdit->text().toStdString();
    SetValue( value );
    emit ValueChanged( _value );
}

void VLineEdit::ShowContextMenu( const QPoint& pos ) {
std::cout << "void VLineEdit::ShowContextMenu( const QPoint& pos ) {" << std::endl;
    if ( !_menuEnabled )
        return;

    QMenu menu;
    
    VSpinBoxAction* decimalAction = new VSpinBoxAction(tr("Decimal digits"), _decDigits);
    connect( decimalAction, &VSpinBoxAction::editingFinished,
        this, &VLineEdit::_decimalDigitsChanged );
    menu.addAction(decimalAction);

    VCheckBoxAction* checkBoxAction = new VCheckBoxAction(tr("Scientific"), _scientific);
    connect( checkBoxAction, &VCheckBoxAction::clicked,
        this, &VLineEdit::_scientificClicked );
    menu.addAction(checkBoxAction);

    QPoint globalPos = _lineEdit->mapToGlobal(pos);
    menu.exec(globalPos);
}

void VLineEdit::_decimalDigitsChanged( int value ) {
    _decDigits = value;
    SetValue( _value );
}

void VLineEdit::_scientificClicked( bool value ) {
    _scientific = value;
    SetValue( _value );
}
