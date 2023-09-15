#include "PDynamicMixin.h"
#include <vapor/VAssert.h>
#include "PWidget.h"
#include <vapor/ParamsBase.h>

PWidget *PDynamicMixin::EnableDynamicUpdate(bool enabled)
{
    PWidget *pw = getPWidget();
    pw->_dynamicUpdateIsOn = enabled;
    return pw;
}

void PDynamicMixin::dynamicSetParamsDouble(double v)
{
    PWidget *pw = getPWidget();
    if (pw->_dynamicUpdateIsOn) {
        pw->dynamicUpdateBegin();
        pw->_setParamsDouble(v);
        pw->getParams()->IntermediateChange();
    }
}

void PDynamicMixin::dynamicSetParamsLong(long v)
{
    PWidget *pw = getPWidget();
    if (pw->_dynamicUpdateIsOn) {
        pw->dynamicUpdateBegin();
        pw->_setParamsLong(v);
        pw->getParams()->IntermediateChange();
    }
}

void PDynamicMixin::dynamicSetParamsString(const std::string &v)
{
    PWidget *pw = getPWidget();
    if (pw->_dynamicUpdateIsOn) {
        pw->dynamicUpdateBegin();
        pw->_setParamsString(v);
        pw->getParams()->IntermediateChange();
    }
}

PWidget *PDynamicMixin::getPWidget()
{
    PWidget *pw = dynamic_cast<PWidget *>(this);
    VAssert(pw != nullptr);
    return pw;
}
