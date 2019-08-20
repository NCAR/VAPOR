#include "TFOpacityWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <glm/glm.hpp>

using namespace VAPoR;
using std::vector;
using glm::vec2;
using glm::clamp;

static vec2 qvec2(const QPoint &qp)  { return vec2(qp.x(), qp.y()); }
static vec2 qvec2(const QPointF &qp) { return vec2(qp.x(), qp.y()); }
static QPointF qvec2(const vec2 &v) { return QPointF(v.x, v.y); }

vec2 Project(vec2 a, vec2 b, vec2 p)
{
    vec2 n = glm::normalize(b-a);
    float t = glm::dot(n, p-a);
    
    return n * t + a;
}

float DistanceToLine(vec2 a, vec2 b, vec2 p)
{
    vec2 n = glm::normalize(b-a);
    float t = glm::dot(n, p-a);
    
    if (t < 0) return glm::distance(a, p);
    if (t > glm::distance(a, b)) return glm::distance(b, p);
    
    vec2 projection = n * t + a;
    return glm::distance(projection, p);
}






TFOpacityWidget::TFOpacityWidget()
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    this->setFrameStyle(QFrame::Box);
    _controlPoints.Add(vec2(0,0));
    _controlPoints.Add(vec2(0.2,0.5));
    _controlPoints.Add(vec2(0.5,0.8));
}

void TFOpacityWidget::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rp)
{
    _renderParams = rp;
    
    MapperFunction *mf = rp->GetMapperFunc(rp->GetVariableName());
    // TODO Multiple opacity maps?
    //    int n = mf->getNumOpacityMaps();
    //    printf("# opacity maps = %i\n", n);
    
    OpacityMap *om = mf->GetOpacityMap(0);
    
    vector<double> cp = om->GetControlPoints();
    _controlPoints.Resize(cp.size()/2);
    for (int i = 0; i < cp.size(); i+=2) {
        _controlPoints[i/2].y = cp[i];
        _controlPoints[i/2].x = cp[i+1];
    }
}

QSize TFOpacityWidget::minimumSizeHint() const
{
    return QSize(100, 75);
}

#define CONTROL_POINT_RADIUS (4.0f)
#define PADDING (CONTROL_POINT_RADIUS + 1.0f)

void TFOpacityWidget::paintEvent(QPaintEvent* event)
{
    QFrame::paintEvent(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
//    p.setViewport(10, 10, 30, 30);
//    p.setWindow(10, 10, 30, 30);
    
//    p.fillRect(event->rect(), QBrush(QColor(64, 32, 64)));
    
    if (_controlPoints.Size()) {
        ControlPointList &cp = _controlPoints;
        
        for (auto it = cp.BeginLines(); it != cp.EndLines(); ++it) {
            p.drawLine(QNDCToPixel(it.a()), QNDCToPixel(it.b()));
            
//            p.drawEllipse(qvec2(Project(NDCToPixel(it.a()), NDCToPixel(it.b()), m)), 2, 2);
        }
        
        for (auto it = --cp.EndPoints(); it != --cp.BeginPoints(); --it)
            drawControl(p, *it, it.Index() == _selectedControl);
    }
}

void TFOpacityWidget::drawControl(QPainter &p, glm::vec2 ndc, bool selected) const
{
    QPen pen(Qt::darkGray, 0.5);
    QBrush brush(QColor(0xfa, 0xfa, 0xfa));
    float radius = CONTROL_POINT_RADIUS;
    
    if (selected) {
        pen.setColor(Qt::black);
        pen.setWidth(1.5);
        
        brush.setColor(Qt::white);
    }
    
    p.setBrush(brush);
    p.setPen(pen);
    
    p.drawEllipse(QNDCToPixel(ndc), radius, radius);
}

void TFOpacityWidget::mousePressEvent(QMouseEvent *event)
{
    vec2 mouse = qvec2(event->posF());
    auto it = findSelectedControlPoint(mouse);
    
    if (it != _controlPoints.EndPoints()) {
        _draggedControl = it;
        _dragOffset = NDCToPixel(*it) - mouse;
        _isDraggingControl = true;
        _controlStartValue = *it;
        selectControlPoint(it);
    } else {
        DeselectControlPoint();
        event->ignore();
    }
}

void TFOpacityWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (_isDraggingControl)
        if (*_draggedControl != _controlStartValue)
            opacityChanged();
    _isDraggingControl = false;
}

void TFOpacityWidget::mouseMoveEvent(QMouseEvent *event)
{
    vec2 mouse = qvec2(event->pos());
    m = mouse;
//    const int i = _draggedID;
//    ControlPointList &cp = _controlPoints;
//    const int N = cp.Size();
    
    if (_isDraggingControl) {
        const auto &it = _draggedControl;
        vec2 newVal = glm::clamp(
                            PixelToNDC(mouse + _dragOffset),
                            vec2(it.IsFirst() ? 0 : (*(it-1)).x, 0),
                            vec2(it.IsLast() ? 1 : (*(it+1)).x, 1));
        
        *_draggedControl = newVal;
        update();
    } else {
        event->ignore();
    }
}

void TFOpacityWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    vec2 mouse = qvec2(event->pos());
    ControlPointList &cp = _controlPoints;
    
    auto controlPointIt = findSelectedControlPoint(mouse);
    if (controlPointIt != cp.EndPoints()) {
        cp.Remove(controlPointIt);
        opacityChanged();
        DeselectControlPoint();
        update();
        return;
    }
    
    for (auto it = cp.BeginLines(); it != cp.EndLines(); ++it) {
        const vec2 a = NDCToPixel(it.a());
        const vec2 b = NDCToPixel(it.b());
        
        if (DistanceToLine(a, b, mouse) <= CONTROL_POINT_RADIUS) {
            cp.Add(PixelToNDC(Project(a, b, mouse)), it);
            opacityChanged();
            update();
            return;
        }
    }
}

void TFOpacityWidget::opacityChanged()
{
    MapperFunction *mf = _renderParams->GetMapperFunc(_renderParams->GetVariableName());
    
    OpacityMap *om = mf->GetOpacityMap(0);
    
    vector<double> cp(_controlPoints.Size() * 2);
    for (int i = 0; i < _controlPoints.Size(); i++) {
        cp[i*2]   = _controlPoints[i].y;
        cp[i*2+1] = _controlPoints[i].x;
    }
    om->SetControlPoints(cp);
}

bool TFOpacityWidget::controlPointContainsPixel(const vec2 &cp, const vec2 &pixel) const
{
    return glm::distance(pixel, NDCToPixel(cp)) <= CONTROL_POINT_RADIUS;
}

ControlPointList::PointIterator TFOpacityWidget::findSelectedControlPoint(const glm::vec2 &mouse)
{
    const auto end = _controlPoints.EndPoints();
    for (auto it = _controlPoints.BeginPoints(); it != end; ++it)
        if (controlPointContainsPixel(*it, mouse))
            return it;
    return end;
}

glm::vec2 TFOpacityWidget::NDCToPixel(const glm::vec2 &v) const
{
    return vec2(PADDING + v.x * (width()-2*PADDING), PADDING + (1.0f - v.y) * (height()-2*PADDING));
}

QPointF TFOpacityWidget::QNDCToPixel(const glm::vec2 &v) const
{
    const vec2 p = NDCToPixel(v);
    return QPointF(p.x, p.y);
}

glm::vec2 TFOpacityWidget::PixelToNDC(const glm::vec2 &p) const
{
    float width = QWidget::width();
    float height = QWidget::height();
    VAssert(width != 0 && height != 0);
    
    return vec2((p.x - PADDING) / (width-2*PADDING), 1.0f - (p.y - PADDING) / (height-2*PADDING));
}

glm::vec2 TFOpacityWidget::PixelToNDC(const QPointF &p) const
{
    return PixelToNDC(vec2(p.x(), p.y()));
}

void TFOpacityWidget::selectControlPoint(ControlPointList::PointIterator it)
{
    _selectedControl = it.Index();
    update();
    emit ControlPointSelected(it.Index());
}

void TFOpacityWidget::DeselectControlPoint()
{
    _selectedControl = -1;
    update();
    emit ControlPointDeselected();
}
