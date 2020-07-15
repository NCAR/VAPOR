#include <string>
#include <sstream>
#include <iomanip>

#include <QString>
#include <QLineEdit>

#include <VStringLineEdit.h>

VStringLineEdit::VStringLineEdit( std::string value ) : 
    VContainer(),
    _lineEdit( new QLineEdit ),
    _value( value )
{
    layout()->addWidget( _lineEdit );
    connect( _lineEdit, SIGNAL( editingFinished() ),
        this, SLOT( _valueChanged() ) );

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
        emit ValueChanged( _value );
    }
}
