#pragma once

#include "PWidget.h"

class QLabel;

//! \class PLabel
//! \brief Displays static text.
//! \author Stas Jaroszynski

class PLabel : public PWidget {
    QLabel *_label;

public:
    PLabel(const std::string &text);
    void updateGUI() const override {}
};
