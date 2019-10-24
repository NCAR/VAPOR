#include "PDoubleInput.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>

PDoubleInput::PDoubleInput(const std::string &tag, const std::string &label)
: PWidget(tag, new VLineItem(label==""?tag:label, _doubleInput = new VDoubleInput))
{
//    connect(_doubleInput, SIGNAL(ValueChanged(double)), this, SLOT(doubleInputValueChanged(double)));
}

void PDoubleInput::update() const
{
    double value = getParams()->GetValueDouble(GetTag(), 0);
    // _doubleInput->SetValue(value);
}

void PDoubleInput::doubleInputValueChanged(double v)
{
    getParams()->SetValueDouble(GetTag(), GetTag(), v);
}
