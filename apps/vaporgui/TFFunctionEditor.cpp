#include "TFFunctionEditor.h"
#include <QPaintEvent>
#include <glm/glm.hpp>

using std::vector;
using glm::vec2;
using glm::clamp;

vec2 qvec2(const QPoint &qp)
{
    return vec2(qp.x(), qp.y());
}

QPointF qvec2(const vec2 &v)
{
    return QPointF(v.x, v.y);
}

TFFunctionEditor::TFFunctionEditor()
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    
    _controlPoints.Add(vec2(50,50));
    _controlPoints.Add(vec2(70,40));
}

void TFFunctionEditor::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
}

QSize TFFunctionEditor::minimumSizeHint() const
{
    return QSize(100, 100);
}

#define CONTROL_POINT_RADIUS (4.0f)

void TFFunctionEditor::paintEvent(QPaintEvent* event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    
//    p.fillRect(event->rect(), QBrush(QColor(64, 32, 64)));
    
    if (_controlPoints.Size()) {
        ControlPointList &cp = _controlPoints;
        const int N = cp.Size();
        
        
        p.drawLine(QLine(0, cp[0].y, cp[0].x, cp[0].y));
        
        for (auto it = cp.BeginLines(); it != cp.EndLines(); ++it) {
            vec2 a = it.a();
            vec2 b = it.b();
            p.drawLine(QLine(QPoint(a.x, a.y), QPoint(b.x, b.y)));
            
//            vec2 a = cp[i];
//            vec2 b = cp[i+1];
//
//            vec2 n = glm::normalize(b-a);
//            float t = glm::dot(n, m-a);
//            vec2 isect = n * t + a;
//            p.drawEllipse(qvec2(isect), 2, 2);
        }
        
        p.drawLine(QLine(cp[N-1].x, cp[N-1].y, width(), cp[N-1].y));
        
        
        
        QPen pen(Qt::darkGray, 0.8);
        pen.setWidth(0.5);
        p.setPen(pen);
        QBrush brush(QColor(0xFA, 0xFA, 0xFA));
        p.setBrush(brush);
        
        for (auto it = cp.BeginPoints(); it != cp.EndPoints(); ++it)
            p.drawEllipse(QPointF((*it).x, (*it).y), CONTROL_POINT_RADIUS, CONTROL_POINT_RADIUS);
        
    }
}

void TFFunctionEditor::mousePressEvent(QMouseEvent *event)
{
    vec2 mouse(event->posF().x(), event->posF().y());
    for (int i = 0; i < _controlPoints.Size(); i++) {
        if (glm::distance(mouse, _controlPoints[i]) <= CONTROL_POINT_RADIUS) {
            _draggedID = i;
            _dragOffset = _controlPoints[i] - mouse;
            break;
        }
    }
}

void TFFunctionEditor::mouseReleaseEvent(QMouseEvent *event)
{
    _draggedID = -1;
}

void TFFunctionEditor::mouseMoveEvent(QMouseEvent *event)
{
    vec2 mouse = qvec2(event->pos());
    m = mouse;
    const int i = _draggedID;
    ControlPointList &cp = _controlPoints;
    const int N = cp.Size();
    
    if (_draggedID >= 0) {
        vec2 newPos = mouse + _dragOffset;
        newPos = glm::clamp(
                            newPos,
                            vec2(i>0?cp[i-1].x:0, 0),
                            vec2(i<N-1?cp[i+1].x:width(), height()));
        
        _controlPoints[_draggedID] = newPos;
    }
    update();
}

vec2 project(vec2 a, vec2 b, vec2 p)
{
    vec2 n = glm::normalize(b-a);
    float t = glm::dot(n, p-a);
    
    return n * t + a;
}

float dist(vec2 a, vec2 b, vec2 p)
{
    vec2 n = glm::normalize(b-a);
    float t = glm::dot(n, p-a);
    
    if (t < 0) return glm::distance(a, p);
    if (t > glm::distance(a, b)) return glm::distance(b, p);
    
    vec2 projection = n * t + a;
    return glm::distance(projection, p);
}

void TFFunctionEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
    printf("Double click\n");
    
    vec2 mouse = qvec2(event->pos());
    ControlPointList &cp = _controlPoints;
    
    if (dist(vec2(0, cp[0].y), cp[0], mouse) <= CONTROL_POINT_RADIUS)
        cp.Add(project(vec2(0, cp[0].y), cp[0], mouse), 0);
    
    for (int i = 0; i < cp.Size()-1; i++) {
        if (dist(cp[i], cp[i+1], mouse) <= CONTROL_POINT_RADIUS) {
            cp.Add(project(cp[i], cp[i+1], mouse), i+1);
            break;
        }
    }
    
    if (dist(cp[cp.Size()-1], vec2(0, cp[cp.Size()-1].y), mouse) <= CONTROL_POINT_RADIUS)
        cp.Add(project(cp[cp.Size()-1], vec2(0, cp[cp.Size()-1].y), mouse), cp.Size()-1);
    
    update();
}

QPointF TFFunctionEditor::NDCToPixel(const glm::vec2 &v) const
{
    return QPointF(v.x * width(), v.y * height());
}

glm::vec2 TFFunctionEditor::PixelToNDC(const QPointF &p) const
{
    float width = QWidget::width();
    float height = QWidget::height();
    VAssert(width != 0 && height != 0);
    
    return vec2(p.x() / width, p.y() / height);
}

























/*
TFFunctionEditor::TFFunctionEditor()
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    this->setMinimumHeight(100);
}

void TFFunctionEditor::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    this->setMaximumSize(width()-1, height()-1);
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    
    updateGL();
}

void TFFunctionEditor::paintGL()
{
    bool swapped = false;
    if (!swapped) {
//        swapBuffers();
        swapped = true;
    }
    printf("%s\n", glGetString(GL_VERSION));
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glColor3f(0, 0, 1);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    glVertex2f(1, 1);
    glVertex2f(0, 1);
    glEnd();
}

void TFFunctionEditor::resizeGL(int w, int h)
{
//    swapBuffers();
}
*/
