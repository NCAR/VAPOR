#include "VRouter.h"
#include "vapor/STLUtils.h"

VRouter::VRouter(AbstractWidgetGroup<VRouter, QWidget>::List children)
: VContainer(_stack = new QStackedWidget)
{
    AddM(children);
}

VRouter *VRouter::Add(QWidget *w)
{
    AbstractWidgetGroup::Add(w);
    _stack->addWidget(w);
    return this;
}

void VRouter::Show(QWidget *w)
{
    if (w == nullptr)
        return Show(emptyWidget());
    if (!STLUtils::Contains(_children, w)) Add(w);
    _stack->setCurrentWidget(w);
}

QWidget *VRouter::emptyWidget()
{
    if (_emptyWidget == nullptr)
        _emptyWidget = new QWidget;
    return _emptyWidget;
}