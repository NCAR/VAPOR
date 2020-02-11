#include "PVariableSelector.h"
#include "VCheckBox.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>
#include <vapor/RenderParams.h>
#include <assert.h>

using VAPoR::RenderParams;
using VAPoR::Box;

PVariableSelector::PVariableSelector(const std::string &tag, const std::string &label)
: PStringDropdown(tag, {}, label)
{}

void PVariableSelector::updateGUI() const
{
    RenderParams *rp = dynamic_cast<RenderParams*>(getParams());
    assert(rp && "Params must be RenderParams");
    
    int nDims = rp->GetBox()->GetOrientation() == Box::XY ? 2 : 3;
    
    auto varNames = getDataMgr()->GetDataVarNames(nDims);
    SetItems(varNames);
    
    PStringDropdown::updateGUI();
}
