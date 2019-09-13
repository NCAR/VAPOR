#include "TFIsoValueWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include "QPaintUtils.h"

using namespace VAPoR;
using glm::vec2;
using std::vector;

static vec2 qvec2(const QPoint &qp)  { return vec2(qp.x(), qp.y()); }
static vec2 qvec2(const QPointF &qp) { return vec2(qp.x(), qp.y()); }
static QPointF qvec2(const vec2 &v) { return QPointF(v.x, v.y); }

TFIsoValueMap::TFIsoValueMap(TFMapWidget *parent)
: TFMap(parent) {}

void TFIsoValueMap::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rp)
{
    _renderParams = rp;
    _paramsMgr = paramsMgr;
    update();
}

QSize TFIsoValueMap::minimumSizeHint() const
{
    return QSize(100, 30);
}

void TFIsoValueMap::Deactivate()
{
    DeselectControlPoint();
}

TFInfoWidget *TFIsoValueMap::createInfoWidget()
{
//    TFIsoValueInfoWidget *info = new TFIsoValueInfoWidget;
//
//    connect(this, SIGNAL(ControlPointDeselected()), info, SLOT(DeselectControlPoint()));
//    connect(this, SIGNAL(UpdateInfo(float, QColor)), info, SLOT(SetControlPoint(float, QColor)));
//    connect(info, SIGNAL(ControlPointChanged(float, QColor)), this, SLOT(UpdateFromInfo(float, QColor)));
//
//    return info;
    return nullptr;
}

#define CONTROL_POINT_RADIUS (4.0f)
#define PADDING (CONTROL_POINT_RADIUS + 1.0f)

void TFIsoValueMap::paintEvent(QPainter &p)
{
    //     243 245 249
    p.fillRect(rect(), Qt::lightGray);
    QPaintUtils::BoxDropShadow(p, paddedRect(), 10, QColor(0,0,0,120));
    
    if (_renderParams) {
        RenderParams *rp = _renderParams;
        
        ColorMap *cm = rp->GetMapperFunc(rp->GetVariableName())->GetColorMap();
        
        
        int nSamples = paddedRect().width();
        unsigned char buf[nSamples*3];
        float rgb[3];
        for (int i = 0; i < nSamples; i++) {
            cm->colorNormalized(i/(float)nSamples).toRGB(rgb);
            buf[i*3+0] = rgb[0]*255;
            buf[i*3+1] = rgb[1]*255;
            buf[i*3+2] = rgb[2]*255;
        }
        QImage image(buf, nSamples, 1, QImage::Format::Format_RGB888);
        
        p.drawImage(paddedRect(), image);
        
        for (int i = 0; i < _isoValues.size(); i++) {
            drawControl(p, controlQPositionForValue(_isoValues[i]), i == _selectedId);
        }
    }
}

void TFIsoValueMap::drawControl(QPainter &p, const QPointF &pos, bool selected) const
{
    float r = CONTROL_POINT_RADIUS;
    float t = GetControlPointTriangleHeight();
    float s = GetControlPointSquareHeight();
    
    QPen pen(Qt::darkGray, 0.5);
    QBrush brush(QColor(0xfa, 0xfa, 0xfa));
    p.setBrush(brush);
    p.setPen(pen);
    
//    p.drawEllipse(pos, radius, radius);
    
    QPolygonF graph;
    graph.push_back(pos + QPointF( 0,  0));
    graph.push_back(pos + QPointF(-r,  t));
    graph.push_back(pos + QPointF(-r,  t+s));
    graph.push_back(pos + QPointF( r,  t+s));
    graph.push_back(pos + QPointF( r,  t));
    
    p.drawPolygon(graph);
}

float TFIsoValueMap::GetControlPointTriangleHeight() const
{
    return GetControlPointRadius() * 2 * 0.618;
}

float TFIsoValueMap::GetControlPointSquareHeight() const
{
    return GetControlPointRadius() * 1.618;
}

QRect TFIsoValueMap::GetControlPointArea(const QPoint &p) const
{
    float h = GetControlPointSquareHeight() + GetControlPointTriangleHeight();
    float r = GetControlPointRadius();
    return QRect(p-QPoint(r, 0), p+QPoint(r, h));
}

void TFIsoValueMap::mousePressEvent(QMouseEvent *event)
{
    emit Activated(this);
    vec2 mouse(event->pos().x(), event->pos().y());
    
    for (int i = 0; i < _isoValues.size(); i++) {
        float value = _isoValues[i];
        if (controlPointContainsPixel(value, mouse)) {
            _isDraggingControl = true;
            _draggingControlID = i;
            selectControlPoint(i);
            update();
            _dragOffset = controlPositionForValue(value) - mouse;
            _paramsMgr->BeginSaveStateGroup("Colormap modification");
            return;
        }
    }
    
    DeselectControlPoint();
    event->ignore();
    update();
}

void TFIsoValueMap::mouseReleaseEvent(QMouseEvent *event) {
    if (_isDraggingControl)
        _paramsMgr->EndSaveStateGroup();
    else
        event->ignore();
    _isDraggingControl = false;
}

void TFIsoValueMap::mouseMoveEvent(QMouseEvent *event) {
    vec2 mouse = qvec2(event->pos());
    
    if (_isDraggingControl) {
        float newVal = glm::clamp(valueForControlX(mouse.x + _dragOffset.x), 0.f, 1.f);
        
        moveControlPoint(&_draggingControlID, newVal);
        selectControlPoint(_draggingControlID);
        update();
        _paramsMgr->IntermediateChange();
    } else {
        event->ignore();
    }
}

void TFIsoValueMap::mouseDoubleClickEvent(QMouseEvent *event) {
    vec2 mouse = qvec2(event->pos());
    ColorMap *cm = getColormap();
    int selectedId = findSelectedControlPoint(mouse);
    if (selectedId >= 0) {
        deleteControlPoint(selectedId);
        DeselectControlPoint();
        update();
        return;
    }
    
    float newVal = valueForControlX(mouse.x);
    if (newVal >= 0 && newVal <= 1)
        selectControlPoint(addControlPoint(newVal));
    
    update();
}

int TFIsoValueMap::addControlPoint(float value)
{
    for (int i = 0; i < _isoValues.size(); i++) {
        if (value < _isoValues[i]) {
            _isoValues.insert(_isoValues.begin()+i, value);
            return i;
        }
    }
    _isoValues.push_back(value);
    return _isoValues.size()-1;
}

void TFIsoValueMap::deleteControlPoint(int i)
{
    _isoValues.erase(_isoValues.begin() + i);
}

void TFIsoValueMap::moveControlPoint(int *index, float value)
{
    deleteControlPoint(*index);
    *index = addControlPoint(value);
}

ColorMap *TFIsoValueMap::getColormap() const
{
    return _renderParams->GetMapperFunc(_renderParams->GetVariableName())->GetColorMap();
}

void TFIsoValueMap::selectControlPoint(int index)
{
    _selectedId = index;
    ColorMap *cm = getColormap();
    
    float value = cm->controlPointValueNormalized(index);
    ColorMap::Color vColor = cm->controlPointColor(index);
    
    UpdateInfo(value, VColorToQColor(vColor));
}

void TFIsoValueMap::DeselectControlPoint()
{
    _selectedId = -1;
    emit ControlPointDeselected();
    update();
}

void TFIsoValueMap::UpdateFromInfo(float value, QColor color)
{
//    moveControlPoint(&_selectedId, value, QColorToVColor(color));
}

int TFIsoValueMap::findSelectedControlPoint(const glm::vec2 &mouse) const
{
    for (int i = 0; i < _isoValues.size(); i++)
        if (controlPointContainsPixel(_isoValues[i], mouse))
            return i;
    return -1;
}

bool TFIsoValueMap::controlPointContainsPixel(float cp, const vec2 &p) const
{
    QRect rect = GetControlPointArea(controlQPositionForValue(cp));
    return rect.contains(QPoint(p.x, p.y));
}

QPoint TFIsoValueMap::controlQPositionForValue(float value) const
{
    const vec2 v = controlPositionForValue(value);
    return QPoint(v.x, v.y);
}

glm::vec2 TFIsoValueMap::controlPositionForValue(float value) const
{
    return vec2(controlXForValue(value), 0);
}

float TFIsoValueMap::controlXForValue(float value) const
{
    return NDCToPixel(vec2(value, 0.f)).x;
}

float TFIsoValueMap::valueForControlX(float position) const
{
    return PixelToNDC(vec2(position, 0.f)).x;
}

QColor TFIsoValueMap::VColorToQColor(const ColorMap::Color &c)
{
    float rgb[3];
    c.toRGB(rgb);
    return QColor(rgb[0]*255, rgb[1]*255, rgb[2]*255);
}

ColorMap::Color TFIsoValueMap::QColorToVColor(const QColor &c)
{
    double h, s, v;
    c.getHsvF(&h, &s, &v);
    return ColorMap::Color(h, s, v);
}
