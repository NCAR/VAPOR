#pragma once

#include "PWidget.h"

class VLabel;

//! \class PLabel
//! \brief Displays static text.
//! \author Stas Jaroszynski

class PLabel : public PWidget {
    VLabel *_label;

public:
    PLabel(const std::string &text);
    void updateGUI() const override {}
};
