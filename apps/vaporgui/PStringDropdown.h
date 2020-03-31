#pragma once

#include "PLineItem.h"
#include <vector>
//#include "VaporWidgetsFwd.h"

class VComboBox;

//! \class PStringDropdown same as PEnumDropdown except it sets the param
//! to directly reflect the string in the drowdown.
//! \copydoc PEnumDropdown

class PStringDropdown : public PLineItem {
    Q_OBJECT

    VComboBox *_vComboBox;

public:
    PStringDropdown(const std::string &tag, const std::vector<std::string> &items, const std::string &label = "");
    //! Sets the items presented in the dropdown
    void SetItems(const std::vector<std::string> &items) const;

protected:
    virtual void updateGUI() const override;

private slots:
    void dropdownTextChanged(std::string text);
};
