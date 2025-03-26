#pragma once

#include "VLabel.h"

//! \class VHyperlink
//! A VLabel that contains a hyperlink and optional bulletpoint

class VHyperlink : public VLabel {
public:
    VHyperlink(const std::string &text = "", const std::string &url = "", bool bullet=false);
};
