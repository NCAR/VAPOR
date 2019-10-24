#include "PCheckbox.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>

PCheckbox::PCheckbox(const std::string &tag, const std::string &label)
: PWidget(tag, new VLineItem(label==""?tag:label, _qcheckbox = new QCheckBox))
{}

void PCheckbox::update() const
{
    bool on = getParams()->GetValueLong(GetTag(), 0);
    _qcheckbox->setChecked(on);
}

void PCheckbox::checkboxStateChanged(int state)
{
    getParams()->SetValueLong(GetTag(), GetTag(), state == Qt::CheckState::Checked);
}
