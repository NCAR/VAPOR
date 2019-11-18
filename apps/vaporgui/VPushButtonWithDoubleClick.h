#pragma once

class VPushButtonWithDoubleClick : public QPushButton {
    Q_OBJECT
    using QPushButton::QPushButton;
    void mouseDoubleClickEvent(QMouseEvent * e) {
        emit doubleClicked();
    }

signals:
    void doubleClicked();
};
