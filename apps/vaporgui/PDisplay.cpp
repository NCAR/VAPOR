#include "PDisplay.h"
#include "VCheckBox.h"
#include "VLineItem.h"
#include <vapor/ParamsBase.h>

PDisplay::PDisplay(const std::string &tag, const std::string &label) : PWidget(tag, new VLineItem(label == "" ? tag : label, _label = new QLabel)) {}

void PStringDisplay::updateGUI() const
{
    std::string text = getParamsString();
    _label->setText(QString::fromStdString(text));
}

void PIntegerDisplay::updateGUI() const
{
    long value = getParamsLong();
    _label->setText(QString::number(value));
}

void PDoubleDisplay::updateGUI() const
{
    double value = getParamsDouble();
    _label->setText(QString::number(value));
}

void PBooleanDisplay::updateGUI() const
{
    bool on = getParamsLong();
    _label->setText(on ? "True" : "False");
}
