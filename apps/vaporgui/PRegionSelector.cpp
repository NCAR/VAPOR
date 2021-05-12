#include "PRegionSelector.h"
#include "QRangeSliderTextCombo.h"
#include <vapor/RenderParams.h>
#include <assert.h>
#include "POrientationSelector.h"

using namespace VAPoR;

PRegionSelector::PRegionSelector(const std::string &label) : PSection(label)
{
    Add(new PRegionSelectorX);
    Add(new PRegionSelectorY);
    Add(new PRegionSelectorZ);
}

PRegionSelector1D::PRegionSelector1D(int dim) : PLineItem("", dim == 0 ? "X" : dim == 1 ? "Y" : "Z", _slider = new QRangeSliderTextCombo), _dim(dim)
{
    QObject::connect(_slider, &QRangeSliderTextCombo::ValueChanged, this, &PRegionSelector1D::sliderValueChanged);
}

void PRegionSelector1D::updateGUI() const
{
    RenderParams *rp = getParams<RenderParams>();

    vector<double> min, max;
    size_t         ts = rp->GetCurrentTimestep();
    int            level = rp->GetRefinementLevel();
    int            lod = rp->GetCompressionLevel();
    string         varName = rp->GetFirstVariableName();

    int ret = getDataMgr()->GetVariableExtents(ts, varName, level, lod, min, max);
    if (ret < 0) {
        _slider->SetRange(0, 0);
        _slider->SetValue(0, 0);
        return;
    }

    _slider->SetRange(min[_dim], max[_dim]);

    Box *box = getBox();
    box->GetExtents(min, max);
    _slider->SetValue(min[_dim], max[_dim]);
}

bool PRegionSelector1D::isShown() const
{
    Box *box = getBox();

    const Box::Orientation o = (Box::Orientation)box->GetOrientation();
    const int              d = _dim;

    switch (o) {
    case Box::XYZ: return true;
    case Box::XY: return d == 0 || d == 1;
    case Box::XZ: return d == 0 || d == 2;
    case Box::YZ: return d == 1 || d == 2;
    default: assert(0);
    }

    return false;
}

VAPoR::Box *PRegionSelector1D::getBox() const { return getParams<RenderParams>()->GetBox(); }

void PRegionSelector1D::sliderValueChanged(float v0, float v1)
{
    Box *box = getBox();

    vector<double> min, max;
    box->GetExtents(min, max);
    min[_dim] = v0;
    max[_dim] = v1;
    box->SetExtents(min, max);
}
