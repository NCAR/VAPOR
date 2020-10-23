#include "PButton.h"
#include "VPushButton.h"

PButton::PButton(std::string label, Callback cb) : PWidget("", _button = new VPushButton(label)), _cb(cb) { QObject::connect(_button, &VPushButton::ButtonClicked, this, &PButton::clicked); }

void PButton::clicked() { _cb(getParams()); }
