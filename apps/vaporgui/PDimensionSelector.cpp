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

    if (!rp->GetBox()->IsPlanar()) {
        vector<double> min, max;
        rp->GetBox()->GetExtents(min, max);
        float minZ = min[2];
        float maxZ = max[2];
        float epsilon = std::max(abs(minZ), abs(maxZ)) * __FLT_EPSILON__;
        if (abs(maxZ - minZ) <= epsilon) {
            vector<double> dmin, dmax;
            size_t         ts = rp->GetCurrentTimestep();
            int            level = rp->GetRefinementLevel();
            int            lod = rp->GetCompressionLevel();
            string         varName = rp->GetFirstVariableName();

            int ret = getDataMgr()->GetVariableExtents(ts, varName, level, lod, dmin, dmax);
            if (ret == 0) {
                min[2] = dmin[2];
                max[2] = dmax[2];
                rp->GetBox()->SetExtents(min, max);
            }
        }
    }
    rp->EndGroup();
}
