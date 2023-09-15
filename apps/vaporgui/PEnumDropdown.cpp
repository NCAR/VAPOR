#include "PEnumDropdown.h"
#include <vapor/ParamsBase.h>
#include <vapor/VAssert.h>
#include "VComboBox.h"

PEnumDropdown::PEnumDropdown(const std::string &tag, const std::vector<std::string> &items, const std::vector<long> &itemValues, const std::string &label)
: PLineItem(tag, label, _vComboBox = new VComboBox(items)), _enumMap(itemValues)
{
    VAssert(itemValues.empty() || items.size() == itemValues.size());
    connect(_vComboBox, &VComboBox::IndexChanged, this, &PEnumDropdown::dropdownIndexChanged);
}

void PEnumDropdown::updateGUI() const
{
    int value = getParamsLong();

    int index = value;
    for (int i = 0; i < _enumMap.size(); i++) {
        if (_enumMap[i] == value) {
            index = i;
            break;
        }
    }

    _vComboBox->SetIndex(index);
}

void PEnumDropdown::dropdownIndexChanged(int index)
{
    int value;
    if (_enumMap.empty()) {
        value = index;
    } else {
        VAssert(index >= 0 && index < _enumMap.size());
        value = _enumMap[index];
    }
    setParamsLong(value);
}
