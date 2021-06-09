#include "PWidgetWrapper.h"

PWidgetWrapper::PWidgetWrapper(PWidget *p) : PWidgetWrapper("", p) {}

PWidgetWrapper::PWidgetWrapper(std::string tag, PWidget *p) : PWidget(tag, _child = p) {}

void PWidgetWrapper::updateGUI() const { _child->Update(getWrappedParams(), getParamsMgr(), getDataMgr()); }

VAPoR::ParamsBase *PWidgetWrapper::getWrappedParams() const
{
    return getParams();
}
