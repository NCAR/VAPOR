#pragma once

#include "PLineItem.h"

class VRadioButton;

//! \class PRadioButton
//! Creates a Qt RadioButton synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PRadioButton : public PLineItem {
    Q_OBJECT

    std::string _paramValue;
    VRadioButton *_vRadioButton;

public:
    PRadioButton(const std::string &tag, const std::string &label = "");

protected:
    void updateGUI() const override;

private slots:
    //void radioButtonStateChanged(std::string input);
    void radioButtonStateChanged();
};
