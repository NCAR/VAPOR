#include "VLineItem.h"
#include <QHBoxLayout>
#include <QLabel>

VLineItem::VLineItem(const std::string &label, QLayoutItem *centerItem, QWidget *rightWidget)
{
    setLayout(new QHBoxLayout);
    layout()->setMargin(0);
    
    layout()->addWidget(new QLabel(label.c_str()));
    layout()->addItem(centerItem);
    layout()->addWidget(rightWidget);
}

VLineItem::VLineItem(const std::string &label, QWidget *centerWidget, QWidget *rightWidget)
: VLineItem(label, new QWidgetItem(centerWidget), rightWidget)
{}

VLineItem::VLineItem(const std::string &label, QWidget *rightWidget)
: VLineItem(label, (QLayoutItem*)new QSpacerItem(108, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), rightWidget)
{}
