#pragma once

#include <string>

#include <QWidget>
#include <QPushButton>

#include "VHBoxWidget.h"

//! \class VPushButton
//!
//! Wraps a QPushButton with vaporgui's VContainer, so that
//! the QPushButton follows correct size policy

class VPushButton : public VHBoxWidget {
    Q_OBJECT

public:
    VPushButton(const std::string &buttonText = "Select");

private:
    QPushButton *_pushButton;

public slots:
    void emitButtonClicked();

signals:
    void ButtonClicked();
};
