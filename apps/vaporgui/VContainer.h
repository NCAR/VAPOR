#pragma once

#include <QWidget>
#include <QHBoxLayout>

#include <string>

#include "VLineItem.h"

//! \class VContainer
//!
//! A simple wrapper class for one or more VWidgets.
//! Sets a standard layout and margin policy, and protects
//! against unwanted mouse scroll whell events.

class VContainer : public QWidget {
    Q_OBJECT

protected:
    VContainer();
};

// Helper object for disabling the mouse scroll-wheel on a given widget.
//

#include <QObject>

class MouseWheelWidgetAdjustmentGuard : public QObject
{
public:
    explicit MouseWheelWidgetAdjustmentGuard(QObject *parent);

protected:
    bool eventFilter(QObject* o, QEvent* e) override;
};
