#include "PRadioButtons.h"
#include "VRadioButton.h"
#include "VGroup.h"

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
        _vg->Add(rb);
    }
}

void PRadioButtons::updateGUI() const
{
    for (int i =0; i< _vg->layout()->count(); i++) {
        VRadioButton *rb = dynamic_cast<VRadioButton*>(_vg->layout()->itemAt(i)->widget());
        if (getParamsString() == rb->GetText()) {
            rb->SetValue(true);
        }
        else rb->SetValue(false);
    }
}

void PRadioButtons::radioButtonStateChanged() { 
    VRadioButton *rb = dynamic_cast<VRadioButton*>(sender());
    setParamsString(rb->GetText()); 
}
