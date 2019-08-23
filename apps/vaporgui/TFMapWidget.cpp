#include "TFMapWidget.h"
#include <QPainter>

#define CONTROL_POINT_RADIUS (4.0f)
#define PADDING (CONTROL_POINT_RADIUS + 1.0f)

TFInfoWidget *TFMapWidget::GetInfoWidget()
{
    if (!_infoWidget)
        _infoWidget = createInfoWidget();
    
    return _infoWidget;
}

void TFMapWidget::drawControl(QPainter &p, const QPointF &pos, bool selected) const
{
    QPen pen(Qt::darkGray, 0.5);
    QBrush brush(QColor(0xfa, 0xfa, 0xfa));
    float radius = CONTROL_POINT_RADIUS;
    
    //    if (selected) {
    //        pen.setColor(Qt::black);
    //        pen.setWidth(1.5);
    //
    //        brush.setColor(Qt::white);
    //    }
    
    p.setBrush(brush);
    p.setPen(pen);
    
    p.drawEllipse(pos, radius, radius);
    
    if (selected) {
        p.setPen(Qt::NoPen);
        p.setBrush(QBrush(Qt::black));
        radius *= 0.38;
        p.drawEllipse(pos, radius, radius);
    }
}
