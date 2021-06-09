#include "VLabel.h"
#include <QLabel>

VLabel::VLabel(const std::string &text) : VContainer(_ql = new QLabel)
{
    _ql->setWordWrap(true);
    SetText(text);
}

void VLabel::SetText(const std::string &text) { _ql->setText(QString::fromStdString(text)); }

void VLabel::MakeSelectable() { _ql->setTextInteractionFlags(_ql->textInteractionFlags() | Qt::TextSelectableByMouse); }
