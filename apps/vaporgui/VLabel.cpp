#include "VLabel.h"
#include <QLabel>

VLabel::VLabel(const std::string &text) : VContainer(_ql = new QLabel)
{
    _ql->setWordWrap(true);
    SetText(text);
}

void VLabel::SetText(const std::string &text) { _ql->setText(QString::fromStdString(text)); }

void VLabel::MakeSelectable() { _ql->setTextInteractionFlags(_ql->textInteractionFlags() | Qt::TextSelectableByMouse); }

VHyperlink::VHyperlink(const std::string &text, const std::string &url) : VLabel() {
    std::string link = "<a href=\"" + url + "\">" + text + "</a>";
    _ql->setText(QString::fromStdString(link));
    _ql->setTextFormat(Qt::RichText);
    _ql->setTextInteractionFlags(Qt::TextBrowserInteraction);
    _ql->setOpenExternalLinks(true); 
}
