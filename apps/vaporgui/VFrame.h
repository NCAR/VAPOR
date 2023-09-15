#pragma once

#include <QWidget>
#include <QFrame>
#include <forward_list>

//! \class VFrame
//!
//! A simple class that sets a layout, spacing, and margin
//! policy onto a QFrame for consistency throughout the application.
class VFrame : public QFrame {
    Q_OBJECT

public:
    VFrame();

    void addWidget(QWidget *widget);

    int getNumOfChildWidgets() const;

    // The following two methods control if a child is shown or hidden.
    // If a child widget is the ith added to this VFrame, then idx should be i-1.
    // It returns 0 upon success, and 1 upon failure (e.g. invalid index).
    int hideChildAtIdx(int idx);
    int showChildAtIdx(int idx);

private:
    std::forward_list<QWidget *> _child_widgets;
    int                          _num_of_children = 0;
};
