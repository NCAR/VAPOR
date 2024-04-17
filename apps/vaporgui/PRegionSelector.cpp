#include "PRegionSelector.h"
#include "QRangeSliderTextCombo.h"
#include <vapor/RenderParams.h>
#include <assert.h>
#include "POrientationSelector.h"

using namespace VAPoR;

PRegionSelector::PRegionSelector(VAPoR::ControlExec* ce, const std::string &label) : PSection(label)
{
    Add(new PRegionSelectorX(ce));
    Add(new PRegionSelectorY(ce));
    Add(new PRegionSelectorZ(ce));
}

PRegionSelector1D::PRegionSelector1D(int dim, VAPoR::ControlExec* ce) : PLineItem("", dim == 0 ? "X" : dim == 1 ? "Y" : "Z", _slider = new QRangeSliderTextCombo), _dim(dim), _ce(ce)
{
    QObject::connect(_slider, &QRangeSliderTextCombo::ValueChanged, this, &PRegionSelector1D::sliderValueChanged);
    _slider->AllowCustomRange();
}

void PRegionSelector1D::updateGUI() const
{
    RenderParams *rp = getParams<RenderParams>();

    VAPoR::CoordType min, max;
    size_t         ts = rp->GetCurrentTimestep();
    int            level = rp->GetRefinementLevel();
    int            lod = rp->GetCompressionLevel();
    string         varName = rp->GetFirstVariableName();

    Box *box = getBox();

    // In some cases (ImageRenderer), there may be no variable to configure extents with.
    // Therefore, configure the sliders with the extents of the data domain, acquired from DataStatus 
    if (varName == "") {
        box->GetExtents(min, max);
        _slider->SetValue(min[_dim], max[_dim]);
        VAPoR::ParamsMgr* pm = _ce->GetParamsMgr();
        VAPoR::DataStatus* ds = _ce->GetDataStatus();
        VAPoR::CoordType minExts, maxExts;
        ds->GetActiveExtents(pm, ts, minExts, maxExts);
        _slider->SetRange(minExts[_dim], maxExts[_dim]);
        return;
    }

    int ret = getDataMgr()->GetVariableExtents(ts, varName, level, lod, min, max);
    if (ret < 0) {
        _slider->SetRange(0, 0);
        _slider->SetValue(0, 0);
        return;
    }

    _slider->SetRange(min[_dim], max[_dim]);

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

    CoordType min, max;
    box->GetExtents(min, max);
    min[_dim] = v0;
    max[_dim] = v1;
    box->SetExtents(min, max);
}
