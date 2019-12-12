#include "VSpinBox.h"

VSpinBox::VSpinBox( int min, int max )
: VContainer()
{
    _spinBox = new QSpinBox;
    SetRange( min, max );
    SetValue( min );
    layout()->addWidget(_spinBox);

    connect( _spinBox, &QSpinBox::editingFinished,
        this, &VSpinBox::emitSpinBoxChanged );
}


void VSpinBox::SetValue( int value ) {
    _spinBox->blockSignals(true);
    _spinBox->setValue( value );
    _spinBox->blockSignals(false);
}

void VSpinBox::SetRange( int min, int max ) {
    _spinBox->setRange( min, max );
}

int VSpinBox::GetValue() const {
    return _spinBox->value();
}

void VSpinBox::emitSpinBoxChanged() {
    emit ValueChanged( GetValue() );
}
