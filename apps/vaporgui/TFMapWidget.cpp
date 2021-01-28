#include "TFMapWidget.h"
#include <QPainter>
#include <QResizeEvent>
#include <QMenu>
#include <vapor/VAssert.h>
#include <vapor/ParamsMgr.h>

using glm::vec2;

#define CONTROL_POINT_RADIUS (4.0f)
#define PADDING              (CONTROL_POINT_RADIUS + 1.0f)

TFMap::TFMap(const std::string &variableNameTag, TFMapWidget *parent) : _variableNameTag(variableNameTag), _parent(parent) {}

TFInfoWidget *TFMap::GetInfoWidget()
{
    if (!_infoWidget) _infoWidget = createInfoWidget();

    return _infoWidget;
}

void TFMap::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    assert(_parent);
    if (_renderParams != rParams) LostFocus();

    _dataMgr = dataMgr;
    _paramsMgr = paramsMgr;
    _renderParams = rParams;

    if (HasValidParams()) paramsUpdate();
}

bool TFMap::HasValidParams() const { return _dataMgr && _paramsMgr && _renderParams && _dataMgr->VariableExists(_renderParams->GetCurrentTimestep(), getVariableName()); }

bool TFMap::IsShown() const { return !_hidden; }

void TFMap::resize(int width, int height)
{
    _width = width;
    _height = height;
}

bool TFMap::isLargeEnoughToPaint() const
{
    QMargins p = GetPadding();
    if (_width - (p.left() + p.right()) <= 0) return false;
    if (_height - (p.top() + p.bottom()) <= 0) return false;
    return true;
}

void TFMap::mousePressEvent(QMouseEvent *event) { event->ignore(); }
void TFMap::mouseReleaseEvent(QMouseEvent *event) { event->ignore(); }
void TFMap::mouseMoveEvent(QMouseEvent *event) { event->ignore(); }
void TFMap::mouseDoubleClickEvent(QMouseEvent *event) { event->ignore(); }

void TFMap::update()
{
    if (_parent) _parent->update();
}

void TFMap::show()
{
    _hidden = false;
    if (_parent) _parent->showMap(this);
}

void TFMap::hide()
{
    _hidden = true;
    if (_parent) _parent->hideMap(this);
}

VAPoR::MapperFunction *TFMap::getMapperFunction() const { return _renderParams->GetMapperFunc(getVariableName()); }

std::string TFMap::getVariableName() const { return _renderParams->GetValueString(_variableNameTag, ""); }

const std::string &TFMap::getVariableNameTag() const { return _variableNameTag; }

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
        p.drawEllipse(pos, radius * 2, radius * 2);
    }

    QPen   pen(Qt::darkGray, 0.5);
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

QRect TFMap::paddedRect() const
{
    const QMargins p = GetPadding();
    return QRect(p.left(), p.top(), width() - (p.left() + p.right()), height() - (p.top() + p.bottom()));
}

QRect TFMap::rect() const { return QRect(0, 0, width(), height()); }

const QFont TFMap::getFont() const
{
    if (_parent) return _parent->font();
    return QFont();
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

QPointF TFMap::NDCToQPixel(float x, float y) const { return NDCToQPixel(vec2(x, y)); }

glm::vec2 TFMap::PixelToNDC(const glm::vec2 &p) const
{
    float width = TFMap::width();
    float height = TFMap::height();
    if (width == 0 || height == 0) return vec2(0);

    return vec2((p.x - PADDING) / (width - 2 * PADDING), 1.0f - (p.y - PADDING) / (height - 2 * PADDING));
}

glm::vec2 TFMap::PixelToNDC(const QPointF &p) const { return PixelToNDC(vec2(p.x(), p.y())); }

QMargins TFMap::GetPadding() const { return QMargins(PADDING, PADDING, PADDING, PADDING); }

int TFMap::GetControlPointRadius() const { return CONTROL_POINT_RADIUS; }

void TFMap::BeginSaveStateGroup(VAPoR::ParamsMgr *paramsMgr, const std::string &description)
{
    assert(!_insideSaveStateGroup);
    paramsMgr->BeginSaveStateGroup(description);
    _insideSaveStateGroup = true;
}

void TFMap::EndSaveStateGroup(VAPoR::ParamsMgr *paramsMgr)
{
    assert(_insideSaveStateGroup);
    paramsMgr->EndSaveStateGroup();
    _insideSaveStateGroup = false;
}

void TFMap::CancelSaveStateGroup(VAPoR::ParamsMgr *paramsMgr)
{
    if (_insideSaveStateGroup) paramsMgr->EndSaveStateGroup();
    _insideSaveStateGroup = false;
}

TFMapWidget::TFMapWidget(TFMap *map)
{
    AddMap(map);
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(_showContextMenu(const QPoint &)));
}

TFMapWidget::~TFMapWidget()
{
    for (TFMap *map : _maps) { delete map; }
}

void TFMapWidget::AddMap(TFMap *map)
{
    if (std::find(_maps.begin(), _maps.end(), map) == _maps.end()) {
        map->_parent = this;
        _maps.push_back(map);
        connect(map, SIGNAL(Activated(TFMap *)), this, SLOT(_mapActivated(TFMap *)));
    }
}

std::vector<TFMap *> TFMapWidget::GetMaps() const { return _maps; }

TFInfoWidget *TFMapWidget::GetInfoWidget()
{
    assert(_maps.size() == 1);
    if (_maps.size()) return _maps[0]->GetInfoWidget();
    return nullptr;
}

void TFMapWidget::Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams)
{
    for (auto map : _maps) map->Update(dataMgr, paramsMgr, rParams);
}

void TFMapWidget::Deactivate()
{
    for (auto map : _maps) map->LostFocus();
}

QSize TFMapWidget::minimumSizeHint() const
{
    QSize max(0, 0);
    for (auto map : _maps) {
        QSize s = map->minimumSizeHint();
        max.setWidth(std::max(max.width(), s.width()));
        max.setHeight(std::max(max.height(), s.height()));
    }
    return max;
}

void TFMapWidget::showMap(TFMap *map)
{
    map->_hidden = false;
    show();
}

void TFMapWidget::hideMap(TFMap *map)
{
    map->_hidden = true;
    int nHidden = 0;
    for (TFMap *m : _maps)
        if (m->_hidden) nHidden++;
    if (nHidden == _maps.size()) hide();
}

void TFMapWidget::_mapActivated(TFMap *who)
{
    for (auto map : _maps)
        if (map != who) map->LostFocus();

    emit Activated(this);
}

void TFMapWidget::_showContextMenu(const QPoint &qp)
{
    vec2  p(qp.x(), qp.y());
    QMenu menu("Context Menu", this);

    for (auto map : _maps) {
        if (map->HasValidParams() && map->rect().contains(qp)) {
            map->PopulateContextMenu(&menu, p);
            menu.addSeparator();
            map->PopulateSettingsMenu(&menu);
            menu.addSeparator();
        }
    }

    menu.exec(mapToGlobal(qp));
}

//#include <vapor/GLManager.h>
void TFMapWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    //    void *s = VAPoR::GLManager::BeginTimer();
    for (int i = _maps.size() - 1; i >= 0; i--) {
        if (!_maps[i]->HasValidParams() || !_maps[i]->isLargeEnoughToPaint() || _maps[i]->_hidden) continue;
        p.save();
        _maps[i]->paintEvent(p);
        p.restore();
    }
    //    printf("Paint took %fs\n", VAPoR::GLManager::EndTimer(s));
}

void TFMapWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) return;    // Reserved for context menu

    for (auto map : _maps) {
        if (!map->HasValidParams() || map->_hidden) continue;
        event->accept();
        map->mousePressEvent(event);
        if (event->isAccepted()) break;
    }
}

void TFMapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) return;    // Reserved for context menu

    for (auto map : _maps) {
        if (!map->HasValidParams() || map->_hidden) continue;
        event->accept();
        map->mouseReleaseEvent(event);
        if (event->isAccepted()) break;
    }
}

void TFMapWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) return;    // Reserved for context menu

    for (auto map : _maps) {
        if (!map->HasValidParams() || map->_hidden) continue;
        event->accept();
        map->mouseMoveEvent(event);
        if (event->isAccepted()) break;
    }
}

void TFMapWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) return;    // Reserved for context menu

    for (auto map : _maps) {
        if (!map->HasValidParams() || map->_hidden) continue;
        event->accept();
        map->mouseDoubleClickEvent(event);
        if (event->isAccepted()) break;
    }
}

void TFMapWidget::resizeEvent(QResizeEvent *event)
{
    for (auto map : _maps) { map->resize(event->size().width(), event->size().height()); }
}
