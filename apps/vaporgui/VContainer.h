#pragma once

#include <QWidget>
#include <QHBoxLayout>

#include <string>

#include "VLineItem.h"

#define LEFT_MARGIN 0
#define TOP_MARGIN 0
#define RIGHT_MARGIN 0
#define BOTTOM_MARGIN  0

class VContainer : public QWidget {
    Q_OBJECT

public:
    void Hide(); 
    void Show();
    
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
