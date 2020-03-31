#include "PGroup.h"
#include <vapor/ParamsBase.h>
#include <QVBoxLayout>

PGroup::PGroup() : PWidget("", _widget = new QWidget)
{
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    _widget->setLayout(layout);
}

PGroup *PGroup::Add(PWidget *pw)
{
    _children.push_back(pw);
    _widget->layout()->addWidget(pw);
    return this;
}

void PGroup::updateGUI() const
{
    auto params = getParams();
    auto paramsMgr = getParamsMgr();
    auto dataMgr = getDataMgr();

    for (PWidget *child : _children) child->Update(params, paramsMgr, dataMgr);
}
