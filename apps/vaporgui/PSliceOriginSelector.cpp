#include "PSliceOriginSelector.h"
#include "PSliderEdit.h"
#include "PLabel.h"
#include <vapor/SliceParams.h>
#include <assert.h>

using namespace VAPoR;

PSliceOriginSelector::PSliceOriginSelector() : PSection("Slice Origin")
{
    _xSlider = new PDoubleSliderEdit(RenderParams::XSlicePlaneOriginTag, "X");
    _ySlider = new PDoubleSliderEdit(RenderParams::YSlicePlaneOriginTag, "Y");
    _zSlider = new PDoubleSliderEdit(RenderParams::ZSlicePlaneOriginTag, "Z");

    _xSlider->EnableDynamicUpdate();
    _ySlider->EnableDynamicUpdate();
    _zSlider->EnableDynamicUpdate();

    Add(_xSlider);
    Add(_ySlider);
    Add(_zSlider);
    Add(new PLabel("Slice origin is shown in-scene as a yellow crosshair"));
}

void PSliceOriginSelector::updateGUI() const
{
    RenderParams *rp = getParams<RenderParams>();

    CoordType      min, max;
    size_t         ts = rp->GetCurrentTimestep();
    int            level = rp->GetRefinementLevel();
    int            lod = rp->GetCompressionLevel();
    string         varName = rp->GetVariableName();

    int ret = getDataMgr()->GetVariableExtents(ts, varName, level, lod, min, max);
    if (ret) return;

    _xSlider->SetRange(min[0], max[0]);
    _ySlider->SetRange(min[1], max[1]);
    _zSlider->SetRange(min[2], max[2]);

    PSection::updateGUI();
}



#include <vapor/ArbitrarilyOrientedRegularGrid.h>

PSliceOffsetSelector::PSliceOffsetSelector() : PSection("Slice Offset")
{
    _offsetSlider = new PDoubleSliderEdit(RenderParams::SliceOffsetTag, "Offset");
    _offsetSlider->EnableDynamicUpdate();
    Add(_offsetSlider);
}

void PSliceOffsetSelector::updateGUI() const
{
    RenderParams *rp = getParams<RenderParams>();

    planeDescription pd;
    size_t           ts = rp->GetCurrentTimestep();
    int              level = rp->GetRefinementLevel();
    int              lod = rp->GetCompressionLevel();
    string           varName = rp->GetVariableName();

    int ret = getDataMgr()->GetVariableExtents(ts, varName, level, lod, pd.boxMin, pd.boxMax);
    if (ret) return;
    pd.origin = rp->GetSlicePlaneOrigin();
    if (rp->GetValueLong(rp->SlicePlaneOrientationModeTag, 0) == (int)RenderParams::SlicePlaneOrientationMode::Normal) {
        pd.normal = rp->GetSlicePlaneNormal();
    } else {
        auto n = ArbitrarilyOrientedRegularGrid::GetNormalFromRotations(rp->GetSlicePlaneRotation());
        pd.normal = {n[0], n[1], n[2]};
    }

    auto range = ArbitrarilyOrientedRegularGrid::GetOffsetRange(pd);
    _offsetSlider->SetRange(range.first, range.second);

    PSection::updateGUI();
}
