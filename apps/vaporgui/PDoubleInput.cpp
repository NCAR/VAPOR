#include "PDoubleInput.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>
#include "VLineEdit.h"

PDoubleInput::PDoubleInput(const std::string &tag, const std::string &label)
: PLineItem(tag, label, _doubleInput = new VLineEdit)
{
    _doubleInput->UseDoubleMenu();
    connect( _doubleInput, SIGNAL( ValueChanged( double ) ), this, SLOT( doubleInputValueChanged( double ) ) );
}

void PDoubleInput::updateGUI() const
{
    double value = getParamsDouble();
    _doubleInput->SetValue(to_string(value));
}

void PDoubleInput::doubleInputValueChanged( double d )
{
    setParamsDouble(d);
}
