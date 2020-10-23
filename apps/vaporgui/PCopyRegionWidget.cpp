#include "PCopyRegionWidget.h"
#include "CopyRegionWidget.h"
#include <vapor/RenderParams.h>

PCopyRegionWidget::PCopyRegionWidget() : PWidget("", _widget = new CopyRegionWidget) {}

void PCopyRegionWidget::updateGUI() const { _widget->Update(getParamsMgr(), getParams<VAPoR::RenderParams>()); }
