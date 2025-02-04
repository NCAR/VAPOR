#include "PRadioButton.h"
#include "VRadioButton.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>
#include "DatasetTypeLookup.h"
#include <vapor/GUIStateParams.h>

PRadioButton::PRadioButton(const std::string &tag, const std::string &label) : PLineItem(tag, label, _vRadioButton = new VRadioButton)
{
    _paramValue = label;
    connect(_vRadioButton, &VRadioButton::ValueChanged, this, &PRadioButton::radioButtonStateChanged);
}

void PRadioButton::updateGUI() const
{
    //std::cout << "Updating PRadioButton " << getParamsString() << " " << _paramValue << std::endl;
    //_vRadioButton->setEnabled(true);
    if (getParamsString() == _paramValue) _vRadioButton->SetValue(true);
    //if (getParams()->GetValueString(GUIStateParams::SelectedImportDataTypeTag, "NetCDF-CF") == _paramValue) _vRadioButton->SetValue(true);
    else _vRadioButton->SetValue(false);
}

void PRadioButton::radioButtonStateChanged() { 
    setParamsString(_paramValue); 
}
