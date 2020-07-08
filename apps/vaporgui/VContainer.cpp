#include <iostream>
#include <QEvent>
#include <QWidget>

#include "VContainer.h"

const int VContainer::_LEFT_MARGIN   = 0;
const int VContainer::_TOP_MARGIN    = 0;
const int VContainer::_RIGHT_MARGIN  = 0;
const int VContainer::_BOTTOM_MARGIN = 0;

VContainer::VContainer() 
: QFrame() 
{
    QHBoxLayout* layout = new QHBoxLayout;
    layout->setContentsMargins(
        _LEFT_MARGIN,
        _TOP_MARGIN,
        _RIGHT_MARGIN,
        _BOTTOM_MARGIN
    );
    setLayout( layout );

    setFrameStyle( QFrame::Panel );

    setSizePolicy( QSizePolicy::Maximum, QSizePolicy::Preferred );
}

MouseWheelWidgetAdjustmentGuard::MouseWheelWidgetAdjustmentGuard(QObject *parent) : QObject(parent)
{
}

bool MouseWheelWidgetAdjustmentGuard::eventFilter(QObject *o, QEvent *e)
{
    const QWidget* widget = dynamic_cast<QWidget*>(o);
    if (e->type() == QEvent::Wheel && widget && !widget->hasFocus())
    {
        e->ignore();
        return true;
    }

    return QObject::eventFilter(o, e);
}

QSize VContainer::sizeHint() const {
    QWidget* parent = this->parentWidget();
    if ( layout()->count() > 1 ) {
        return QSize( parent->width() / 2., 20 );
    }
    else {
        return QSize( parent->width() / 3. , 20 );
    }
}
