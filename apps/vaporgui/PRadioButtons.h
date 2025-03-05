#pragma once

#include "PWidget.h"

//class VRadioButton;
class VGroup;

//! \class PRadioButtons
//! Creates a Qt RadioButton synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PRadioButtons : public PWidget {
    Q_OBJECT

    VGroup *_vg;
    std::string _paramValue;

public:
    PRadioButtons(const std::string &tag, const std::vector<std::string> labels);

protected:
    void updateGUI() const override;

private slots:
    void radioButtonStateChanged();
};
