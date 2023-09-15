#pragma once

#include "PLineItem.h"
#include <vector>
//#include "VaporWidgetsFwd.h"

class VComboBox;

//! \class PEnumDropdown
//! Creates a Qt dropdown for selecting enum values synced with the paramsdatabase using the PWidget interface.
//! \copydoc PWidget

class PEnumDropdown : public PLineItem {
    Q_OBJECT

    VComboBox *             _vComboBox;
    const std::vector<long> _enumMap;

public:
    //! If itemValues is empty, the item values will be initialized to the index of each item.
    PEnumDropdown(const std::string &tag, const std::vector<std::string> &items, const std::vector<long> &itemValues = {}, const std::string &label = "");

protected:
    void updateGUI() const override;

private slots:
    void dropdownIndexChanged(int index);
};
