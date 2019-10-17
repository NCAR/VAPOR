#pragma once

#include <QWidget>
#include <string>

//! \class VLineItem
//! A widget that creates a label - input pair separated by a spacer
//! as is used throughout vapor

class VLineItem : public QWidget {
    Q_OBJECT

public:
    VLineItem(std::string label, QWidget *rightWidget);
};
