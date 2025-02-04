#include "VRadioButton.h"
#include <QRadioButton>
#include <iostream>

VRadioButton::VRadioButton(bool checked) : VHBoxWidget()
{
    _radioButton = new QRadioButton;
    SetValue(checked);
    layout()->addWidget(_radioButton);

    //connect(_radioButton, &QRadioButton::clicked, this, &VRadioButton::emitRadioButtonChanged);
    connect(_radioButton, &QRadioButton::toggled, this, &VRadioButton::emitRadioButtonChanged);
}

// Stas thinks that we should have setValues and setValue instead of Update
//
void VRadioButton::SetValue(bool checked)
{
    _radioButton->blockSignals(true);
    _radioButton->setChecked(checked);
    _radioButton->blockSignals(false);
}

bool VRadioButton::GetValue() const { return _radioButton->isChecked(); }

//std::string VRadioButton::GetText() const { 
//    std::cout << "GetText() " << _radioButton->text().toStdString() << std::endl; 
//    return _radioButton->text().toStdString(); 
//};

void VRadioButton::emitRadioButtonChanged(bool checked) { emit ValueChanged(checked); }
