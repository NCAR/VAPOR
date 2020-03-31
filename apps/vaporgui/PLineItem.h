#pragma once

#include "PWidget.h"

//! \class PLineItem
//! Internal PWidget class. Standardizes PWidgets that take up a single line with a label, spacer, input.
//! \copydoc PWidget

class PLineItem : public PWidget {
    Q_OBJECT

public:
    PLineItem(const std::string &tag, const std::string &label, QWidget *centerWidget, QWidget *rightWidget);
    PLineItem(const std::string &tag, const std::string &label, QWidget *rightWidget);
};
