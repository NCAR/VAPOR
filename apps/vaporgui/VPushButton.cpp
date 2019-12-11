#include "VPushButton.h"

VPushButton::VPushButton(
    const std::string& buttonText
) : VContainer()
{
    _pushButton = new QPushButton( QString::fromStdString( buttonText ) );
    layout()->addWidget(_pushButton);

    connect( _pushButton, SIGNAL( clicked( bool ) ),
        this, SLOT( emitButtonClicked() ) );
}

void VPushButton::emitButtonClicked() {
    emit ButtonClicked();
}
