#include "TFOpacityWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QAction>
#include <glm/glm.hpp>
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include "TFOpacityInfoWidget.h"
#include "TFUtils.h"

using namespace VAPoR;
using glm::clamp;
using glm::vec2;
using std::vector;

static vec2    qvec2(const QPoint &qp) { return vec2(qp.x(), qp.y()); }
static vec2    qvec2(const QPointF &qp) { return vec2(qp.x(), qp.y()); }
static QPointF qvec2(const vec2 &v) { return QPointF(v.x, v.y); }

static vec2 Project(vec2 a, vec2 b, vec2 p)
{
    vec2  n = glm::normalize(b - a);
    float t = glm::dot(n, p - a);

    return n * t + a;
}

static float DistanceToLine(vec2 a, vec2 b, vec2 p)
{
    vec2  n = glm::normalize(b - a);
    float t = glm::dot(n, p - a);

    if (t < 0) return glm::distance(a, p);
    if (t > glm::distance(a, b)) return glm::distance(b, p);

    vec2 projection = n * t + a;
    return glm::distance(projection, p);
}

#define CONTROL_POINT_RADIUS (4.0f)
#define PADDING              (CONTROL_POINT_RADIUS + 1.0f)

TFOpacityMap::TFOpacityMap(const std::string &variableNameTag, TFMapWidget *parent) : TFMap(variableNameTag, parent) {}

QSize TFOpacityMap::minimumSizeHint() const { return QSize(100, 75); }

void TFOpacityMap::LostFocus() { DeselectControlPoint(); }

#define PROPERTY_INDEX    ("index")
#define PROPERTY_LOCATION ("location")

void TFOpacityMap::PopulateContextMenu(QMenu *menu, const glm::vec2 &p)
{
    auto selected = findSelectedControlPoint(p);

    if (selected != _controlPoints.EndPoints())
        menu->addAction("Delete control point", this, SLOT(menuDeleteSelectedControlPoint()))->setProperty(PROPERTY_INDEX, QVariant(selected.Index()));
    else
        menu->addAction("Add control point", this, SLOT(menuAddControlPoint()))->setProperty(PROPERTY_LOCATION, QVariant(qvec2(PixelToNDC(p))));
}

void TFOpacityMap::PopulateSettingsMenu(QMenu *menu) const
{
    menu->addAction("Save Transfer Function", this, SLOT(menuSave()));
    menu->addAction("Load Transfer Function", this, SLOT(menuLoad()));
}

void TFOpacityMap::paramsUpdate()
{
    MapperFunction *mf = getRenderParams()->GetMapperFunc(getVariableName());
    // TODO Multiple opacity maps?
    //    int n = mf->getNumOpacityMaps();
    //    printf("# opacity maps = %i\n", n);

    OpacityMap *om = mf->GetOpacityMap(0);

    vector<double> cp = om->GetControlPoints();
    _controlPoints.Resize(cp.size() / 2);
    for (int i = 0; i < cp.size(); i += 2) {
        _controlPoints[i / 2].y = cp[i];
        _controlPoints[i / 2].x = cp[i + 1];
    }
    update();

    if (_selectedControl > -1) UpdateInfo(_controlPoints[_selectedControl].x, _controlPoints[_selectedControl].y);
}

TFInfoWidget *TFOpacityMap::createInfoWidget()
{
    TFOpacityInfoWidget *info = new TFOpacityInfoWidget(getVariableNameTag());

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

        for (auto it = --cp.EndPoints(); it != --cp.BeginPoints(); --it) drawControl(p, NDCToQPixel(*it), it.Index() == _selectedControl);
    }
}

void TFOpacityMap::mousePressEvent(QMouseEvent *event)
{
    emit Activated(this);
    vec2 mouse = qvec2(event->localPos());
    auto it = findSelectedControlPoint(mouse);
    auto lineIt = findSelectedControlLine(mouse);

    if (it != _controlPoints.EndPoints()) {
        _draggedControl = it;
        _dragOffset = NDCToPixel(*it) - mouse;
        _isDraggingControl = true;
        selectControlPoint(it);
        getParamsMgr()->BeginSaveStateGroup("Move opacity control point");
    } else if (lineIt != _controlPoints.EndLines()) {
        _draggedLine = lineIt;
        _dragOffset = NDCToPixel(lineIt.a()) - mouse;
        _dragOffsetB = NDCToPixel(lineIt.b()) - mouse;
        _isDraggingLine = true;
        getParamsMgr()->BeginSaveStateGroup("Move opacity control line");
    } else {
        DeselectControlPoint();
        event->ignore();
    }
}

void TFOpacityMap::mouseReleaseEvent(QMouseEvent *event)
{
    if (_isDraggingControl || _isDraggingLine) {
        opacityChanged();
        getParamsMgr()->EndSaveStateGroup();
    } else
        event->ignore();
    _isDraggingControl = false;
    _isDraggingLine = false;
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
        vec2        newVal = glm::clamp(PixelToNDC(mouse + _dragOffset), vec2(it.IsFirst() ? 0 : (*(it - 1)).x, 0), vec2(it.IsLast() ? 1 : (*(it + 1)).x, 1));

        *_draggedControl = newVal;
        emit UpdateInfo(newVal.x, newVal.y);
        update();
        opacityChanged();
        getParamsMgr()->IntermediateChange();
    } else if (_isDraggingLine) {
        auto &it = _draggedLine;
        it.setA(glm::clamp(PixelToNDC(mouse + _dragOffset), vec2(it.IsFirst() ? 0 : (it - 1).a().x, 0), vec2(it.IsLast() ? 1 : (it + 1).b().x, 1)));
        it.setB(glm::clamp(PixelToNDC(mouse + _dragOffsetB), vec2(it.IsFirst() ? 0 : (it - 1).a().x, 0), vec2(it.IsLast() ? 1 : (it + 1).b().x, 1)));
        update();
        opacityChanged();
        getParamsMgr()->IntermediateChange();
    } else {
        event->ignore();
    }
}

void TFOpacityMap::mouseDoubleClickEvent(QMouseEvent *event)
{
    vec2              mouse = qvec2(event->pos());
    ControlPointList &cp = _controlPoints;

    auto controlPointIt = findSelectedControlPoint(mouse);
    if (controlPointIt != cp.EndPoints()) {
        deleteControlPoint(controlPointIt);
        return;
    }

    auto controlLineIt = findSelectedControlLine(mouse);
    if (controlLineIt != cp.EndLines()) {
        const vec2 a = NDCToPixel(controlLineIt.a());
        const vec2 b = NDCToPixel(controlLineIt.b());
        addControlPoint(PixelToNDC(Project(a, b, mouse)));
        return;
    }

    event->ignore();
}

void TFOpacityMap::opacityChanged()
{
    if (!getRenderParams()) return;

    MapperFunction *mf = getRenderParams()->GetMapperFunc(getVariableName());

    OpacityMap *om = mf->GetOpacityMap(0);

    vector<double> cp(_controlPoints.Size() * 2);
    for (int i = 0; i < _controlPoints.Size(); i++) {
        cp[i * 2] = _controlPoints[i].y;
        cp[i * 2 + 1] = _controlPoints[i].x;
    }
    om->SetControlPoints(cp);
}

bool TFOpacityMap::controlPointContainsPixel(const vec2 &cp, const vec2 &pixel) const { return glm::distance(pixel, NDCToPixel(cp)) <= GetControlPointRadius(); }

ControlPointList::PointIterator TFOpacityMap::findSelectedControlPoint(const glm::vec2 &mouse)
{
    const auto end = _controlPoints.EndPoints();
    for (auto it = _controlPoints.BeginPoints(); it != end; ++it)
        if (controlPointContainsPixel(*it, mouse)) return it;
    return end;
}

ControlPointList::LineIterator TFOpacityMap::findSelectedControlLine(const glm::vec2 &mouse)
{
    ControlPointList &cp = _controlPoints;
    const float       radius = GetControlPointRadius();

    for (auto it = cp.BeginLines(); it != cp.EndLines(); ++it) {
        const vec2 a = NDCToPixel(it.a());
        const vec2 b = NDCToPixel(it.b());

        if (DistanceToLine(a, b, mouse) <= radius) return it;
    }
    return cp.EndLines();
}

void TFOpacityMap::selectControlPoint(ControlPointList::PointIterator it)
{
    _selectedControl = it.Index();
    update();
    emit UpdateInfo((*it).x, (*it).y);
}

void TFOpacityMap::deleteControlPoint(ControlPointList::PointIterator it)
{
    if (_isDraggingControl || _isDraggingLine) {
        getParamsMgr()->EndSaveStateGroup();
        _isDraggingControl = false;
        _isDraggingLine = false;
    }

    if (_selectedControl == it.Index())
        DeselectControlPoint();
    else if (_selectedControl > it.Index())
        _selectedControl--;

    _controlPoints.Remove(it);
    update();
    opacityChanged();
}

void TFOpacityMap::addControlPoint(const glm::vec2 &ndc)
{
    ControlPointList &cp = _controlPoints;
    int               index = cp.Add(ndc);
    selectControlPoint(cp.BeginPoints() + index);
    update();
    opacityChanged();
}

void TFOpacityMap::menuDeleteSelectedControlPoint()
{
    emit     Activated(this);
    QVariant indexVariant = sender()->property(PROPERTY_INDEX);
    if (indexVariant.isValid()) {
        int index = indexVariant.toInt();
        if (index >= 0 && index < _controlPoints.Size()) deleteControlPoint(_controlPoints.BeginPoints() + index);
    }
}

void TFOpacityMap::menuAddControlPoint()
{
    emit     Activated(this);
    QVariant location = sender()->property(PROPERTY_LOCATION);
    if (location.isValid()) addControlPoint(qvec2(location.toPointF()));
}

void TFOpacityMap::menuLoad()
{
    RenderParams *rp = getRenderParams();
    if (!rp) return;
    TFUtils::LoadTransferFunction(getParamsMgr(), rp->GetMapperFunc(getVariableName()));
}

void TFOpacityMap::menuSave()
{
    RenderParams *rp = getRenderParams();
    if (!rp) return;
    TFUtils::SaveTransferFunction(getParamsMgr(), rp->GetMapperFunc(getVariableName()));
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

TFOpacityWidget::TFOpacityWidget(const std::string &variableNameTag) : TFMapWidget(new TFOpacityMap(variableNameTag, this))
{
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    // this->setFrameStyle(QFrame::Box);
}
