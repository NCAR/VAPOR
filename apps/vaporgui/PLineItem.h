#pragma once

#include "PWidget.h"

//! \class PLineItem
//! Internal PWidget class. Standardizes PWidgets that take up a single line with a label, spacer, input.
//! \copydoc PWidget

class PLineItem : public PWidget {
    Q_OBJECT
    PWidget *_child = nullptr;

public:
    PLineItem(const std::string &tag, const std::string &label, QWidget *centerWidget, QWidget *rightWidget);
    PLineItem(const std::string &tag, const std::string &label, QWidget *rightWidget);
    PLineItem(const std::string &label, PWidget *rightWidget);

protected:
    void updateGUI() const override;
};
