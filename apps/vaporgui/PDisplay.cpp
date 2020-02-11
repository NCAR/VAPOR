#include "PDisplay.h"
#include "VCheckBox.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>

PDisplay::PDisplay(const std::string &tag, const std::string &label)
: PWidget(tag, new VLineItem(label==""?tag:label, _label = new QLabel))
{
}



void PStringDisplay::updateGUI() const
{
    std::string text = getParams()->GetValueString(GetTag(), "<empty>");
    _label->setText(QString::fromStdString(text));
}



void PIntegerDisplay::updateGUI() const
{
    long value = getParams()->GetValueLong(GetTag(), 0);
    _label->setText(QString::number(value));
}



void PDoubleDisplay::updateGUI() const
{
    double value = getParams()->GetValueDouble(GetTag(), 0.0);
    _label->setText(QString::number(value));
}



void PBooleanDisplay::updateGUI() const
{
    bool on = getParams()->GetValueLong(GetTag(), false);
    _label->setText(on ? "True" : "False");
}
