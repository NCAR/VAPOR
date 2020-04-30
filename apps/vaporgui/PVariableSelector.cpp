#include "PVariableSelector.h"
#include "VCheckBox.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>
#include <vapor/RenderParams.h>
#include <VComboBox.h>
#include <assert.h>

using VAPoR::Box;
using VAPoR::RenderParams;

PVariableSelector::PVariableSelector(const std::string &tag, const std::string &label) : PStringDropdown(tag, {}, label) {}

void PVariableSelector::updateGUI() const
{
    RenderParams *rp = dynamic_cast<RenderParams *>(getParams());
    assert(rp && "Params must be RenderParams");
    static_cast<void>(rp);    // Silence unused variable warning

    int nDims = getDimensionality();

    auto varNames = getDataMgr()->GetDataVarNames(nDims);
    SetItems(varNames);

    PStringDropdown::updateGUI();
}

int PVariableSelector::getDimensionality() const
{
    // return rp->GetBox()->GetOrientation() == Box::XY ? 2 : 3;

    return getDataMgr()->GetNumDimensions(getParamsString());
}
