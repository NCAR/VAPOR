#include "PDisplay.h"
#include "VCheckBox.h"
#include "VLineItem.h"
#include "VLabel.h"
#include <vapor/ParamsBase.h>

PDisplay::PDisplay(const std::string &tag, const std::string &label) : PWidget(tag, new VLineItem(label == "" ? tag : label, _label = new VLabel)) {}

PDisplay *PDisplay::Selectable()
{
    _label->MakeSelectable();
    return this;
}

void PDisplay::setText(std::string text) const
{
    if (text.empty())
        setText("<empty>");
    else
        _label->SetText(text);
}

void PStringDisplay::updateGUI() const
{
    std::string text = getParamsString();
    setText(text);
}

void PIntegerDisplay::updateGUI() const
{
    long value = getParamsLong();
    setText(QString::number(value).toStdString());
}

void PDoubleDisplay::updateGUI() const
{
    double value = getParamsDouble();
    setText(QString::number(value).toStdString());
}

void PBooleanDisplay::updateGUI() const
{
    bool on = getParamsLong();
    setText(on ? "True" : "False");
}
