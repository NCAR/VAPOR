#include "TFColorWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include "TFColorInfoWidget.h"

using namespace VAPoR;
using glm::vec2;
using std::vector;

static vec2 qvec2(const QPoint &qp)  { return vec2(qp.x(), qp.y()); }
static vec2 qvec2(const QPointF &qp) { return vec2(qp.x(), qp.y()); }
static QPointF qvec2(const vec2 &v) { return QPointF(v.x, v.y); }

void TFColorWidget::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rp)
{
    _renderParams = rp;
    _paramsMgr = paramsMgr;
    update();
}

QSize TFColorWidget::minimumSizeHint() const
{
    return QSize(100, 30);
}

TFInfoWidget *TFColorWidget::CreateInfoWidget()
{
    TFColorInfoWidget *info = new TFColorInfoWidget;
    
    connect(this, SIGNAL(ControlPointDeselected()), info, SLOT(DeselectControlPoint()));
    connect(this, SIGNAL(UpdateInfo(float, QColor)), info, SLOT(SetControlPoint(float, QColor)));
    connect(info, SIGNAL(ControlPointChanged(float, QColor)), this, SLOT(UpdateFromInfo(float, QColor)));
    
    return info;
}

#define CONTROL_POINT_RADIUS (4.0f)
#define PADDING (CONTROL_POINT_RADIUS + 1.0f)

void TFColorWidget::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
    p.fillRect(0, 0, width(), height(), Qt::gray);
    
    if (_renderParams) {
        RenderParams *rp = _renderParams;
        
        ColorMap *cm = rp->GetMapperFunc(rp->GetVariableName())->GetColorMap();
        
        int nSamples = width()-PADDING*2;
        unsigned char buf[nSamples*3];
        float rgb[3];
        for (int i = 0; i < nSamples; i++) {
            cm->colorNormalized(i/(float)nSamples).toRGB(rgb);
            buf[i*3+0] = rgb[0]*255;
            buf[i*3+1] = rgb[1]*255;
            buf[i*3+2] = rgb[2]*255;
        }
        QImage image(buf, nSamples, 1, QImage::Format::Format_RGB888);
        
        p.drawImage(QRect(PADDING, PADDING, width()-PADDING*2, height()-PADDING*2), image);
        
        for (int i = 0; i < cm->numControlPoints(); i++) {
            drawControl(p, controlQPositionForValue(cm->controlPointValueNormalized(i)), i == _selectedId);
        }
    }
}

void TFColorWidget::mousePressEvent(QMouseEvent *event) {
    ColorMap *cm = getColormap();
    vec2 mouse(event->pos().x(), event->pos().y());
    
    for (int i = 0; i < cm->numControlPoints(); i++) {
        float value = cm->controlPointValueNormalized(i);
        if (controlPointContainsPixel(value, mouse)) {
            _isDraggingControl = true;
            _draggingControlID = i;
            selectControlPoint(i);
            _dragOffset = controlPositionForValue(value) - mouse;
            _paramsMgr->BeginSaveStateGroup("Colormap modification");
            return;
        }
    }
    
    DeselectControlPoint();
    event->ignore();
    update();
}

void TFColorWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (_isDraggingControl)
        _paramsMgr->EndSaveStateGroup();
    else
        event->ignore();
    _isDraggingControl = false;
}

void TFColorWidget::mouseMoveEvent(QMouseEvent *event) {
    vec2 mouse = qvec2(event->pos());
    
    if (_isDraggingControl) {
        float newVal = glm::clamp(valueForControlX(mouse.x + _dragOffset.x), 0.f, 1.f);
        
        moveControlPoint(&_draggingControlID, newVal);
        selectControlPoint(_draggingControlID);
        update();
    } else {
        event->ignore();
    }
}

void TFColorWidget::mouseDoubleClickEvent(QMouseEvent *event) {
    vec2 mouse = qvec2(event->pos());
    ColorMap *cm = getColormap();
    int selectedId = findSelectedControlPoint(mouse);
    if (selectedId >= 0) {
        cm->deleteControlPoint(selectedId);
        DeselectControlPoint();
        update();
        return;
    }
    
    float newVal = valueForControlX(mouse.x);
    if (newVal >= 0 && newVal <= 1)
        selectControlPoint(cm->addNormControlPointAt(newVal));
    
    update();
}

void TFColorWidget::moveControlPoint(int *index, float value, const VAPoR::ColorMap::Color &c)
{
    ColorMap *cm = getColormap();
    
    cm->deleteControlPoint(*index);
    *index = cm->addNormControlPoint(value, c);
}

void TFColorWidget::moveControlPoint(int *index, float value)
{
    ColorMap *cm = getColormap();
    ColorMap::Color c = cm->controlPointColor(_draggingControlID);
    moveControlPoint(index, value, c);
}

ColorMap *TFColorWidget::getColormap() const
{
    return _renderParams->GetMapperFunc(_renderParams->GetVariableName())->GetColorMap();
}

void TFColorWidget::selectControlPoint(int index)
{
    _selectedId = index;
    ColorMap *cm = getColormap();
    
    float value = cm->controlPointValueNormalized(index);
    ColorMap::Color vColor = cm->controlPointColor(index);
    
    UpdateInfo(value, VColorToQColor(vColor));
}

void TFColorWidget::DeselectControlPoint()
{
    _selectedId = -1;
    emit ControlPointDeselected();
}

void TFColorWidget::UpdateFromInfo(float value, QColor color)
{
    moveControlPoint(&_selectedId, value, QColorToVColor(color));
}

int TFColorWidget::findSelectedControlPoint(const glm::vec2 &mouse) const
{
    const ColorMap *cm = getColormap();
    const int n = cm->numControlPoints();
    for (int i = 0; i < n; i++)
        if (controlPointContainsPixel(cm->controlPointValueNormalized(i), mouse))
            return i;
    return -1;
}

bool TFColorWidget::controlPointContainsPixel(float cp, const vec2 &pixel) const
{
    return glm::distance(pixel, controlPositionForValue(cp)) <= CONTROL_POINT_RADIUS;
}

QPointF TFColorWidget::controlQPositionForValue(float value) const
{
    const vec2 v = controlPositionForValue(value);
    return QPointF(v.x, v.y);
}

glm::vec2 TFColorWidget::controlPositionForValue(float value) const
{
    return vec2(value * (width()-PADDING*2) + PADDING, height()/2.f);
}

float TFColorWidget::controlXForValue(float value) const
{
    return value * (width()-PADDING*2) + PADDING;
}

float TFColorWidget::valueForControlX(float position) const
{
    return (position - PADDING) / (width()-PADDING*2);
}

QColor TFColorWidget::VColorToQColor(const ColorMap::Color &c)
{
    float rgb[3];
    c.toRGB(rgb);
    return QColor(rgb[0]*255, rgb[1]*255, rgb[2]*255);
}

ColorMap::Color TFColorWidget::QColorToVColor(const QColor &c)
{
    double h, s, v;
    c.getHsvF(&h, &s, &v);
    return ColorMap::Color(h, s, v);
}
