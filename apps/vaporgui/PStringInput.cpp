#include "PStringInput.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>
#include "VStringLineEdit.h"

PStringInput::PStringInput(const std::string &tag, const std::string &label) : PLineItem(tag, label, _stringLineEdit = new VStringLineEdit)
{
    connect(_stringLineEdit, &VStringLineEdit::ValueChanged, this, &PStringInput::inputValueChanged);
}

void PStringInput::updateGUI() const
{
    const string value = getParamsString();
    _stringLineEdit->SetValueString(value);
}

void PStringInput::inputValueChanged(const std::string &v) { setParamsString(v); }
