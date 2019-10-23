#include "VLineItem.h"
#include <QHBoxLayout>
#include <QLabel>

VLineItem::VLineItem(std::string label, QWidget *rightWidget)
{
    setLayout(new QHBoxLayout);
    layout()->setMargin(0);

    layout()->addWidget(new QLabel(label.c_str()));
    layout()->addItem(new QSpacerItem(108, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
    layout()->addWidget(rightWidget);
}
