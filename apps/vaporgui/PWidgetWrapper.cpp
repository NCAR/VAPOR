#include "PWidgetWrapper.h"

PWidgetWrapper::PWidgetWrapper(PWidget *p) : PWidgetWrapper("", p) {}

PWidgetWrapper::PWidgetWrapper(std::string tag, PWidget *p) : PWidget(tag, _child = p) {}

void PWidgetWrapper::updateGUI() const { _child->Update(getParams(), getParamsMgr(), getDataMgr()); }
