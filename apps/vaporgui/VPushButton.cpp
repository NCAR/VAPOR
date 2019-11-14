#include "VPushButton.h"

VPushButton2::VPushButton2(
    const std::string& buttonText
) : VContainer( this )
{
    _pushButton = new QPushButton( QString::fromStdString( buttonText ) );
    layout()->addWidget(_pushButton);

    connect( _pushButton, SIGNAL( clicked( bool ) ),
        this, SLOT( emitButtonClicked() ) );
}

void VPushButton2::emitButtonClicked() {
    emit ButtonClicked();
}
