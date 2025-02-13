#pragma once

#include <QWidget>
#include <QLayoutItem>
#include <string>

class QLabel;

//! \class VLineItem
//! A widget that creates a label - input pair separated by a spacer
//! as is used throughout vapor

class VLineItem : public QWidget {
    Q_OBJECT
    QLabel *_qLabel;

public:
    VLineItem(const std::string &label, QLayoutItem *centerItem, QWidget *rightWidget);
    VLineItem(const std::string &label, QWidget *centerWidget, QWidget *rightWidget);
    VLineItem(const std::string &label, QWidget *rightWidget);
    void SetLabelText(const std::string &text);

private:
    static const int _LEFT_MARGIN;
    static const int _TOP_MARGIN;
    static const int _BOTTOM_MARGIN;
    static const int _RIGHT_MARGIN;

    VLineItem(const std::string &label);
};
