#include "PLabel.h"
#include "VLabel.h"

PLabel::PLabel(const std::string &text) : PWidget("", _label = new VLabel(text)) {}
