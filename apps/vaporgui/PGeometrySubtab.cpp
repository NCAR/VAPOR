#include "PGeometrySubtab.h"
#include "PRegionSelector.h"
#include "PCopyRegionWidget.h"
#include "PTransformWidget.h"

PGeometrySubtab::PGeometrySubtab(VAPoR::ControlExec* ce) : PGroup({new PRegionSelector(ce), new PCopyRegionWidget, new PRendererTransformSection}) {}
