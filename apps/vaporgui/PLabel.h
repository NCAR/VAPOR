#pragma once

#include "PWidget.h"

class PLabel : public PWidget {
public:
    PLabel(const std::string &text);
    void updateGUI() const override {}
};
