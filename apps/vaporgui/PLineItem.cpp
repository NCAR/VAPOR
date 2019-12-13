#include "PLineItem.h"
#include "VLineItem.h"

PLineItem::PLineItem(const std::string &tag, QWidget *centerWidget, QWidget *rightWidget, const std::string &label)
: PWidget(tag, new VLineItem(label.empty() ? tag : label, centerWidget, rightWidget))
{
}

PLineItem::PLineItem(const std::string &tag, QWidget *rightWidget, const std::string &label) : PWidget(tag, new VLineItem(label.empty() ? tag : label, rightWidget)) {}
