#pragma once

class QPushButtonWithDoubleClick : public QPushButton {
    Q_OBJECT
    using QPushButton::QPushButton;
    void mouseDoubleClickEvent(QMouseEvent *e) { emit doubleClicked(); }

signals:
    void doubleClicked();
};
