#include "PCopyRegionAnnotationWidget.h"
#include "CopyRegionAnnotationWidget.h"
#include <vapor/RenderParams.h>

PCopyRegionAnnotationWidget::PCopyRegionAnnotationWidget( VAPoR::ControlExecutive* ce ) : PWidget("", _widget = new CopyRegionAnnotationWidget(ce) ) {}

void PCopyRegionAnnotationWidget::updateGUI() const { _widget->Update(getParamsMgr()); }
