#include "PDynamicMixin.h"
#include <vapor/VAssert.h>
#include "PWidget.h"
#include <vapor/ParamsBase.h>

PWidget *PDynamicMixin::EnableDynamicUpdate()
{
    PWidget *pw = getPWidget();
    pw->_dynamicUpdateIsOn = true;
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
