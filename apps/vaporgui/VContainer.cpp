#include <QEvent>
#include <QWidget>

#include "VContainer.h"

VContainer::VContainer( QWidget* containee ) 
: QWidget() 
{
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(
        LEFT_MARGIN,
        TOP_MARGIN,
        RIGHT_MARGIN,
        BOTTOM_MARGIN
    );
    setLayout( layout );

    setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
}

MouseWheelWidgetAdjustmentGuard::MouseWheelWidgetAdjustmentGuard(QObject *parent) : QObject(parent)
{
}

bool MouseWheelWidgetAdjustmentGuard::eventFilter(QObject *o, QEvent *e)
{
    const QWidget* widget = static_cast<QWidget*>(o);
    if (e->type() == QEvent::Wheel && widget && !widget->hasFocus())
    {
        e->ignore();
        return true;
    }

    return QObject::eventFilter(o, e);
}
