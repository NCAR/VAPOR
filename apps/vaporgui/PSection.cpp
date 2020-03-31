#include "PSection.h"
#include "VSection.h"
#include "PGroup.h"
#include <vapor/ParamsBase.h>

PSection::PSection(const std::string &label) : PWidget("", _vsection = new VSection(label))
{
    _pgroup = new PGroup;
    _vsection->layout()->addWidget(_pgroup);
}

void PSection::updateGUI() const { _pgroup->Update(getParams(), getParamsMgr(), getDataMgr()); }

PSection *PSection::Add(PWidget *pw)
{
    _pgroup->Add(pw);
    return this;
}
