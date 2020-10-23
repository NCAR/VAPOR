#include "VGroup.h"
#include <QVBoxLayout>

VGroup::VGroup()
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(4);
    setLayout(layout);
}

VGroup *VGroup::Add(QWidget *w)
{
    layout()->addWidget(w);
    return this;
}

VSubGroup::VSubGroup() { layout()->setContentsMargins(12, 0, 0, 0); }
