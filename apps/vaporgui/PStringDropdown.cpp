#include "PStringDropdown.h"
#include <vapor/ParamsBase.h>
#include <vapor/VAssert.h>
#include "VComboBox.h"

PStringDropdown::PStringDropdown(const std::string &tag, const std::vector<std::string> &items, const std::string &label) : PLineItem(tag, label, _vComboBox = new VComboBox(items))
{
    connect(_vComboBox, &VComboBox::ValueChanged, this, &PStringDropdown::dropdownTextChanged);
}

void PStringDropdown::SetItems(const std::vector<std::string> &items) const { _vComboBox->SetOptions(items); }

void PStringDropdown::updateGUI() const
{
    auto val = getParamsString();
    if (val.empty())
        val = _customBlankText;
    _vComboBox->SetValue(val);
}

void PStringDropdown::setCustomBlankText(std::string text) { _customBlankText = text; }

void PStringDropdown::dropdownTextChanged(std::string text) { setParamsString(text); }
