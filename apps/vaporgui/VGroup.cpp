#include "VGroup.h"
#include <QVBoxLayout>

VGroup::VGroup(List children)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(4);
    setLayout(layout);
    AddM(children);
}

VGroup *VGroup::Add(QWidget *w)
{
    layout()->addWidget(w);
    return this;
}

VSubGroup::VSubGroup(List children)
{
    layout()->setContentsMargins(12, 0, 0, 0);
    AddM(children);
}
