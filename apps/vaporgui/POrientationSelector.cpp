#include "POrientationSelector.h"
#include "PEnumDropdown.h"
#include <vapor/RenderParams.h>
#include <assert.h>

using namespace VAPoR;

POrientationSelector::POrientationSelector() : PWidget("", _dropdown = new PEnumDropdown(Box::m_orientationTag, {"XY", "XZ", "YZ"}, {Box::XY, Box::XZ, Box::YZ}, "Orientation")) {}

void POrientationSelector::updateGUI() const
{
    RenderParams *rp = dynamic_cast<RenderParams *>(getParams());
    VAssert(rp);
    _dropdown->Update(rp->GetBox(), getParamsMgr(), getDataMgr());
}
