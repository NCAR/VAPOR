#include "TFOpacityWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <glm/glm.hpp>
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include "TFOpacityInfoWidget.h"
#include <QAction>

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


#define CONTROL_POINT_RADIUS (4.0f)
#define PADDING (CONTROL_POINT_RADIUS + 1.0f)

TFOpacityMap::TFOpacityMap(TFMapWidget *parent)
: TFMap(parent) {}

void TFOpacityMap::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rp)
{
    _renderParams = rp;
    _paramsMgr = paramsMgr;
    
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
    update();
}

QSize TFOpacityMap::minimumSizeHint() const
{
    return QSize(100, 75);
}

void TFOpacityMap::Deactivate()
{
    DeselectControlPoint();
}

QList<QAction *> TFOpacityMap::GetActionsForLocation(const glm::vec2 &p)
{
    QList<QAction *> actions;
    auto selected = findSelectedControlPoint(p);
    if (selected != _controlPoints.EndPoints()) {
        QAction *deleteControl = new QAction("Delete control point", nullptr);
        connect(deleteControl, SIGNAL(triggered()), this, SLOT(menuDeleteSelectedControlPoint()));
        actions.push_back(deleteControl);
    } else {
        QAction *addControl = new QAction("Add control point", nullptr);
        connect(addControl, SIGNAL(triggered()), this, SLOT(menuAddControlPoint()));
        actions.push_back(addControl);
        addControl->setProperty("location", QVariant(qvec2(PixelToNDC(p))));
    }
    return actions;
}

TFInfoWidget *TFOpacityMap::createInfoWidget()
{
    TFOpacityInfoWidget *info = new TFOpacityInfoWidget;
    connect(info, SIGNAL(ControlPointChanged(float, float)), this, SLOT(UpdateFromInfo(float, float)));
    connect(this, SIGNAL(UpdateInfo(float, float)), info, SLOT(SetControlPoint(float, float)));
    connect(this, SIGNAL(ControlPointDeselected()), info, SLOT(DeselectControlPoint()));
    
    return info;
}

void TFOpacityMap::paintEvent(QPainter &p)
{
//    p.setViewport(10, 10, 30, 30);
//    p.setWindow(10, 10, 30, 30);
    
//    p.fillRect(event->rect(), QBrush(QColor(64, 32, 64)));
    
    if (_controlPoints.Size()) {
        ControlPointList &cp = _controlPoints;
        
        for (auto it = cp.BeginLines(); it != cp.EndLines(); ++it) {
            p.drawLine(NDCToQPixel(it.a()), NDCToQPixel(it.b()));
            
//            p.drawEllipse(qvec2(Project(NDCToPixel(it.a()), NDCToPixel(it.b()), m)), 2, 2);
        }
        
        for (auto it = --cp.EndPoints(); it != --cp.BeginPoints(); --it)
            drawControl(p, NDCToQPixel(*it), it.Index() == _selectedControl);
    }
}

void TFOpacityMap::mousePressEvent(QMouseEvent *event)
{
    emit Activated(this);
    vec2 mouse = qvec2(event->posF());
    auto it = findSelectedControlPoint(mouse);
    
    if (it != _controlPoints.EndPoints()) {
        _draggedControl = it;
        _dragOffset = NDCToPixel(*it) - mouse;
        _isDraggingControl = true;
        _controlStartValue = *it;
        selectControlPoint(it);
        _paramsMgr->BeginSaveStateGroup("Move opacity control point");
    } else {
        DeselectControlPoint();
        event->ignore();
    }
}

void TFOpacityMap::mouseReleaseEvent(QMouseEvent *event)
{
    if (_isDraggingControl) {
//        if (*_draggedControl != _controlStartValue)
        opacityChanged();
        _paramsMgr->EndSaveStateGroup();
    } else
        event->ignore();
    _isDraggingControl = false;
}

void TFOpacityMap::mouseMoveEvent(QMouseEvent *event)
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
        emit UpdateInfo(newVal.x, newVal.y);
        update();
        opacityChanged();
        _paramsMgr->IntermediateChange();
    } else {
        event->ignore();
    }
}

void TFOpacityMap::mouseDoubleClickEvent(QMouseEvent *event)
{
    vec2 mouse = qvec2(event->pos());
    ControlPointList &cp = _controlPoints;
    
    auto controlPointIt = findSelectedControlPoint(mouse);
    if (controlPointIt != cp.EndPoints()) {
        deleteControlPoint(controlPointIt);
        return;
    }
    
    for (auto it = cp.BeginLines(); it != cp.EndLines(); ++it) {
        const vec2 a = NDCToPixel(it.a());
        const vec2 b = NDCToPixel(it.b());
        
        if (DistanceToLine(a, b, mouse) <= GetControlPointRadius()) {
            addControlPoint(PixelToNDC(Project(a, b, mouse)));
            return;
        }
    }
    
    event->ignore();
}

void TFOpacityMap::opacityChanged()
{
    if (!_renderParams)
        return;
    
    MapperFunction *mf = _renderParams->GetMapperFunc(_renderParams->GetVariableName());
    
    OpacityMap *om = mf->GetOpacityMap(0);
    
    vector<double> cp(_controlPoints.Size() * 2);
    for (int i = 0; i < _controlPoints.Size(); i++) {
        cp[i*2]   = _controlPoints[i].y;
        cp[i*2+1] = _controlPoints[i].x;
    }
    om->SetControlPoints(cp);
}

bool TFOpacityMap::controlPointContainsPixel(const vec2 &cp, const vec2 &pixel) const
{
    return glm::distance(pixel, NDCToPixel(cp)) <= GetControlPointRadius();
}

ControlPointList::PointIterator TFOpacityMap::findSelectedControlPoint(const glm::vec2 &mouse)
{
    const auto end = _controlPoints.EndPoints();
    for (auto it = _controlPoints.BeginPoints(); it != end; ++it)
        if (controlPointContainsPixel(*it, mouse))
            return it;
    return end;
}

void TFOpacityMap::selectControlPoint(ControlPointList::PointIterator it)
{
    _selectedControl = it.Index();
    update();
    emit UpdateInfo((*it).x, (*it).y);
}

void TFOpacityMap::deleteControlPoint(ControlPointList::PointIterator it)
{
    _controlPoints.Remove(it);
    update();
    opacityChanged();
    DeselectControlPoint();
}

void TFOpacityMap::addControlPoint(const glm::vec2 &ndc)
{
    ControlPointList &cp = _controlPoints;
    int index = cp.Add(ndc);
    selectControlPoint(cp.BeginPoints() + index);
    update();
    opacityChanged();
}

void TFOpacityMap::menuDeleteSelectedControlPoint()
{
    if (_selectedControl >= 0 && _selectedControl < _controlPoints.Size())
        deleteControlPoint(_controlPoints.BeginPoints() + _selectedControl);
}

void TFOpacityMap::menuAddControlPoint()
{
    QVariant location = sender()->property("location");
    if (location.isValid())
        addControlPoint(qvec2(location.toPointF()));
}

void TFOpacityMap::DeselectControlPoint()
{
    _selectedControl = -1;
    update();
    emit ControlPointDeselected();
}

void TFOpacityMap::UpdateFromInfo(float value, float opacity)
{
    assert(_selectedControl >= 0);
    assert(value >= 0 && value <= 1);
    assert(opacity >= 0 && opacity <= 1);
    
    _controlPoints.Remove(_controlPoints.BeginPoints() + _selectedControl);
    _selectedControl = _controlPoints.Add(vec2(value, opacity));
    
    opacityChanged();
}



TFOpacityWidget::TFOpacityWidget()
: TFMapWidget(new TFOpacityMap(this))
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    this->setFrameStyle(QFrame::Box);
}
