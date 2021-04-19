#include "PSliceSampleLocationSelector.h"
#include "VDoubleSliderEdit.h"
#include <vapor/SliceParams.h>
#include <assert.h>

using namespace VAPoR;

PSliceSampleLocationSelector::PSliceSampleLocationSelector()
{
    Add(new PSliceSampleLocationSelectorX);
    Add(new PSliceSampleLocationSelectorY);
    Add(new PSliceSampleLocationSelectorZ);
}

PSliceSampleLocationSelector1D::PSliceSampleLocationSelector1D(int dim) : PLineItem("", dim == 0 ? "Sample X" : dim == 1 ? "Sample Y" : "Sample Z", _slider = new VDoubleSliderEdit), _dim(dim)
{
    QObject::connect(_slider, &VDoubleSliderEdit::ValueChanged, this, &PSliceSampleLocationSelector1D::sliderValueChanged);
}

void PSliceSampleLocationSelector1D::updateGUI() const
{
    SliceParams *rp = getParams<SliceParams>();

    vector<double> min, max;
    size_t         ts = rp->GetCurrentTimestep();
    int            level = rp->GetRefinementLevel();
    int            lod = rp->GetCompressionLevel();
    string         varName = rp->GetVariableName();

    int ret = getDataMgr()->GetVariableExtents(ts, varName, level, lod, min, max);
    assert(ret == 0);
    (void)ret;
    _slider->SetMinimum(min[_dim]);
    _slider->SetMaximum(max[_dim]);

    min = rp->GetValueDoubleVec(SliceParams::SampleLocationTag);
    _slider->SetValue(min[_dim]);
}

bool PSliceSampleLocationSelector1D::isShown() const
{
    RenderParams *rp = dynamic_cast<RenderParams *>(getParams());
    VAssert(rp);
    Box *box = rp->GetBox();

    const Box::Orientation o = (Box::Orientation)box->GetOrientation();
    const int              d = _dim;

    switch (o) {
    case Box::XYZ: return true;
    case Box::XY: return d == 2;
    case Box::XZ: return d == 1;
    case Box::YZ: return d == 0;
    default: assert(0);
    }

    return false;
}

void PSliceSampleLocationSelector1D::sliderValueChanged(float v0)
{
    SliceParams *rp = getParams<SliceParams>();

    vector<double> loc = rp->GetValueDoubleVec(SliceParams::SampleLocationTag);
    loc[_dim] = v0;
    rp->SetValueDoubleVec(SliceParams::SampleLocationTag, "", loc);
}
