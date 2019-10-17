#include "TFIsoValueWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include "QPaintUtils.h"
#include "TFIsoValueInfoWidget.h"

using namespace VAPoR;
using glm::vec2;
using std::vector;

static vec2 qvec2(const QPoint &qp) { return vec2(qp.x(), qp.y()); }

TFIsoValueMap::TFIsoValueMap(TFMapWidget *parent) : TFMap(parent) {}

void TFIsoValueMap::paramsUpdate()
{
    if (!getRenderParams()->HasIsoValues()) {
        hide();
        return;
    }

    loadFromParams(getRenderParams());
    update();

    if (_selectedId > -1) UpdateInfo(_isoValues[_selectedId]);
}

#define PROPERTY_INDEX ("index")
#define PROPERTY_VALUE ("value")

void TFIsoValueMap::PopulateContextMenu(QMenu *menu, const glm::vec2 &p)
{
    if (_equidistantIsoValues) return;

    int selectedId = findSelectedControlPoint(p);
    if (selectedId != -1)
        menu->addAction("Delete control point", this, SLOT(menuDeleteControlPoint()))->setProperty(PROPERTY_INDEX, selectedId);
    else {
        QAction *action = menu->addAction("Add control point", this, SLOT(menuAddControlPoint()));
        action->setProperty(PROPERTY_VALUE, QVariant(valueForControlX(p.x)));
        if (_isoValues.size() >= 4) action->setDisabled(true);
    }
}

QSize TFIsoValueMap::minimumSizeHint() const
{
    QSize s = GetControlPointArea(QPoint(0, 0)).size();
    if (BottomPadding) s.setHeight(s.height() + GetControlPointRadius());
    return s;
}

void TFIsoValueMap::LostFocus() { DeselectControlPoint(); }

TFInfoWidget *TFIsoValueMap::createInfoWidget()
{
    TFIsoValueInfoWidget *info = new TFIsoValueInfoWidget;
    info->UsingColormapVariable = this->UsingColormapVariable;

    connect(this, SIGNAL(ControlPointDeselected()), info, SLOT(Deselect()));
    connect(this, SIGNAL(UpdateInfo(float)), info, SLOT(SetControlPoint(float)));
    connect(info, SIGNAL(ControlPointChanged(float)), this, SLOT(UpdateFromInfo(float)));

    return info;
}

void TFIsoValueMap::paintEvent(QPainter &p)
{
    p.fillRect(rect(), Qt::lightGray);

    if (getRenderParams()) {
        for (int i = 0; i < _isoValues.size(); i++) {
            float v = _isoValues[i];
            bool  invalid = false;
            if (v < 0 || v > 1) {
                v = glm::clamp(v, 0.f, 1.f);
                invalid = true;
            }
            drawControl(p, controlQPositionForValue(v), i == _selectedId, invalid);
        }

        if (_isoValues.size() == 0) {
            QFont font = getFont();
            font.setPixelSize(rect().height());
            p.setFont(font);
            p.drawText(rect(), Qt::AlignCenter, "doubleclick to add isovalues");
        }
    }
}

void TFIsoValueMap::drawControl(QPainter &p, const QPointF &pos, bool selected, bool invalid) const
{
    float r = GetControlPointRadius();
    float t = GetControlPointTriangleHeight();
    float s = GetControlPointSquareHeight();

    QPen   pen(Qt::darkGray, 0.5);
    QBrush brush(QColor(0xfa, 0xfa, 0xfa));
    p.setBrush(brush);
    p.setPen(pen);

    if (invalid) p.setBrush(QBrush(QColor(0xfa, 0x8a, 0x8a)));

    QPolygonF graph;
    graph.push_back(pos + QPointF(0, 0));
    graph.push_back(pos + QPointF(-r, t));
    graph.push_back(pos + QPointF(-r, t + s));
    graph.push_back(pos + QPointF(r, t + s));
    graph.push_back(pos + QPointF(r, t));

    p.drawPolygon(graph);

    if (selected) {
        if (selected) {
            p.setPen(Qt::NoPen);
            p.setBrush(QBrush(Qt::black));
            r *= 0.38;
            p.drawEllipse(pos + QPointF(0, t + (s / 3)), r, r);
        }
    }
}

float TFIsoValueMap::GetControlPointTriangleHeight() const { return GetControlPointRadius() * 2 * 0.618; }

float TFIsoValueMap::GetControlPointSquareHeight() const { return GetControlPointRadius() * 1.618; }

QRect TFIsoValueMap::GetControlPointArea(const QPoint &p) const
{
    float h = GetControlPointSquareHeight() + GetControlPointTriangleHeight();
    float r = GetControlPointRadius();
    return QRect(p - QPoint(r, 0), p + QPoint(r, h));
}

void TFIsoValueMap::mousePressEvent(QMouseEvent *event)
{
    emit Activated(this);
    vec2 mouse(event->pos().x(), event->pos().y());

    int selectedId = findSelectedControlPoint(mouse);
    if (selectedId >= 0) {
        _isDraggingControl = true;
        _draggingControlID = selectedId;
        selectControlPoint(selectedId);
        update();
        _dragOffset = controlPositionForValue(glm::clamp(_isoValues[selectedId], 0.f, 1.f)) - mouse;
        getParamsMgr()->BeginSaveStateGroup("IsoValue modification");
        return;
    }

    DeselectControlPoint();
    event->ignore();
    update();
}

void TFIsoValueMap::mouseReleaseEvent(QMouseEvent *event)
{
    if (_isDraggingControl)
        getParamsMgr()->EndSaveStateGroup();
    else
        event->ignore();
    _isDraggingControl = false;
}

void TFIsoValueMap::mouseMoveEvent(QMouseEvent *event)
{
    vec2 mouse = qvec2(event->pos());

    if (_isDraggingControl) {
        float newVal = glm::clamp(valueForControlX(mouse.x + _dragOffset.x), 0.f, 1.f);

        moveControlPoint(&_draggingControlID, newVal);
        selectControlPoint(_draggingControlID);
        saveToParams(getRenderParams());
        update();
        getParamsMgr()->IntermediateChange();
    } else {
        event->ignore();
    }
}

void TFIsoValueMap::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (_equidistantIsoValues) return;

    vec2 mouse = qvec2(event->pos());
    int  selectedId = findSelectedControlPoint(mouse);
    if (selectedId >= 0) {
        deleteControlPoint(selectedId);
        return;
    }

    if (_isoValues.size() < 4) {
        float newVal = valueForControlX(mouse.x);
        if (newVal >= 0 && newVal <= 1) { addControlPoint(newVal); }
        return;
    }
}

QMargins TFIsoValueMap::GetPadding() const
{
    QMargins m = TFMap::GetPadding();
    m.setTop(0);
    return m;
}

void TFIsoValueMap::saveToParams(VAPoR::RenderParams *rp) const
{
    if (!rp) return;
    if (!rp->HasIsoValues()) return;
    assert(rp->HasIsoValues());

    const float min = getMapRangeMin();
    const float max = getMapRangeMax();

    vector<double> values(_isoValues.size());
    for (int i = 0; i < values.size(); i++) values[i] = _isoValues[i] * (max - min) + min;
    rp->SetIsoValues(values);
}

void TFIsoValueMap::loadFromParams(VAPoR::RenderParams *rp)
{
    if (!rp) return;
    if (!rp->HasIsoValues()) return;
    assert(rp->HasIsoValues());

    const float min = getMapRangeMin();
    const float max = getMapRangeMax();

    vector<double> newValues = rp->GetIsoValues();
    if (newValues.size() != _isoValues.size()) {
        DeselectControlPoint();
        _isoValues.resize(newValues.size());
    }
    for (int i = 0; i < newValues.size(); i++) _isoValues[i] = (newValues[i] - min) / (max - min);

    //    if (!_equidistantIsoValues)
    //        clampIsoValuesToMappingRange();
}

// void TFIsoValueMap::clampIsoValuesToMappingRange()
//{
//   / for (int i = 0; i < _isoValues.size(); i++)
//        _isoValues[i] = glm::clamp(_isoValues[i], 0.f, 1.f);
//}

int TFIsoValueMap::addControlPoint(float value)
{
    int index = -1;
    for (int i = 0; i < _isoValues.size(); i++) {
        if (value < _isoValues[i]) {
            _isoValues.insert(_isoValues.begin() + i, value);
            index = i;
            break;
        }
    }
    if (index == -1) {
        _isoValues.push_back(value);
        index = _isoValues.size() - 1;
    }
    saveToParams(getRenderParams());
    selectControlPoint(index);
    update();
    return index;
}

void TFIsoValueMap::deleteControlPoint(int i)
{
    if (i == _selectedId) DeselectControlPoint();
    _isoValues.erase(_isoValues.begin() + i);
    update();
    saveToParams(getRenderParams());
}

void TFIsoValueMap::moveControlPoint(int *index, float value)
{
    if (_equidistantIsoValues) {
        if (*index == 0) {
            const float initialValue = _isoValues[0];
            const float diff = value - initialValue;
            for (float &v : _isoValues) v += diff;
        } else {
            const float baseValue = _isoValues[0];
            float       spacing = (value - baseValue) / (float)(*index);
            spacing = std::max(spacing, 0.f);
            for (int i = 1; i < _isoValues.size(); i++) _isoValues[i] = baseValue + spacing * i;
        }
    } else {
        deleteControlPoint(*index);
        *index = addControlPoint(value);
    }
}

void TFIsoValueMap::selectControlPoint(int index)
{
    _selectedId = index;
    float value = _isoValues[index];
    emit  UpdateInfo(value);
}

void TFIsoValueMap::DeselectControlPoint()
{
    _selectedId = -1;
    emit ControlPointDeselected();
    update();
}

void TFIsoValueMap::UpdateFromInfo(float value)
{
    if (_selectedId >= 0 && _selectedId < _isoValues.size()) {
        moveControlPoint(&_selectedId, value);
        update();
        saveToParams(getRenderParams());
    }
}

int TFIsoValueMap::findSelectedControlPoint(const glm::vec2 &mouse) const
{
    for (int i = _isoValues.size() - 1; i >= 0; i--)
        if (controlPointContainsPixel(glm::clamp(_isoValues[i], 0.f, 1.f), mouse)) return i;
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

glm::vec2 TFIsoValueMap::controlPositionForValue(float value) const { return vec2(controlXForValue(value), 0); }

float TFIsoValueMap::controlXForValue(float value) const { return NDCToPixel(vec2(value, 0.f)).x; }

float TFIsoValueMap::valueForControlX(float position) const { return PixelToNDC(vec2(position, 0.f)).x; }

float TFIsoValueMap::getMapRangeMin() const
{
    if (!getRenderParams()) return 0;
    return getRenderParams()->GetMapperFunc(getRenderParams()->GetVariableName())->getMinMapValue();
}

float TFIsoValueMap::getMapRangeMax() const
{
    if (!getRenderParams()) return 1;
    return getRenderParams()->GetMapperFunc(getRenderParams()->GetVariableName())->getMaxMapValue();
}

void TFIsoValueMap::menuDeleteControlPoint()
{
    emit     Activated(this);
    QVariant valueVariant = sender()->property(PROPERTY_INDEX);
    if (valueVariant.isValid()) {
        int index = valueVariant.toInt();
        if (index >= 0 && index < _isoValues.size()) deleteControlPoint(index);
    }
}

void TFIsoValueMap::menuAddControlPoint()
{
    emit     Activated(this);
    QVariant value = sender()->property(PROPERTY_VALUE);
    if (value.isValid()) addControlPoint(value.toFloat());
}
