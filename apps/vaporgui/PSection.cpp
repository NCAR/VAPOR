#include "PSection.h"
#include "VSection.h"
#include "PGroup.h"
#include <vapor/ParamsBase.h>

PSection::PSection(const std::string &label, const PGroup::List &widgets) : PWidget("", _vsection = new VSection(label))
{
    _pgroup = new PGroup;
    _vsection->layout()->addWidget(_pgroup);
    _pgroup->AddM(widgets);
}

void PSection::updateGUI() const { _pgroup->Update(getParams(), getParamsMgr(), getDataMgr()); }

PSection *PSection::Add(PWidget *pw)
{
    _pgroup->Add(pw);
    return this;
}

PSection *PSection::Add(const PGroup::List &widgets)
{
    _pgroup->AddM(widgets);
    return this;
}
