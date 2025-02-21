#pragma once

//#include "PLineItem.h"
//#include "PGroup.h"
#include "PWidget.h"
//#include "VGroup.h"

//class VRadioButton;
class VGroup;

//! \class PRadioButtons
//! Creates a Qt RadioButton synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

//class PRadioButtons : public PLineItem {
//class PRadioButtons : public VGroup {
class PRadioButtons : public PWidget {
    Q_OBJECT

    VGroup *_vg;
    std::string _paramValue;
    //VRadioButton *_vRadioButton;

public:
    //PRadioButtons(const std::string &tag, const std::string &label = "");
    PRadioButtons(const std::string &tag, const std::vector<std::string> labels);

protected:
    void updateGUI() const override;

private slots:
    void radioButtonStateChanged();
};
