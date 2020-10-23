#pragma once

#include <QFrame>
#include <QHBoxLayout>

#include <string>

#include "VLineItem.h"

//! \class VContainer
//!
//! A simple wrapper class for one or more VWidgets.
//! Sets a standard layout and margin policy, and protects
//! against unwanted mouse scroll whell events.

class VHBoxWidget : public QFrame {
    Q_OBJECT

public:
    //    virtual QSize sizeHint() const;

protected:
    VHBoxWidget();

private:
    static const int _LEFT_MARGIN;
    static const int _TOP_MARGIN;
    static const int _RIGHT_MARGIN;
    static const int _BOTTOM_MARGIN;
};

// Helper object for disabling the mouse scroll-wheel on a given widget.
//

#include <QObject>

class MouseWheelWidgetAdjustmentGuard : public QObject {
public:
    explicit MouseWheelWidgetAdjustmentGuard(QObject *parent);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;
};
