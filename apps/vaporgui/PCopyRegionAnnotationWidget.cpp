#include "PCopyRegionAnnotationWidget.h"
#include "CopyRegionAnnotationWidget.h"
#include <vapor/ControlExecutive.h>
#include <vapor/RenderParams.h>

using namespace VAPoR;

PCopyRegionAnnotationWidget::PCopyRegionAnnotationWidget(ControlExec *ce) : PWidget("", _widget = new CopyRegionAnnotationWidget(ce)) {}

void PCopyRegionAnnotationWidget::updateGUI() const { _widget->Update(getParamsMgr()); }
