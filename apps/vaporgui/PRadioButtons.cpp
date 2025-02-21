#include "PRadioButtons.h"
#include "VRadioButton.h"
#include "VGroup.h"
#include <iostream>


//PRadioButtons::PRadioButtons(const std::string &tag, const std::string &label) : PLineItem(tag, label, _vRadioButton = new VRadioButton)
//PRadioButtons::PRadioButtons(const std::string &tag, const std::vector<std::string> labels) : PGroup()
//PRadioButtons::PRadioButtons(const std::string &tag, const std::vector<std::string> labels) : VGroup()
PRadioButtons::PRadioButtons(const std::string &tag, const std::vector<std::string> labels) : PWidget(tag, _vg = new VGroup())
{
    bool first = true;
    for (auto l : labels) {
        VRadioButton *rb = new VRadioButton(l, false);
        if (first) {
            rb->SetValue(true);
            first = false;
        }
        connect(rb, &VRadioButton::ValueChanged, this, &PRadioButtons::radioButtonStateChanged);
        std::cout << "adding " << l << std::endl;
        _vg->Add(rb);
    }
}

void PRadioButtons::updateGUI() const
{
    std::cout << "PRadioButtons::updateGUI" << std::endl;
    //for (auto vRadioButton : _vg->layout()->children()) {
    for (int i =0; i< _vg->layout()->count(); i++) {
        VRadioButton *rb = dynamic_cast<VRadioButton*>(_vg->layout()->itemAt(i)->widget());
        std::cout << "string " << getParamsString() << " " << _paramValue << std::endl;
        //if (getParamsString() == _paramValue) {
        if (getParamsString() == rb->GetText()) {
            rb->SetValue(true);
        }
        else rb->SetValue(false);
    }
        //if (getParamsString() == _paramValue) _vRadioButton->SetValue(true);
        //else _vRadioButton->SetValue(false);
}

void PRadioButtons::radioButtonStateChanged() { 
    //setParamsString(getTag()); 
    VRadioButton *rb = dynamic_cast<VRadioButton*>(sender());
    setParamsString(rb->GetText()); 
}
