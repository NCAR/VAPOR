#include <string>
#include <sstream>
#include <iomanip>

#include <QString>

#include <VStringLineEdit.h>

VStringLineEdit::VStringLineEdit( std::string value ) : 
    VAbstractLineEdit( false ),
    _value( value )
{
    _lineEdit->setContextMenuPolicy( Qt::DefaultContextMenu );
    _lineEdit->setText( QString::fromStdString( value ) );
    _lineEdit->setToolTip( QString::fromStdString( value ) );
}

void VStringLineEdit::SetValue( std::string value ) {
    _value = value;
    _lineEdit->setText( QString::fromStdString( _value ) );
    _lineEdit->setToolTip( QString::fromStdString( _value ) );
}

std::string VStringLineEdit::GetValue() const {
    return _value;
}

void VStringLineEdit::_valueChanged() {
    std::string value = _lineEdit->text().toStdString();
    if ( value != _value ) {
        _value = value;
        emit VAbstractLineEdit::ValueChanged( _value );
    }
}
