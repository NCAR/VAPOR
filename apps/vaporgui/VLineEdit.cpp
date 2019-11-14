#include "VLineEdit.h"

VLineEdit2::VLineEdit2( const std::string& value )
: VContainer( this )
{
    _lineEdit = new QLineEdit;
    SetValue( value );
    layout()->addWidget(_lineEdit);

    connect( _lineEdit, SIGNAL( editingFinished() ),
        this, SLOT( emitLineEditChanged() ) );
}

void VLineEdit2::SetValue( const std::string& value ) {
    _lineEdit->blockSignals(true);
    _lineEdit->setText( QString::fromStdString(value) );
    _lineEdit->blockSignals(false);
}

std::string VLineEdit2::GetValue() const {
    return _lineEdit->text().toStdString();
}

void VLineEdit2::emitLineEditChanged() {
    std::string value = GetValue();
    emit ValueChanged( value );
}
