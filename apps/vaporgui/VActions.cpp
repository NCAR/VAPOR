#include <string>

#include "VActions.h"
#include "VIntSpinBox.h"
#include "VCheckBox.h"
#include "VStringLineEdit.h"
#include "VIntLineEdit.h"
#include "VDoubleLineEdit.h"

VLineAction::VLineAction(const std::string &title, VHBoxWidget *container) : QWidgetAction(NULL)
{
    VLineItem *vli = new VLineItem(title, container);
    vli->setContentsMargins(5, 0, 5, 0);
    setDefaultWidget(vli);
}

VSpinBoxAction::VSpinBoxAction(const std::string &title, int value) : VLineAction(title, _spinBox = new VIntSpinBox(1, 10))
{
    _spinBox->SetValue(value);
    connect(_spinBox, SIGNAL(ValueChanged(int)), this, SLOT(_spinBoxChanged(int)));
}

void VSpinBoxAction::SetValue(int value) { _spinBox->SetValue(value); }

void VSpinBoxAction::_spinBoxChanged(int value) { emit editingFinished(value); }

VCheckBoxAction::VCheckBoxAction(const std::string &title, bool value) : VLineAction(title, _checkBox = new VCheckBox(value))
{
    connect(_checkBox, &VCheckBox::ValueChanged, this, &VCheckBoxAction::_checkBoxChanged);
}

void VCheckBoxAction::SetValue(bool value) { _checkBox->SetValue(value); }

void VCheckBoxAction::_checkBoxChanged(bool value) { emit clicked(value); }

VStringLineEditAction::VStringLineEditAction(const std::string &title, std::string value) : VLineAction(title, _lineEdit = new VStringLineEdit(value))
{
    connect(_lineEdit, SIGNAL(ValueChanged(int)), this, SLOT(_lineEditChanged(int)));
}

void VStringLineEditAction::SetValue(const std::string &value) { _lineEdit->SetValueString(value); }

void VStringLineEditAction::_lineEditChanged(int value) { emit ValueChanged(value); }

VIntLineEditAction::VIntLineEditAction(const std::string &title, int value) : VLineAction(title, _lineEdit = new VIntLineEdit(value))
{
    connect(_lineEdit, SIGNAL(ValueChanged(int)), this, SLOT(_lineEditChanged(int)));
}

void VIntLineEditAction::SetValue(int value) { _lineEdit->SetValueInt(value); }

void VIntLineEditAction::_lineEditChanged(int value) { emit ValueChanged(value); }

VDoubleLineEditAction::VDoubleLineEditAction(const std::string &title, double value) : VLineAction(title, _lineEdit = new VDoubleLineEdit(value))
{
    connect(_lineEdit, SIGNAL(ValueChanged(double)), this, SLOT(_lineEditChanged(double)));
}

void VDoubleLineEditAction::SetValue(double value) { _lineEdit->SetValueDouble(value); }

void VDoubleLineEditAction::_lineEditChanged(double value) { emit ValueChanged(value); }
