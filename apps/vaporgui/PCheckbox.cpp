#include "PCheckbox.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>

PCheckbox::PCheckbox(const std::string &tag, const std::string &label) : PWidget(tag, new VLineItem(label == "" ? tag : label, _qcheckbox = new QCheckBox))
{
    connect(_qcheckbox, SIGNAL(stateChanged(int)), this, SLOT(checkboxStateChanged(int)));
}

void PCheckbox::updateGUI() const
{
    bool on = getParams()->GetValueLong(GetTag(), 0);
    _qcheckbox->setChecked(on);
}

void PCheckbox::checkboxStateChanged(int state) { getParams()->SetValueLong(GetTag(), GetTag(), state == Qt::CheckState::Checked); }
