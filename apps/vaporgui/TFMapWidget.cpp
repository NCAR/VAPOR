#include "TFMapWidget.h"
#include <QPainter>
#include <QResizeEvent>
#include <QMenu>
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
    float radius = CONTROL_POINT_RADIUS;
    
    if (selected) {
        QRadialGradient gradient(0.5, 0.5, 0.4);
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        gradient.setColorAt(0, QColor(0, 0, 0, 100));
        gradient.setColorAt(1, Qt::transparent);
        QBrush shadowBrush(gradient);
    
    
        p.setPen(Qt::NoPen);
        p.setBrush(shadowBrush);
        p.drawEllipse(pos, radius*2, radius*2);
    }
    
    QPen pen(Qt::darkGray, 0.5);
    QBrush brush(QColor(0xfa, 0xfa, 0xfa));
    
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
    const QRect p = paddedRect();
    return vec2(p.x() + v.x * p.width(), p.y() + (1.0f - v.y) * p.height());
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

QRect TFMap::paddedRect() const
{
    const QMargins p = GetPadding();
    return QRect(p.left(), p.top(), width()-(p.left()+p.right()), height()-(p.top()+p.bottom()));
}

QRect TFMap::rect() const
{
    return QRect(0, 0, width(), height());
}

const QFont TFMap::getFont() const
{
    if (_parent)
        return _parent->font();
    return QFont();
}

QMargins TFMap::GetPadding() const
{
    return QMargins(PADDING, PADDING, PADDING, PADDING);
}

int TFMap::GetControlPointRadius() const
{
    return CONTROL_POINT_RADIUS;
}




TFMapWidget::TFMapWidget(TFMap *map)
{
    AddMap(map);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(_showContextMenu(const QPoint &)));
}

TFMapWidget::~TFMapWidget()
{
    for (TFMap *map : _maps) {
        delete map;
    }
}

void TFMapWidget::AddMap(TFMap *map)
{
    if (std::find(_maps.begin(), _maps.end(), map) == _maps.end()) {
        _maps.push_back(map);
        connect(map, SIGNAL(Activated(TFMap*)), this, SLOT(_mapActivated(TFMap*)));
    }
}

std::vector<TFMap *> TFMapWidget::GetMaps() const
{
    return _maps;
}

TFInfoWidget *TFMapWidget::GetInfoWidget()
{
    assert(_maps.size() == 1);
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

void TFMapWidget::_mapActivated(TFMap *who)
{
    for (auto map : _maps)
        if (map != who)
            map->Deactivate();
    
    emit Activated(this);
}

void TFMapWidget::_showContextMenu(const QPoint &qp)
{
    vec2 p(qp.x(), qp.y());
    QMenu menu("Context Menu", this);
    
    for (auto map : _maps) {
        map->PopulateContextMenu(&menu, p);
        menu.addSeparator();;
    }
    
    menu.exec(mapToGlobal(qp));
}

#include <vapor/GLManager.h>
void TFMapWidget::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
//    void *s = VAPoR::GLManager::BeginTimer();
    for (int i = _maps.size() - 1; i >= 0; i--) {
        p.save();
        _maps[i]->paintEvent(p);
        p.restore();
    }
//    printf("Paint took %fs\n", VAPoR::GLManager::EndTimer(s));
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
