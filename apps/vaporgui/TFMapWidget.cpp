#include "TFMapWidget.h"
#include <QPainter>
#include <vapor/VAssert.h>

using glm::vec2;

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

glm::vec2 TFMapWidget::NDCToPixel(const glm::vec2 &v) const
{
    return vec2(PADDING + v.x * (width()-2*PADDING), PADDING + (1.0f - v.y) * (height()-2*PADDING));
}

QPointF TFMapWidget::NDCToQPixel(const glm::vec2 &v) const
{
    const vec2 p = NDCToPixel(v);
    return QPointF(p.x, p.y);
}

QPointF TFMapWidget::NDCToQPixel(float x, float y) const
{
    return NDCToQPixel(vec2(x, y));
}

glm::vec2 TFMapWidget::PixelToNDC(const glm::vec2 &p) const
{
    float width = QWidget::width();
    float height = QWidget::height();
    if (width == 0 || height == 0)
        return vec2(0);
    
    return vec2((p.x - PADDING) / (width-2*PADDING), 1.0f - (p.y - PADDING) / (height-2*PADDING));
}

glm::vec2 TFMapWidget::PixelToNDC(const QPointF &p) const
{
    return PixelToNDC(vec2(p.x(), p.y()));
}

QRectF TFMapWidget::PaddedRect() const
{
    return QRectF(PADDING, PADDING, width()-PADDING*2, height()-PADDING*2);
}

int TFMapWidget::GetPadding() const
{
    return PADDING;
}

int TFMapWidget::GetControlPointRadius() const
{
    return CONTROL_POINT_RADIUS;
}
