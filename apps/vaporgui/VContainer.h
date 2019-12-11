#pragma once

#include <QWidget>
#include <QHBoxLayout>

#include <string>

#include "VLineItem.h"

namespace {
    int LEFT_MARGIN 0
    int TOP_MARGIN 0
    int RIGHT_MARGIN 0
    int BOTTOM_MARGIN  0
}

class VContainer : public QWidget {
    Q_OBJECT

protected:
    VContainer( QWidget* containee );
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
