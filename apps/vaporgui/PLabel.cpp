#include "PLabel.h"
#include <QLabel>

PLabel::PLabel(const std::string &text) : PWidget("", _label = new QLabel(QString::fromStdString(text))) { _label->setWordWrap(true); }
