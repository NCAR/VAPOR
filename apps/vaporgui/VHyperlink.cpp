#include "VHyperlink.h"
#include <QLabel>

VHyperlink::VHyperlink(const std::string &text, const std::string &url, bool bullet) : VLabel() {
    std::string link = "<a href=\"" + url + "\">" + text + "</a>";
    if (bullet == true) link.insert(0, "• ");
    _ql->setText(QString::fromStdString(link));
    _ql->setTextFormat(Qt::RichText);
    _ql->setTextInteractionFlags(Qt::TextBrowserInteraction);
    _ql->setOpenExternalLinks(true);
}
