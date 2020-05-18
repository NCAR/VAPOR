#include "VLineComboBox.h"

VLineComboBox::VLineComboBox( 
    const std::string& label
) : VLineItem( label, _combo = new VComboBox() )
{
    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );

    // We are forced to used SIGNAL/SLOT macros here because there are two
    // signatures for QComboBox::currentIndexChanged
    connect( _combo, SIGNAL( ValueChanged( std::string ) ),
        this, SLOT( emitComboChanged( std::string ) ) );
}


// Stas thinks that we should have setValues and setValue instead of Update
//
void VLineComboBox::SetOptions( const std::vector<std::string> &values)
{
    _combo->SetOptions( values );
}

void VLineComboBox::SetIndex( int index ) {
    _combo->SetIndex( index );
}

void VLineComboBox::SetValue( const std::string& value ) {
    _combo->SetValue( value );
}

int VLineComboBox::GetCurrentIndex() const {
    return _combo->GetCurrentIndex();
}

std::string VLineComboBox::GetValue() const {
    return _combo->GetValue();
}

int VLineComboBox::GetCount() const {
    return _combo->GetCount();
}

void VLineComboBox::emitComboChanged( const std::string &value ) {
    emit ValueChanged( value );
    emit IndexChanged( _combo->GetCurrentIndex() );
}
