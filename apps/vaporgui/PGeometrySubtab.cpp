#include "PGeometrySubtab.h"
#include "PRegionSelector.h"
#include "PCopyRegionWidget.h"
#include "PTransformWidget.h"

PGeometrySubtab::PGeometrySubtab() : PGroup({new PRegionSelector, new PCopyRegionWidget, new PRendererTransformWidget}) {}
