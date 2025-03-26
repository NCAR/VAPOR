#pragma once

#include "VLabel.h"

class VHyperlink : public VLabel {
public:
    VHyperlink(const std::string &text = "", const std::string &url = "", bool bullet=false);
};
