#include "PCheckbox.h"
#include "VCheckBox.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>

PCheckbox::PCheckbox(const std::string &tag, const std::string &label) : PLineItem(tag, label, _vcheckbox = new VCheckBox)
{
    connect(_vcheckbox, &VCheckBox::ValueChanged, this, &PCheckbox::checkboxStateChanged);
}

void PCheckbox::updateGUI() const
{
    bool on = getParamsLong();

    _vcheckbox->SetValue(on);
}

void PCheckbox::checkboxStateChanged(bool on) { setParamsLong(on); }
