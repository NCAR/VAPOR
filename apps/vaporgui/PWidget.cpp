#include "PWidget.h"
#include <vapor/VAssert.h>

PWidget::PWidget(const std::string &tag)
: _tag(tag) {}

void PWidget::Update(VAPoR::ParamsBase *params, VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr)
{
    _params = params;
    _paramsMgr = paramsMgr;
    _dataMgr = dataMgr;
    
    if (params) {
        if (requireDataMgr()   && !dataMgr)   VAssert(!"Data manager required but missing");
        if (requireParamsMgr() && !paramsMgr) VAssert(!"Params manager required but missing");
        updateGUI();
    }
}

const std::string &PWidget::GetTag() const
{
    return _tag;
}

VAPoR::ParamsBase *PWidget::getParams()    const { return _params; }
VAPoR::ParamsMgr  *PWidget::getParamsMgr() const { return _paramsMgr; }
VAPoR::DataMgr    *PWidget::getDataMgr()   const { return _dataMgr; }
