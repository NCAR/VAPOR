#include <iostream>
#include <QEvent>
#include <QWidget>

#include "VHBoxWidget.h"

const int VHBoxWidget::_LEFT_MARGIN = 0;
const int VHBoxWidget::_TOP_MARGIN = 0;
const int VHBoxWidget::_RIGHT_MARGIN = 0;
const int VHBoxWidget::_BOTTOM_MARGIN = 0;

VHBoxWidget::VHBoxWidget() : QFrame()
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(_LEFT_MARGIN, _TOP_MARGIN, _RIGHT_MARGIN, _BOTTOM_MARGIN);
    setLayout(layout);

    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
}

MouseWheelWidgetAdjustmentGuard::MouseWheelWidgetAdjustmentGuard(QObject *parent) : QObject(parent) {}

bool MouseWheelWidgetAdjustmentGuard::eventFilter(QObject *o, QEvent *e)
{
    const QWidget *widget = dynamic_cast<QWidget *>(o);
    if (e->type() == QEvent::Wheel && widget && !widget->hasFocus()) {
        e->ignore();
        return true;
    }

    return QObject::eventFilter(o, e);
}

// QSize VContainer::sizeHint() const {
//    QWidget* parent = this->parentWidget();
//    if ( layout()->count() > 1 ) {
//        return QSize( parent->width() / 2., 20 );
//    }
//    else {
//        return QSize( parent->width() / 3. , 20 );
//    }
//}
