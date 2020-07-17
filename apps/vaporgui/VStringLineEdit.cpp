#include <string>
#include <sstream>
#include <iomanip>

#include <QString>
#include <QLineEdit>

#include <VStringLineEdit.h>

VStringLineEdit::VStringLineEdit( std::string value ) : 
    VContainer(),
    _lineEdit( new QLineEdit ),
    _strValue( value )
{
    layout()->addWidget( _lineEdit );
    connect( _lineEdit, SIGNAL( editingFinished() ),
        this, SLOT( _valueChanged() ) );

    _lineEdit->setContextMenuPolicy( Qt::DefaultContextMenu );
    _lineEdit->setText( QString::fromStdString( value ) );
    _lineEdit->setToolTip( QString::fromStdString( value ) );
}

void VStringLineEdit::SetValueString( std::string value ) {
    _strValue = value;
    _lineEdit->setText( QString::fromStdString( _strValue ) );
    _lineEdit->setToolTip( QString::fromStdString( _strValue ) );
}

std::string VStringLineEdit::GetValueString() const {
    return _strValue;
}

void VStringLineEdit::_valueChanged() {
    std::string value = _lineEdit->text().toStdString();
    if ( value != _strValue ) {
        _strValue = value;
        emit ValueChanged( _strValue );
    }
}

std::string VStringLineEdit::_getText() const {
    std::string value = _lineEdit->text().toStdString();
    return value;
}
