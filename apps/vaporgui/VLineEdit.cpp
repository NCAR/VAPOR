#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>

#include <QMenu>

#include "VLineEdit.h"
#include "ErrorReporter.h"

VLineEdit::VLineEdit( const std::string& value )
: VContainer( this ),
  _menu( nullptr ),
  _value( value ),
  _isDouble( false ),
  _scientific( false ),
  _menuEnabled( false ),
  _decDigits( 4 )
{
    _lineEdit = new QLineEdit;
    SetValue( _value );
    layout()->addWidget(_lineEdit);

    connect( _lineEdit, SIGNAL( editingFinished() ),
        this, SLOT( emitLineEditChanged() ) );
}

void VLineEdit::UseMenu() {
    _menu = new QMenu();
    
    SpinBoxAction* decimalAction = new SpinBoxAction(tr("Decimal digits"), _decDigits);
    connect( decimalAction, SIGNAL( editingFinished( int ) ),
        this, SLOT( _decimalDigitsChanged( int ) ) );
    _menu->addAction(decimalAction);

    CheckBoxAction* checkBoxAction = new CheckBoxAction(tr("Scientific"), _scientific);
    connect( checkBoxAction, SIGNAL( clicked( bool ) ),
        this, SLOT( _scientificClicked( bool ) ) );
    _menu->addAction(checkBoxAction);
    
    _lineEdit->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( _lineEdit, SIGNAL( customContextMenuRequested( const QPoint& ) ),
        this, SLOT( ShowContextMenu( const QPoint& ) ) );
}

VLineEdit::~VLineEdit() {
    // Qt is currently crashing if QMenu is given a parent, so delete it here
    // 
    if (_menu != nullptr) delete _menu;
}

void VLineEdit::SetValue( const std::string& value ) {
    // see if value is a valid double
    if (_isDouble) {
        double dValue;
        try {
            dValue = std::stod( value );
        }
        catch (...) {
            MSG_ERR("VLineEDit::SetValue failed to set value of " + value );
        }

        std::stringstream stream;
        if (_menuEnabled) {
            stream << std::fixed << std::setprecision( _decDigits );
            if (_scientific)
                stream << std::scientific;
        }
        stream << dValue << std::endl;
        _value = stream.str();
    }
    else
        _value = value;

    _lineEdit->blockSignals(true);
    _lineEdit->setText( QString::fromStdString(_value) );
    _lineEdit->blockSignals(false);
}

std::string VLineEdit::GetValue() const {
    return _value;    
}

void VLineEdit::SetIsDouble( bool isDouble ) {
    _isDouble = isDouble;

}

void VLineEdit::emitLineEditChanged() {
    std::string value = _lineEdit->text().toStdString();
    SetValue( value );
    emit ValueChanged( _value );
}

void VLineEdit::ShowContextMenu( const QPoint& pos ) {
    if ( !_menuEnabled )
        return;

    QPoint globalPos = _lineEdit->mapToGlobal(pos);
    _menu->exec(globalPos);

    //QAction* selectedItem = _menu->exec(globalPos);
    //if (selectedItem)
    //else
}

void VLineEdit::_decimalDigitsChanged( int value ) {
    _decDigits = value;
    SetValue( _value );
}

void VLineEdit::_scientificClicked( bool value ) {
    _scientific = value;
    SetValue( _value );
}
