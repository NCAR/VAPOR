#include "PDoubleInput.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>
#include <QLineEdit>

PDoubleInput::PDoubleInput(const std::string &tag, const std::string &label) : PLineItem(tag, _doubleInput = new VDoubleInput, label)
{
    //    connect(_doubleInput, SIGNAL(ValueChanged(double)), this, SLOT(doubleInputValueChanged(double)));
}

void PDoubleInput::updateGUI() const
{
    // double value = getParams()->GetValueDouble(GetTag(), 0);
    // _doubleInput->SetValue(value);
}

void PDoubleInput::doubleInputValueChanged(double v) { getParams()->SetValueDouble(GetTag(), GetTag(), v); }
