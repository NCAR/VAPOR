#include "VLineItem.h"
#include <QHBoxLayout>
#include <QLabel>

#define LEFT_MARGIN 10
#define TOP_MARGIN 0
#define RIGHT_MARGIN 10
#define BOTTOM_MARGIN  0

VLineItem::VLineItem(const std::string &label, QLayoutItem *centerItem, QWidget *rightWidget)
{
    setLayout(new QHBoxLayout);
    layout()->setContentsMargins(
        LEFT_MARGIN,
        TOP_MARGIN,
        RIGHT_MARGIN,
        BOTTOM_MARGIN
    );
    //layout()->setMargin(0);
    
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
