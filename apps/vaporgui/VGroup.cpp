#include "VGroup.h"
#include <QVBoxLayout>

VGroup::VGroup(List children)
: VGroup(new QVBoxLayout, children) {}

VGroup::VGroup(QBoxLayout *layout, List children)
{
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

VHGroup::VHGroup()
: VGroup(new QHBoxLayout, {}) {}
