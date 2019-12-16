#include "VPushButton.h"

VPushButton::VPushButton(const std::string &buttonText) : VContainer()
{
    _pushButton = new QPushButton(QString::fromStdString(buttonText));
    layout()->addWidget(_pushButton);

    // We need to use SIGNAL/SLOT macros here because the arguments
    // of the signal and slot do not match
    connect(_pushButton, SIGNAL(clicked(bool)), this, SLOT(emitButtonClicked()));
}

void VPushButton::emitButtonClicked() { emit ButtonClicked(); }
