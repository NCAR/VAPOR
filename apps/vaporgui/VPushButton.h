#pragma once

#include <string>

#include <QWidget>
#include <QPushButton>

#include "VContainer.h"

class VPushButton : public VContainer {
    Q_OBJECT

public:
    VPushButton( const std::string& buttonText = "Select" );

private:
    QPushButton* _pushButton;

public slots:
    void emitButtonClicked();

signals:
    void ButtonClicked();
};
