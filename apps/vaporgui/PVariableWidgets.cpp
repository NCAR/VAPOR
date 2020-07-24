#include "PVariableWidgets.h"
#include "VCheckBox.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>
#include <vapor/RenderParams.h>
#include <VComboBox.h>
#include <assert.h>

using VAPoR::RenderParams;
using VAPoR::Box;

#define NULL_TEXT "<none>"

PVariableSelector::PVariableSelector(const std::string &tag, const std::string &label)
: PStringDropdown(tag, {}, label)
{}

void PVariableSelector::updateGUI() const
{
    RenderParams *rp = dynamic_cast<RenderParams*>(getParams());
    assert(rp && "Params must be RenderParams");
    static_cast<void>(rp);        // Silence unused variable warning
    
    int nDims = getDimensionality();
    
    auto varNames = getDataMgr()->GetDataVarNames(nDims);
    
    if (_addNull)
        varNames.insert(varNames.begin(), NULL_TEXT);
    
    SetItems(varNames);
    PStringDropdown::updateGUI();
}

bool PVariableSelector::isShown() const
{
    if (_onlyShowForDim > 0)
        return getRendererDimension() == _onlyShowForDim;
    return true;
}

int PVariableSelector::getRendererDimension() const
{
    RenderParams *rp = dynamic_cast<RenderParams*>(getParams());
    return rp->GetBox()->GetOrientation() == Box::XYZ ? 3 : 2;
}

int PVariableSelector::getDimensionality() const
{
    int dims = getDataMgr()->GetNumDimensions(getParamsString());
    if (dims > 0)
        return dims;
    
    return getRendererDimension();
}

void PVariableSelector::dropdownTextChanged(std::string text)
{
    if (_addNull && text == NULL_TEXT)
        text = "";
    
    PStringDropdown::dropdownTextChanged(text);
}
