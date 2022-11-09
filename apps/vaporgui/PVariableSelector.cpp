#include "PVariableSelector.h"
#include "PCheckbox.h"
#include <vapor/RenderParams.h>
#include <assert.h>

using VAPoR::Box;
using VAPoR::RenderParams;
typedef VAPoR::DataMgr::VarType VarType;

#define NULL_TEXT "<none>"

PVariableSelector::PVariableSelector(const std::string &tag, const std::string &label) : PStringDropdown(tag, {}, label) {}

void PVariableSelector::updateGUI() const
{
    int nDims = getDimensionality();

    auto varNames = getDataMgr()->GetDataVarNames(nDims, (VarType)getVarType());

    if (_addNull || getParamsString().empty()) varNames.insert(varNames.begin(), NULL_TEXT);

    SetItems(varNames);
    PStringDropdown::updateGUI();
}

bool PVariableSelector::isShown() const
{
    if (_onlyShowForDim > 0) return getRendererDimension() == _onlyShowForDim;
    return true;
}

int PVariableSelector::getVarType() const
{
    if (_showParticleVars)
        return (int)VarType::Particle;
    else
        return (int)VarType::Scalar;
}

int PVariableSelector::getRendererDimension() const { return getParams<RenderParams>()->GetRenderDim(); }

int PVariableSelector::getDimensionality() const { return getRendererDimension(); }

void PVariableSelector::dropdownTextChanged(std::string text)
{
    if (_addNull && text == NULL_TEXT) text = "";

    PStringDropdown::dropdownTextChanged(text);
}

PScalarVariableSelector::PScalarVariableSelector() : PVariableSelector(RenderParams::_variableNameTag, "Variable Name") {}
PColorMapVariableSelector::PColorMapVariableSelector() : PVariableSelector(RenderParams::_colorMapVariableNameTag, "Color mapped variable") {}
PXFieldVariableSelector::PXFieldVariableSelector() : PVariableSelector(RenderParams::_xFieldVariableNameTag, "X Field") { AddNullOption(); }
PYFieldVariableSelector::PYFieldVariableSelector() : PVariableSelector(RenderParams::_yFieldVariableNameTag, "Y Field") { AddNullOption(); }
PZFieldVariableSelector::PZFieldVariableSelector() : PVariableSelector(RenderParams::_zFieldVariableNameTag, "Z Field") { AddNullOption(); }

PHeightVariableSelector::PHeightVariableSelector() : PGroup() {
    Add((new PVariableSelector2D(RenderParams::_heightVariableNameTag, "Height variable"))->AddNullOption());
    Add(new PCheckbox(RenderParams::AddHeightToBottomTag, "Add height values to bottom of domain"));
    OnlyShowForDim(2);
}

PHeightVariableSelector* PHeightVariableSelector::OnlyShowForDim(int dim) {
    _onlyShowForDim = dim;
    return this;
}

bool PHeightVariableSelector::isShown() const {
    if (_onlyShowForDim > 0) return getParams<RenderParams>()->GetRenderDim() == _onlyShowForDim;
    return true;
}
