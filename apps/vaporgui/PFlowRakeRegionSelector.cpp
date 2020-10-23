#include "PFlowRakeRegionSelector.h"
#include <vapor/FlowParams.h>

using VAPoR::FlowParams;

VAPoR::Box *PFlowRakeRegionSelector1D::getBox() const { return getParams<FlowParams>()->GetRakeBox(); }
