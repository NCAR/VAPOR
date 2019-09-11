#pragma once

#include <QWidget>
#include <string>

class VLineItem : public QWidget {
    Q_OBJECT
    
public:
    VLineItem(std::string label, QWidget *rightWidget);
};
