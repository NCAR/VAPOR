#pragma once

#include "PWidget.h"

class PLineItem : public PWidget {
    Q_OBJECT

public:
    PLineItem(const std::string &tag, QWidget *centerWidget, QWidget *rightWidget, const std::string &label = "");
    PLineItem(const std::string &tag, QWidget *rightWidget, const std::string &label = "");
};
