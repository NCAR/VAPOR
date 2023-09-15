#include "VContainer.h"
#include <cassert>
#include <QVBoxLayout>

VContainer::VContainer(QWidget *w)
{
    QLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(w);
    QWidget::setLayout(layout);
}

void VContainer::AddBottomStretch()
{
    QVBoxLayout *layout = dynamic_cast<QVBoxLayout *>(QWidget::layout());
    assert(layout);
    assert(layout->count() == 1);
    layout->addStretch();
}

void VContainer::SetPadding(int left, int top, int right, int bottom)
{
    QVBoxLayout *layout = dynamic_cast<QVBoxLayout *>(QWidget::layout());
    assert(layout);
    layout->setContentsMargins(left, top, right, bottom);
}
