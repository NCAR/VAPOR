#include "TFColorWidget.h"
#include <QPainter>
#include <QMouseEvent>

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
        
//        printf("============= %i Controls ==============\n", cm->numControlPoints());
//        for (int i = 0; i < cm->numControlPoints(); i++) {
//            float rgb[3];
//            cm->controlPointColor(i).toRGB(rgb);
//            printf("[%i] %0.2f (%0.1f, %0.1f, %0.1f)\n", i, cm->controlPointValueNormalized(i), rgb[0], rgb[1], rgb[2]);
//        }
        
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
        
        QPen pen(Qt::darkGray, 0.8);
        pen.setWidth(0.5);
        p.setPen(pen);
        QBrush brush(QColor(0xFA, 0xFA, 0xFA));
        p.setBrush(brush);
        
        for (int i = 0; i < cm->numControlPoints(); i++) {
            p.drawEllipse(controlQPositionForValue(cm->controlPointValueNormalized(i)), CONTROL_POINT_RADIUS, CONTROL_POINT_RADIUS);
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
            _dragOffset = controlPositionForValue(value) - mouse;
            _paramsMgr->BeginSaveStateGroup("Colormap modification");
            return;
        }
    }
    
    event->ignore();
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
        
        ColorMap *cm = getColormap();
        ColorMap::Color c = cm->controlPointColor(_draggingControlID);
        cm->deleteControlPoint(_draggingControlID);
        _draggingControlID = cm->addNormControlPoint(newVal, c);
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
        update();
        return;
    }
    
    float newVal = valueForControlX(mouse.x);
    if (newVal >= 0 && newVal <= 1)
        cm->addNormControlPointAt(newVal);
    
    update();
}

ColorMap *TFColorWidget::getColormap() const
{
    return _renderParams->GetMapperFunc(_renderParams->GetVariableName())->GetColorMap();
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
