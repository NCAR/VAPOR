#include "VLineComboBox.h"

VLineComboBox::VLineComboBox( 
    const std::string& label
) : VLineItem( label, _combo = new VComboBox() )
{
    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );

    connect( _combo, SIGNAL( ValueChanged( std::string ) ),
        this, SLOT( emitComboChanged( std::string ) ) );
}

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
