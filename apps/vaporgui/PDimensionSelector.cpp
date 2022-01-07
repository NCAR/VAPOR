#include "PDimensionSelector.h"
#include <vapor/RenderParams.h>
#include "VComboBox.h"

using namespace VAPoR;

PDimensionSelector::PDimensionSelector() : PLineItem("", "Variable Dimension", _vComboBox = new VComboBox({"2D", "3D"}))
{
    connect(_vComboBox, &VComboBox::ValueChanged, this, &PDimensionSelector::dropdownTextChanged);
}

void PDimensionSelector::updateGUI() const
{
    if (getParams<RenderParams>()->GetRenderDim() == 3)
        _vComboBox->SetValue("3D");
    else
        _vComboBox->SetValue("2D");
}

void PDimensionSelector::dropdownTextChanged(std::string text)
{
    RenderParams *rp = getParams<RenderParams>();
    int           dim = text == "2D" ? 2 : 3;

    rp->BeginGroup("Change dim");
    if (dim == 2) {
        rp->GetBox()->SetPlanar(true);
        rp->GetBox()->SetOrientation(VAPoR::Box::XY);
    } else {
        rp->GetBox()->SetPlanar(false);
        rp->GetBox()->SetOrientation(VAPoR::Box::XYZ);
    }
    rp->SetDefaultVariables(dim, true);
    rp->EndGroup();
}
