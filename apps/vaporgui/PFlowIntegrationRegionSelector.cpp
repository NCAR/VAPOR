#include "PFlowIntegrationRegionSelector.h"
#include <vapor/FlowParams.h>

using VAPoR::FlowParams;

VAPoR::Box *PFlowIntegrationRegionSelector1D::getBox() const { return getParams<FlowParams>()->GetIntegrationBox(); }
