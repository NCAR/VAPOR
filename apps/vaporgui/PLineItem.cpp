#include "PLineItem.h"
#include "VLineItem.h"

PLineItem::PLineItem(const std::string &tag, const std::string &label, QWidget *centerWidget, QWidget *rightWidget)
: PWidget(tag, new VLineItem(label.empty() ? tag : label, centerWidget, rightWidget))
{
}

PLineItem::PLineItem(const std::string &tag, const std::string &label, QWidget *rightWidget) : PWidget(tag, new VLineItem(label.empty() ? tag : label, rightWidget)) {}

PLineItem::PLineItem(const std::string &label, PWidget *rightWidget) : PLineItem("", label, rightWidget) { _child = rightWidget; }

void PLineItem::updateGUI() const
{
    assert(_child);
    _child->Update(getParams(), getParamsMgr(), getDataMgr());
}
