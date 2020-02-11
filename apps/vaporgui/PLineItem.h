#pragma once

#include "PWidget.h"

class PLineItem : public PWidget {
    Q_OBJECT
    
public:
    PLineItem(const std::string &tag, const std::string &label, QWidget *centerWidget, QWidget *rightWidget);
    PLineItem(const std::string &tag, const std::string &label, QWidget *rightWidget);
};
