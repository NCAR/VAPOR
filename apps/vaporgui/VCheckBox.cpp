#include "VCheckBox.h"

VCheckBox2::VCheckBox2( bool checked )
: VContainer( this )
{
    _checkBox = new QCheckBox;
    SetValue( checked );
    layout()->addWidget(_checkBox);

    connect( _checkBox, SIGNAL( clicked( bool ) ),
        this, SLOT( emitCheckBoxChanged( bool ) ) );
}


// Stas thinks that we should have setValues and setValue instead of Update
//
void VCheckBox2::SetValue( bool checked ) {
    _checkBox->blockSignals(true);
    _checkBox->setChecked( checked );
    _checkBox->blockSignals(false);
}

bool VCheckBox2::GetValue() const {
    return _checkBox->isChecked();
}

void VCheckBox2::emitCheckBoxChanged( bool checked ) {
    emit ValueChanged( checked );
}
