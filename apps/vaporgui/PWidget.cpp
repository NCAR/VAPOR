#include "PWidget.h"
#include <vapor/VAssert.h>
#include <QHBoxLayout>

PWidget::PWidget(const std::string &tag, QWidget *widget) : _tag(tag)
{
    setLayout(new QHBoxLayout);
    layout()->setMargin(0);
    layout()->addWidget(widget);
    this->setDisabled(true);
}

void PWidget::Update(VAPoR::ParamsBase *params, VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr)
{
    _params = params;
    _paramsMgr = paramsMgr;
    _dataMgr = dataMgr;

    if (params) {
        if (requireDataMgr() && !dataMgr) VAssert(!"Data manager required but missing");
        if (requireParamsMgr() && !paramsMgr) VAssert(!"Params manager required but missing");
        this->setDisabled(false);
        updateGUI();
    } else {
        this->setDisabled(true);
    }
}

const std::string &PWidget::GetTag() const { return _tag; }

VAPoR::ParamsBase *PWidget::getParams() const { return _params; }
VAPoR::ParamsMgr * PWidget::getParamsMgr() const { return _paramsMgr; }
VAPoR::DataMgr *   PWidget::getDataMgr() const { return _dataMgr; }
