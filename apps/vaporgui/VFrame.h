#pragma once

#include <QWidget>

//! \class VFrame
//!
//! A simple class that sets a layout, spacing, and margin
//! policy onto a QFrame for consistency throughout the application.
class VFrame : public QFrame {
    Q_OBJECT

public:
    VFrame()
    {
        setLayout(new QVBoxLayout);
        layout()->setContentsMargins(0, 0, 0, 0);
        layout()->setSpacing(12);
        setFrameStyle(QFrame::NoFrame);
    }

    void addWidget(QWidget *widget) { layout()->addWidget(widget); }
};
