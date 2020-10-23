#pragma once

#include "PLineItem.h"
//#include "VaporWidgetsFwd.h"

class VStringLineEdit;

//! \class PStringInput
//! Creates a Qt text input for string values synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PStringInput : public PLineItem {
    VStringLineEdit *_stringLineEdit;

public:
    PStringInput(const std::string &tag, const std::string &label = "");

protected:
    void updateGUI() const override;

private:
    void inputValueChanged(const std::string &v);
};
