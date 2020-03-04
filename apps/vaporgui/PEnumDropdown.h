#pragma once

#include "PLineItem.h"
#include <vector>
//#include "VaporWidgetsFwd.h"

class VComboBox;

class PEnumDropdown : public PLineItem {
    Q_OBJECT
    
    VComboBox *_vComboBox;
    const std::vector<long> _enumMap;
    
public:
    PEnumDropdown(const std::string &tag, const std::vector<std::string> &items, const std::vector<long> &itemValues = {}, const std::string &label = "");

protected:
    void updateGUI() const override;
    
private slots:
    void dropdownIndexChanged(int index);
};
