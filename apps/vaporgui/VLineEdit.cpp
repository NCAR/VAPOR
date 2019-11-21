
#include <iostream>
#include <string>

#include "VLineEdit.h"

VLineEdit::VLineEdit( const std::string& value )
: VContainer( this ),
  _value( value ),
  _isDouble( false )
{
    _lineEdit = new QLineEdit;
    SetValue( _value );
    layout()->addWidget(_lineEdit);

    connect( _lineEdit, SIGNAL( editingFinished() ),
        this, SLOT( emitLineEditChanged() ) );
}

void VLineEdit::SetValue( const std::string& value ) {
    // see if value is a valid double
    if (_isDouble) {
        try {
            double dValue = std::stod( value );
            _value = value;
        }
        catch (...) {
            std::cerr << "VLineEDit::SetValue failed to set value of " << value << std::endl;
        }
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
