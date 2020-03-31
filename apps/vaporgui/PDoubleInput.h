#pragma once

#include "PLineItem.h"
//#include "VaporWidgetsFwd.h"

class VLineEdit;

//! \class PDoubleInput
//! Creates a Qt text input for double values synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PDoubleInput : public PLineItem {
    Q_OBJECT

    VLineEdit *_doubleInput;

public:
    PDoubleInput(const std::string &tag, const std::string &label = "");

protected:
    void updateGUI() const override;

private slots:
    void doubleInputValueChanged(const std::string &v);
};
