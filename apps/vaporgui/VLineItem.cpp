#include "VLineItem.h"
#include <QHBoxLayout>
#include <QLabel>

const int VLineItem::_LEFT_MARGIN = 0;
const int VLineItem::_TOP_MARGIN = 0;
const int VLineItem::_RIGHT_MARGIN = 0;
const int VLineItem::_BOTTOM_MARGIN = 0;

VLineItem::VLineItem(const std::string &label, QLayoutItem *centerItem, QWidget *rightWidget) : VLineItem(label)
{
    layout()->addItem(centerItem);
    layout()->addWidget(rightWidget);
}

VLineItem::VLineItem(const std::string &label, QWidget *centerWidget, QWidget *rightWidget) : VLineItem(label)
{
    layout()->addWidget(centerWidget);
    layout()->addWidget(rightWidget);
}

VLineItem::VLineItem(const std::string &label, QWidget *rightWidget) : VLineItem(label, (QLayoutItem *)new QSpacerItem(108, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), rightWidget) {}

VLineItem::VLineItem(const std::string &label)
{
    setLayout(new QHBoxLayout);
    layout()->setContentsMargins(_LEFT_MARGIN, _TOP_MARGIN, _RIGHT_MARGIN, _BOTTOM_MARGIN);

    layout()->addWidget(new QLabel(label.c_str()));

    // QWidgetItem broke in Qt5 so this class had to be refactored not to use it.
}
