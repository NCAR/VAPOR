#include "PGroup.h"
#include <vapor/ParamsBase.h>
#include <QVBoxLayout>
#include "VGroup.h"

PGroup::PGroup() : PGroup(new VGroup) {}

PGroup::PGroup(const List &widgets) : PGroup() { AddM(widgets); }

PGroup::PGroup(VGroup *w) : PWidget("", _widget = w), WidgetGroupWrapper(w) {}

void PGroup::updateGUI() const
{
    auto params = getParams();
    auto paramsMgr = getParamsMgr();
    auto dataMgr = getDataMgr();

    for (PWidget *child : _children) child->Update(params, paramsMgr, dataMgr);
}

PSubGroup::PSubGroup() : PGroup(new VSubGroup) {}

PSubGroup::PSubGroup(const List &widgets) : PSubGroup() { AddM(widgets); }
