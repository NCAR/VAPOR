#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include "PVariableWidgets.h"
#include "GUIStateParams.h"
#include "VComboBox.h"
#include "PFidelitySection.h"

using namespace VAPoR;


// ==================================
//         PDimensionSelector
// ==================================


PDimensionSelector::PDimensionSelector()
: PLineItem("", "Variable Dimension", _vComboBox = new VComboBox({"2D", "3D"}))
{
    connect(_vComboBox, &VComboBox::ValueChanged, this, &PDimensionSelector::dropdownTextChanged);
}

void PDimensionSelector::updateGUI() const
{
    RenderParams *rp = dynamic_cast<RenderParams*>(getParams());
    VAssert(rp && "Params must be RenderParams");

    size_t dim = rp->GetRenderDim();
    //VAssert( dim == 2 || dim == 3 && "Invalid dimensionality" );
 
    if ( dim == 2 ) {
        _vComboBox->SetValue("2D");
    }
    else {
        _vComboBox->SetValue("3D");
    }
}

void PDimensionSelector::dropdownTextChanged(std::string text)
{
    RenderParams *rp = (RenderParams*)getParams();
    int dim = text == "2D" ? 2 : 3;
   
    //std::cout << "dropdownTextChanged( " << text << std::endl; 
    ParamsMgr* pm = getParamsMgr();
    //pm->BeginSaveStateGroup( "Set default variables" );
    //std::cout << "Setting dim to " << dim << std::endl;
    rp->SetDefaultVariables(dim, false);
    //pm->EndSaveStateGroup();

    if (dim == 2) {
        rp->GetBox()->SetPlanar(true);
        rp->GetBox()->SetOrientation(VAPoR::Box::XY);
    }
    else {
        rp->GetBox()->SetPlanar(false);
        rp->GetBox()->SetOrientation(VAPoR::Box::XYZ);
    }

}

// ==================================
//         Variable Selectors
// ==================================

#define NULL_TEXT "<none>"

PVariableSelector::PVariableSelector(const std::string &tag, const std::string &label)
: PStringDropdown(tag, {}, label)
{}

void PVariableSelector::updateGUI() const
{
    //std::cout << " void PVariableSelector::updateGUI() const " << std::endl;
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
    //std::cout << "      isShown() " << getRendererDimension() << " " << _onlyShowForDim << std::endl;
    if (_onlyShowForDim > 0)
        return getRendererDimension() == _onlyShowForDim;
    return true;
}

int PVariableSelector::getRendererDimension() const
{
    //RenderParams *rp = dynamic_cast<RenderParams*>(getParams());
    //return rp->GetBox()->GetOrientation() == Box::XYZ ? 3 : 2;
    int dims = dynamic_cast<RenderParams*>(getParams())->GetRenderDim();
    //VAssert( dims == 2 || dims == 3 && "Invalid dimensionality" );
    VAssert( dims < 4 && "Invalid dimensionality" );
    return dims;
}

int PVariableSelector::getDimensionality() const
{
    //int dims = getDataMgr()->GetNumDimensions(getParamsString());
    int dims = dynamic_cast<RenderParams*>(getParams())->GetRenderDim();
    //VAssert( dims == 2 || dims == 3 && "Invalid dimensionality" );
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


PScalarVariableSelector::PScalarVariableSelector()     
    : PVariableSelector  ("", "Variable Name") {}

PColorMapVariableSelector::PColorMapVariableSelector() 
    : PVariableSelector  ("", "Color mapped variable") {}

PHeightVariableSelector::PHeightVariableSelector()     
    : PVariableSelector2D("", "Height variable") { 
    AddNullOption(); OnlyShowForDim(2);
}

PXFieldVariableSelector::PXFieldVariableSelector()     
    : PVariableSelector  ("", "X") { AddNullOption(); }

PYFieldVariableSelector::PYFieldVariableSelector()     
    : PVariableSelector  ("", "Y") { AddNullOption(); }

PZFieldVariableSelector::PZFieldVariableSelector()     
    : PVariableSelector  ("", "Z") { AddNullOption(); }
