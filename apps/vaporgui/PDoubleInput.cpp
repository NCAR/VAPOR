#include "PDoubleInput.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>
#include "VLineEdit_Deprecated.h"

PDoubleInput::PDoubleInput(const std::string &tag, const std::string &label) : PLineItem(tag, label, _doubleInput = new VLineEdit_Deprecated)
{
    _doubleInput->SetIsDouble(true);
    connect(_doubleInput, &VLineEdit_Deprecated::ValueChanged, this, &PDoubleInput::doubleInputValueChanged);
}

void PDoubleInput::updateGUI() const
{
    double value = getParamsDouble();
    _doubleInput->SetValue(to_string(value));
}

void PDoubleInput::doubleInputValueChanged(const std::string &v)
{
    double d = stod(v);
    setParamsDouble(d);
}
