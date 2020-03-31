#pragma once

#include "PLineItem.h"

class VCheckBox;

//! \class PCheckbox
//! Creates a Qt Checkbox synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PCheckbox : public PLineItem {
    Q_OBJECT

    VCheckBox *_vcheckbox;

public:
    PCheckbox(const std::string &tag, const std::string &label = "");

protected:
    void updateGUI() const override;

private slots:
    void checkboxStateChanged(bool on);
};
