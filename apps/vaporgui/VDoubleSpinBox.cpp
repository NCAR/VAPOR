#include "VDoubleSpinBox.h"

VDoubleSpinBox2::VDoubleSpinBox2( double min, double max )
: VContainer( this )
{
    _spinBox = new QDoubleSpinBox;
    SetRange( min, max );
    SetValue( min );
    layout()->addWidget(_spinBox);

    connect( _spinBox, SIGNAL( editingFinished() ),
        this, SLOT( emitSpinBoxChanged() ) );
}


void VSpinBox2::SetValue( double value ) {
    _spinBox->blockSignals(true);
    _spinBox->setValue( value );
    _spinBox->blockSignals(false);
}

void VSpinBox2::SetRange( double min, double max ) {
    _spinBox->setRange( min, max );
}

double VSpinBox2::GetValue() const {
    return _spinBox->value();
}

void VSpinBox2::emitSpinBoxChanged() {
    emit ValueChanged( GetValue() );
}
