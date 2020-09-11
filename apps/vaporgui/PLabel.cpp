#include "PLabel.h"
#include <QLabel>

PLabel::PLabel(const std::string &text)
: PWidget("", new QLabel(QString::fromStdString(text)))
{}
