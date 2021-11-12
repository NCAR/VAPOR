#include "PSliceOriginSelector.h"
#include "PSliderEdit.h"
#include "PSliderEditHLI.h"
#include <vapor/SliceParams.h>
#include <assert.h>

using namespace VAPoR;

PSliceOriginSelector::PSliceOriginSelector()
{
    _xSlider = new PDoubleSliderEdit( SliceParams::XOriginTag, "X Origin");//)->EnableDynamicUpdate();
    _ySlider = new PDoubleSliderEdit( SliceParams::YOriginTag, "Y Origin");//)->EnableDynamicUpdate();
    _zSlider = new PDoubleSliderEdit( SliceParams::ZOriginTag, "Z Origin");//)->EnableDynamicUpdate();

    //_xSlider = new PDoubleSliderEditHLI<SliceParams>( "X", &SliceParams::GetXOrigin, &SliceParams::SetXOrigin);
    //_ySlider = new PDoubleSliderEditHLI<SliceParams>( "Y", &SliceParams::GetYOrigin, &SliceParams::SetYOrigin);
    //_zSlider = new PDoubleSliderEditHLI<SliceParams>( "Z", &SliceParams::GetZOrigin, &SliceParams::SetZOrigin);

    Add(_xSlider);
    Add(_ySlider);
    Add(_zSlider);
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

    //rp->GetBox()->GetExtents(min, max);

    _xSlider->SetRange(min[0], max[0]);
    _ySlider->SetRange(min[1], max[1]);
    _zSlider->SetRange(min[2], max[2]);

    std::cout << "PSOS updateGUI()" << std::endl;
    PGroup::updateGUI();
}
