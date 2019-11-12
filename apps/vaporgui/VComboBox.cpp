#include "VComboBox.h"


VComboBox2::VComboBox2( const std::vector<std::string> &values )
: VContainer( this )
{
    _combo = new QComboBox;
    layout()->addWidget(_combo);
    SetOptions( values );

    connect( _combo, SIGNAL( currentIndexChanged( QString ) ),
        this, SLOT( emitComboChanged( QString ) ) );
}


// Stas thinks that we should have setValues and setValue instead of Update
//
void VComboBox2::SetOptions( const std::vector<std::string> &values)
{
    _combo->blockSignals(true);
    _combo->clear();
    for (auto i : values) {
        _combo->addItem( QString::fromStdString(i) );
    }
    _combo->blockSignals(false);
}

int VComboBox2::GetCurrentIndex() const {
    return _combo->currentIndex();
}

std::string VComboBox2::GetCurrentString() const {
    return _combo->currentText().toStdString();
}

void VComboBox2::emitComboChanged( QString value ) {
    emit ValueChanged( value.toStdString() );
}
