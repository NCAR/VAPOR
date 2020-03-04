#include "PCheckbox.h"
#include "VCheckBox.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>

PCheckbox::PCheckbox(const std::string &tag, const std::string &label)
: PLineItem(tag, label, _qcheckbox = new VCheckBox)
{
    connect(_qcheckbox, &VCheckBox::ValueChanged, this, &PCheckbox::checkboxStateChanged);
}

void PCheckbox::updateGUI() const
{
    bool on = getParamsLong();
    
    _qcheckbox->SetValue(on);
}

void PCheckbox::checkboxStateChanged(bool on)
{
    setParamsLong(on);
}
