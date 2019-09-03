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

void TFMap::mousePressEvent      (QMouseEvent *event) { event->ignore(); }
void TFMap::mouseReleaseEvent    (QMouseEvent *event) { event->ignore(); }
void TFMap::mouseMoveEvent       (QMouseEvent *event) { event->ignore(); }
void TFMap::mouseDoubleClickEvent(QMouseEvent *event) { event->ignore(); }

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




TFMapWidget::TFMapWidget(TFMap *map)
{
    AddMap(map);
}

void TFMapWidget::AddMap(TFMap *map)
{
    if (std::find(_maps.begin(), _maps.end(), map) == _maps.end())
        _maps.push_back(map);
}

TFInfoWidget *TFMapWidget::GetInfoWidget()
{
    if (_maps.size())
        return _maps[0]->GetInfoWidget();
    return nullptr;
}

void TFMapWidget::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    for (auto map : _maps)
        map->Update(dataMgr, paramsMgr, rParams);
}

void TFMapWidget::Deactivate()
{
    for (auto map : _maps)
        map->Deactivate();
}

QSize TFMapWidget::minimumSizeHint() const
{
    QSize max(0, 0);
    for (auto map : _maps) {
        QSize s = map->minimumSizeHint();
        max.setWidth (std::max(max.width(),  s.width()));
        max.setHeight(std::max(max.height(), s.height()));
    }
    return max;
}

void TFMapWidget::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    for (int i = _maps.size() - 1; i >= 0; i--) {
        p.save();
        _maps[i]->paintEvent(p);
        p.restore();
    }
}

void TFMapWidget::mousePressEvent(QMouseEvent *event)
{
    for (auto map : _maps) {
        event->accept();
        map->mousePressEvent(event);
        if (event->isAccepted())
            break;
    }
}

void TFMapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    for (auto map : _maps) {
        event->accept();
        map->mouseReleaseEvent(event);
        if (event->isAccepted())
            break;
    }
}

void TFMapWidget::mouseMoveEvent(QMouseEvent *event)
{
    for (auto map : _maps) {
        event->accept();
        map->mouseMoveEvent(event);
        if (event->isAccepted())
            break;
    }
}

void TFMapWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    for (auto map : _maps) {
        event->accept();
        map->mouseDoubleClickEvent(event);
        if (event->isAccepted())
            break;
    }
}

void TFMapWidget::resizeEvent(QResizeEvent *event)
{
    for (auto map : _maps)
        map->resize(event->size().width(), event->size().height());
}
