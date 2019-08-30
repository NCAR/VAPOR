#include "TFMapWidget.h"
#include <QPainter>
#include <QResizeEvent>
#include <vapor/VAssert.h>

using glm::vec2;

#define CONTROL_POINT_RADIUS (4.0f)
#define PADDING (CONTROL_POINT_RADIUS + 1.0f)

TFMap::TFMap(TFMapWidget *parent)
: _parent(parent) {}

TFInfoWidget *TFMap::GetInfoWidget()
{
    if (!_infoWidget)
        _infoWidget = createInfoWidget();
    
    return _infoWidget;
}

void TFMap::resize(int width, int height)
{
    _width = width;
    _height = height;
}

void TFMap::drawControl(QPainter &p, const QPointF &pos, bool selected) const
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

void TFMap::update()
{
    if (_parent)
        _parent->update();
}

glm::vec2 TFMap::NDCToPixel(const glm::vec2 &v) const
{
    return vec2(PADDING + v.x * (width()-2*PADDING), PADDING + (1.0f - v.y) * (height()-2*PADDING));
}

QPointF TFMap::NDCToQPixel(const glm::vec2 &v) const
{
    const vec2 p = NDCToPixel(v);
    return QPointF(p.x, p.y);
}

QPointF TFMap::NDCToQPixel(float x, float y) const
{
    return NDCToQPixel(vec2(x, y));
}

glm::vec2 TFMap::PixelToNDC(const glm::vec2 &p) const
{
    float width = TFMap::width();
    float height = TFMap::height();
    if (width == 0 || height == 0)
        return vec2(0);
    
    return vec2((p.x - PADDING) / (width-2*PADDING), 1.0f - (p.y - PADDING) / (height-2*PADDING));
}

glm::vec2 TFMap::PixelToNDC(const QPointF &p) const
{
    return PixelToNDC(vec2(p.x(), p.y()));
}

QRectF TFMap::PaddedRect() const
{
    return QRectF(PADDING, PADDING, width()-PADDING*2, height()-PADDING*2);
}

QRectF TFMap::rect() const
{
    return QRectF(0, 0, width(), height());
}

int TFMap::GetPadding() const
{
    return PADDING;
}

int TFMap::GetControlPointRadius() const
{
    return CONTROL_POINT_RADIUS;
}




TFInfoWidget *TFMapWidget::GetInfoWidget()
{
    if (_map)
        return _map->GetInfoWidget();
    return nullptr;
}

void TFMapWidget::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    if (_map)
        _map->Update(dataMgr, paramsMgr, rParams);
}

void TFMapWidget::Deactivate()
{
    if (_map)
        _map->Deactivate();
}

QSize TFMapWidget::minimumSizeHint() const
{
    if (_map)
        return _map->minimumSizeHint();
    return QSize(0, 0);
}

void TFMapWidget::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    if (_map)
        _map->paintEvent(p);
}

void TFMapWidget::mousePressEvent(QMouseEvent *event)
{
    if (_map)
        _map->mousePressEvent(event);
}

void TFMapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (_map)
        _map->mouseReleaseEvent(event);
}

void TFMapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (_map)
        _map->mouseMoveEvent(event);
}

void TFMapWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (_map)
        _map->mouseDoubleClickEvent(event);
}

void TFMapWidget::resizeEvent(QResizeEvent *event)
{
    if (_map)
        _map->resize(event->size().width(), event->size().height());
}
