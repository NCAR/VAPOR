#include "PWidget.h"
#include <assert.h>
#include <vapor/VAssert.h>
#include <vapor/ParamsBase.h>
#include <vapor/ParamsMgr.h>
#include <QHBoxLayout>
#include "SettingsParams.h"

PWidget::PWidget(const std::string &tag, QWidget *widget) : UWidget(widget), _tag(tag) { this->setDisabled(true); }

void PWidget::Update(VAPoR::ParamsBase *params, VAPoR::ParamsMgr *paramsMgr, VAPoR::DataMgr *dataMgr)
{
    _params = params;
    _paramsMgr = paramsMgr;
    _dataMgr = dataMgr;

    if (_dynamicUpdateInsideGroup) return;

    if (params == nullptr) {
        this->setDisabled(true);
        return;
    }
    if (requireDataMgr() && !dataMgr) VAssert(!"Data manager required but missing");
    if (requireParamsMgr() && !paramsMgr) VAssert(!"Params manager required but missing");

    bool paramsVisible = isShown();
    if (paramsVisible && _showBasedOnParam) paramsVisible = _showBasedOnParamValue == params->GetValueLong(_showBasedOnParamTag, 0);
    if (paramsVisible) {
        setVisible(true);
    } else {
        setVisible(false);
        return;
    }

    if (_enableBasedOnParam) {
        int value = params->GetValueLong(_enableBasedOnParamTag, 0);
        setEnabled(value == _enableBasedOnParamValue);
    } else {
        setEnabled(true);
    }

    updateGUI();
}

const std::string &PWidget::getTag() const { return _tag; }

PWidget *PWidget::ShowBasedOnParam(const std::string &tag, int whenEqualTo)
{
    _showBasedOnParam = true;
    _showBasedOnParamTag = tag;
    _showBasedOnParamValue = whenEqualTo;
    return this;
}

PWidget *PWidget::EnableBasedOnParam(const std::string &tag, int whenEqualTo)
{
    _enableBasedOnParam = true;
    _enableBasedOnParamTag = tag;
    _enableBasedOnParamValue = whenEqualTo;
    return this;
}

PWidget *PWidget::SetTooltip(const std::string &text)
{
    QWidget::setToolTip(QString::fromStdString(text));
    return this;
}

VAPoR::ParamsBase *PWidget::getParams() const { return _params; }
VAPoR::ParamsMgr * PWidget::getParamsMgr() const { return _paramsMgr; }
VAPoR::DataMgr *   PWidget::getDataMgr() const { return _dataMgr; }

SettingsParams *PWidget::getSettingsParams() const
{
    VAssert(requireParamsMgr());
    return (SettingsParams *)_paramsMgr->GetParams(SettingsParams::GetClassType());
}

void PWidget::setParamsDouble(double v)
{
    dynamicUpdateFinish();
    _setParamsDouble(v);
}

void PWidget::setParamsLong(long v)
{
    dynamicUpdateFinish();
    _setParamsLong(v);
}

void PWidget::setParamsString(const std::string &v)
{
    dynamicUpdateFinish();
    _setParamsString(v);
}

double PWidget::getParamsDouble() const
{
    if (_usingHLI)
        return _getterDouble(_params);
    else
        return _params->GetValueDouble(_tag, 0.0);
}

long PWidget::getParamsLong() const
{
    if (_usingHLI)
        return _getterLong(_params);
    else
        return _params->GetValueLong(_tag, 0);
}

std::string PWidget::getParamsString() const
{
    if (_usingHLI)
        return _getterString(_params);
    else
        return _params->GetValueString(_tag, "");
}

void PWidget::dynamicUpdateBegin()
{
    assert(_dynamicUpdateIsOn);
    if (!_dynamicUpdateInsideGroup) {
        getParams()->BeginGroup(getTag() + " dynamic change");
        _dynamicUpdateInsideGroup = true;
    }
}

void PWidget::dynamicUpdateFinish()
{
    if (_dynamicUpdateIsOn && _dynamicUpdateInsideGroup) {
        getParams()->EndGroup();
        _dynamicUpdateInsideGroup = false;
    }
}

void PWidget::_setParamsDouble(double v)
{
    if (_usingHLI)
        _setterDouble(_params, v);
    else
        getParams()->SetValueDouble(getTag(), "", v);
}

void PWidget::_setParamsLong(long v)
{
    if (_usingHLI)
        _setterLong(_params, v);
    else
        getParams()->SetValueLong(getTag(), "", v);
}

void PWidget::_setParamsString(const std::string &v)
{
    if (_usingHLI)
        _setterString(_params, v);
    else
        getParams()->SetValueString(getTag(), "", v);
}
