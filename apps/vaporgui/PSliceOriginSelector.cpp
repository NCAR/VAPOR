#include "PSliceOriginSelector.h"
#include "PSliderEdit.h"
#include "PLabel.h"
#include <vapor/SliceParams.h>
#include <assert.h>

using namespace VAPoR;

PSliceOriginSelector::PSliceOriginSelector() : PSection("Slice Origin")
{
    _xSlider = new PDoubleSliderEdit(RenderParams::XOriginTag, "X");
    _ySlider = new PDoubleSliderEdit(RenderParams::YOriginTag, "Y");
    _zSlider = new PDoubleSliderEdit(RenderParams::ZOriginTag, "Z");

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
    SliceParams *rp = getParams<SliceParams>();

    vector<double> min, max;
    size_t         ts = rp->GetCurrentTimestep();
    int            level = rp->GetRefinementLevel();
    int            lod = rp->GetCompressionLevel();
    string         varName = rp->GetVariableName();

    int ret = getDataMgr()->GetVariableExtents(ts, varName, level, lod, min, max);
    assert(ret == 0);
    (void)ret;

    _xSlider->SetRange(min[0], max[0]);
    _ySlider->SetRange(min[1], max[1]);
    _zSlider->SetRange(min[2], max[2]);

    PSection::updateGUI();
}
