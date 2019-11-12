#include "VSpinBox.h"

VSpinBox2::VSpinBox2( int min, int max )
: VContainer( this )
{
    _spinBox = new QSpinBox;
    SetRange( min, max );
    SetValue( min );
    layout()->addWidget(_spinBox);

    //connect( _spinBox, SIGNAL( valueChanged( int ) ),
    connect( _spinBox, SIGNAL( editingFinished() ),
        this, SLOT( emitSpinBoxChanged() ) );
}


void VSpinBox2::SetValue( int value ) {
    _spinBox->blockSignals(true);
    _spinBox->setValue( value );
    _spinBox->blockSignals(false);
}

void VSpinBox2::SetRange( int min, int max ) {
    _spinBox->setRange( min, max );
}

int VSpinBox2::GetValue() const {
    return _spinBox->value();
}

//void VSpinBox2::emitSpinBoxChanged( int value ) {
void VSpinBox2::emitSpinBoxChanged() {
    emit ValueChanged( GetValue() );
}
