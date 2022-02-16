#include "PSliceController.h"
#include "PEnumDropdown.h"
#include "PSliderEditHLI.h"
#include "PLabel.h"
#include "PShowIf.h"
#include "PCheckbox.h"
#include <vapor/SliceParams.h>
#include <assert.h>

using namespace VAPoR;

PSliceController::PSliceController() : PGroup() {
    Add(new PSliceOrientationSelector);
    Add(new PSliceOffsetSelector);
    Add(new PSliceOriginSelector);
}

PSliceOrientationSelector::PSliceOrientationSelector() : PSection("Slice Orientation") {
    Add( new PEnumDropdown(RenderParams::SlicePlaneOrientationModeTag, {"Rotation", "Normal"}, {(int)RenderParams::SlicePlaneOrientationMode::Rotation, (int)RenderParams::SlicePlaneOrientationMode::Normal}, "Orientation Mode"));
    Add( (new PShowIf(RenderParams::SlicePlaneOrientationModeTag))->Equals((int)RenderParams::SlicePlaneOrientationMode::Rotation)->Then({
                (new PDoubleSliderEdit(RenderParams::XSlicePlaneRotationTag, "X"))->SetRange(-90.,90.)->EnableDynamicUpdate(),
                (new PDoubleSliderEdit(RenderParams::YSlicePlaneRotationTag, "Y"))->SetRange(-90.,90.)->EnableDynamicUpdate(),
                (new PDoubleSliderEdit(RenderParams::ZSlicePlaneRotationTag, "Z"))->SetRange(-90.,90.)->EnableDynamicUpdate(),
            })->Else({
                (new PDoubleSliderEdit(RenderParams::SlicePlaneNormalXTag, "X"))->SetRange(-1,1)->EnableDynamicUpdate(),
                (new PDoubleSliderEdit(RenderParams::SlicePlaneNormalYTag, "Y"))->SetRange(-1,1)->EnableDynamicUpdate(),
                (new PDoubleSliderEdit(RenderParams::SlicePlaneNormalZTag, "Z"))->SetRange(-1,1)->EnableDynamicUpdate(),
            })
    );
    SetTooltip("The plane normal of the slice. The offset will move the slice along this normal as well.");
}

PSliceOriginSelector::PSliceOriginSelector() : PSection("Slice Origin")
{
    _xSlider = new PDoubleSliderEditHLI<RenderParams>("X", &RenderParams::GetXSlicePlaneOrigin, &RenderParams::SetXSlicePlaneOrigin);
    _ySlider = new PDoubleSliderEditHLI<RenderParams>("Y", &RenderParams::GetYSlicePlaneOrigin, &RenderParams::SetYSlicePlaneOrigin);
    _zSlider = new PDoubleSliderEditHLI<RenderParams>("Z", &RenderParams::GetZSlicePlaneOrigin, &RenderParams::SetZSlicePlaneOrigin);

    _xSlider->EnableDynamicUpdate();
    _ySlider->EnableDynamicUpdate();
    _zSlider->EnableDynamicUpdate();

    Add({
        new PLabel("Slice origin is shown in-scene as a yellow crosshair"),
        new PCheckbox("GUI_ShowOrigin", "Show Origin Controls"),
        (new PShowIf("GUI_ShowOrigin"))
            ->Then({
                _xSlider,
                _ySlider,
                _zSlider,
            }),
    });
    SetTooltip("The slice plane will pass through this point. \nThe plane can be offset from this point along the plane normal determined by the orientation.");
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
    SetTooltip("Offset the plane from its origin \n along its normal (set by the orientation).");
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
