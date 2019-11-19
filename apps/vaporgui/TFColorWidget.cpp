#include "TFColorWidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include <vapor/ResourcePath.h>
#include <vapor/FileUtils.h>
#include "TFColorInfoWidget.h"
#include "TFUtils.h"
#include "QPaintUtils.h"
#include "ParamsMenuItems.h"

using namespace VAPoR;
using glm::vec2;
using std::vector;

static vec2    qvec2(const QPoint &qp) { return vec2(qp.x(), qp.y()); }
static vec2    qvec2(const QPointF &qp) { return vec2(qp.x(), qp.y()); }
static QPointF qvec2(const vec2 &v) { return QPointF(v.x, v.y); }

TFColorMap::TFColorMap(TFMapWidget *parent) : TFMap(parent)
{
    _colorInterpolationMenu = new ParamsDropdownMenuItem(this, VAPoR::ColorMap::_interpTypeTag, {"Linear", "Discrete", "Diverging"},
                                                         {TFInterpolator::linear, TFInterpolator::discrete, TFInterpolator::diverging}, "Color Interpolation");
    _colorInterpolationWhitepointAction = new ParamsCheckboxMenuItem(this, ColorMap::_useWhitespaceTag, "Add Whitespace");
    _colorInterpolationMenu->menu()->addAction(_colorInterpolationWhitepointAction);
}

QSize TFColorMap::minimumSizeHint() const { return QSize(100, 30); }

void TFColorMap::LostFocus() { DeselectControlPoint(); }

#define PROPERTY_INDEX ("index")
#define PROPERTY_VALUE ("value")

void TFColorMap::PopulateContextMenu(QMenu *menu, const glm::vec2 &p)
{
    int selected = findSelectedControlPoint(p);

    if (selected != -1)
        menu->addAction("Delete control point", this, SLOT(menuDeleteSelectedControlPoint()))->setProperty(PROPERTY_INDEX, selected);
    else
        menu->addAction("Add control point", this, SLOT(menuAddControlPoint()))->setProperty(PROPERTY_VALUE, QVariant(valueForControlX(p.x)));
}

void TFColorMap::PopulateSettingsMenu(QMenu *menu) const
{
    menu->addAction(_colorInterpolationMenu);
    menu->addSeparator();
    menu->addAction("Save Colormap", this, SLOT(menuSave()));
    menu->addAction("Load Colormap", this, SLOT(menuLoad()));

    QMenu *builtinColormapMenu = menu->addMenu("Load Built-In Colormap");
    string builtinPath = GetSharePath("palettes");
    auto   fileNames = FileUtils::ListFiles(builtinPath);
    std::sort(fileNames.begin(), fileNames.end());
    for (int i = 0; i < fileNames.size(); i++) {
        string path = FileUtils::JoinPaths({builtinPath, fileNames[i]});

        if (FileUtils::Extension(path) != "tf3") continue;

        QAction *item = new ColorMapMenuItem(path);
        connect(item, SIGNAL(triggered(std::string)), this, SLOT(menuLoadBuiltin(std::string)));
        builtinColormapMenu->addAction(item);
    }
}

void TFColorMap::paramsUpdate()
{
    _colorInterpolationMenu->Update(getColormap());
    _colorInterpolationWhitepointAction->Update(getColormap());
    _colorInterpolationWhitepointAction->setEnabled(getColormap()->GetInterpType() == TFInterpolator::diverging);
    update();

    if (_selectedId > -1) UpdateInfo(getColormap()->controlPointValueNormalized(_selectedId), VColorToQColor(getColormap()->controlPointColor(_selectedId)));
}

TFInfoWidget *TFColorMap::createInfoWidget()
{
    TFColorInfoWidget *info = new TFColorInfoWidget;
    info->UsingColormapVariable = this->UsingColormapVariable;

    connect(this, SIGNAL(ControlPointDeselected()), info, SLOT(DeselectControlPoint()));
    connect(this, SIGNAL(UpdateInfo(float, QColor)), info, SLOT(SetControlPoint(float, QColor)));
    connect(info, SIGNAL(ControlPointChanged(float, QColor)), this, SLOT(UpdateFromInfo(float, QColor)));

    return info;
}

#define CONTROL_POINT_RADIUS (4.0f)
#define PADDING              (CONTROL_POINT_RADIUS + 1.0f)

void TFColorMap::paintEvent(QPainter &p)
{
    //     243 245 249
    p.fillRect(rect(), Qt::lightGray);
    QPaintUtils::BoxDropShadow(p, paddedRect(), 10, QColor(0, 0, 0, 120));

    RenderParams *rp = getRenderParams();

    ColorMap *cm = rp->GetMapperFunc(getVariableName())->GetColorMap();

    QMargins       padding = GetPadding();
    int            nSamples = width() - (padding.left() + padding.right());
    unsigned char *buf = new unsigned char[nSamples * 3];
    float          rgb[3];
    for (int i = 0; i < nSamples; i++) {
        cm->colorNormalized(i / (float)nSamples).toRGB(rgb);
        buf[i * 3 + 0] = rgb[0] * 255;
        buf[i * 3 + 1] = rgb[1] * 255;
        buf[i * 3 + 2] = rgb[2] * 255;
    }
    QImage image(buf, nSamples, 1, QImage::Format::Format_RGB888);

    p.drawImage(paddedRect(), image);

    for (int i = 0; i < cm->numControlPoints(); i++) { drawControl(p, controlQPositionForValue(cm->controlPointValueNormalized(i)), i == _selectedId); }
    delete[] buf;
}

void TFColorMap::mousePressEvent(QMouseEvent *event)
{
    emit      Activated(this);
    ColorMap *cm = getColormap();
    vec2      mouse(event->pos().x(), event->pos().y());

    for (int i = 0; i < cm->numControlPoints(); i++) {
        float value = cm->controlPointValueNormalized(i);
        if (controlPointContainsPixel(value, mouse)) {
            _isDraggingControl = true;
            _draggingControlID = i;
            selectControlPoint(i);
            update();
            _dragOffset = controlPositionForValue(value) - mouse;
            BeginSaveStateGroup(getParamsMgr(), "Colormap modification");
            return;
        }
    }

    DeselectControlPoint();
    event->ignore();
    update();
}

void TFColorMap::mouseReleaseEvent(QMouseEvent *event)
{
    if (_isDraggingControl)
        EndSaveStateGroup(getParamsMgr());
    else
        event->ignore();
    _isDraggingControl = false;
}

void TFColorMap::mouseMoveEvent(QMouseEvent *event)
{
    vec2 mouse = qvec2(event->pos());

    if (_isDraggingControl) {
        float newVal = glm::clamp(valueForControlX(mouse.x + _dragOffset.x), 0.f, 1.f);

        moveControlPoint(&_draggingControlID, newVal);
        selectControlPoint(_draggingControlID);
        update();
        getParamsMgr()->IntermediateChange();
    } else {
        event->ignore();
    }
}

void TFColorMap::mouseDoubleClickEvent(QMouseEvent *event)
{
    vec2 mouse = qvec2(event->pos());
    int  selectedId = findSelectedControlPoint(mouse);
    if (selectedId >= 0) {
        deleteControlPoint(selectedId);
        return;
    }

    float newVal = valueForControlX(mouse.x);
    if (newVal >= 0 && newVal <= 1) addControlPoint(newVal);
}

void TFColorMap::moveControlPoint(int *index, float value, const VAPoR::ColorMap::Color &c)
{
    ColorMap *cm = getColormap();

    cm->deleteControlPoint(*index);
    *index = cm->addNormControlPoint(value, c);
}

void TFColorMap::moveControlPoint(int *index, float value)
{
    ColorMap *      cm = getColormap();
    ColorMap::Color c = cm->controlPointColor(_draggingControlID);
    moveControlPoint(index, value, c);
}

void TFColorMap::deleteControlPoint(int index)
{
    if (index == _selectedId)
        DeselectControlPoint();
    else if (index < _selectedId)
        _selectedId--;
    getColormap()->deleteControlPoint(index);
    update();
}

void TFColorMap::addControlPoint(float value)
{
    selectControlPoint(getColormap()->addNormControlPointAt(value));
    update();
}

ColorMap *TFColorMap::getColormap() const { return getRenderParams()->GetMapperFunc(getVariableName())->GetColorMap(); }

void TFColorMap::selectControlPoint(int index)
{
    _selectedId = index;
    ColorMap *cm = getColormap();

    float           value = cm->controlPointValueNormalized(index);
    ColorMap::Color vColor = cm->controlPointColor(index);

    UpdateInfo(value, VColorToQColor(vColor));
}

void TFColorMap::DeselectControlPoint()
{
    _selectedId = -1;
    emit ControlPointDeselected();
    update();
}

void TFColorMap::UpdateFromInfo(float value, QColor color) { moveControlPoint(&_selectedId, value, QColorToVColor(color)); }

void TFColorMap::menuDeleteSelectedControlPoint()
{
    emit            Activated(this);
    const ColorMap *cm = getColormap();
    QVariant        valueVariant = sender()->property(PROPERTY_INDEX);
    if (valueVariant.isValid()) {
        int index = valueVariant.toInt();
        if (index >= 0 && index < cm->numControlPoints()) deleteControlPoint(index);
    }
}

void TFColorMap::menuAddControlPoint()
{
    emit     Activated(this);
    QVariant value = sender()->property(PROPERTY_VALUE);
    if (value.isValid()) addControlPoint(value.toFloat());
}

void TFColorMap::menuLoad()
{
    RenderParams *rp = getRenderParams();
    if (!rp) return;
    TFUtils::LoadColormap(getParamsMgr(), rp->GetMapperFunc(getVariableName()));
}

void TFColorMap::menuSave()
{
    RenderParams *rp = getRenderParams();
    if (!rp) return;
    TFUtils::SaveTransferFunction(getParamsMgr(), rp->GetMapperFunc(getVariableName()));
}

void TFColorMap::menuLoadBuiltin(std::string path)
{
    RenderParams *rp = getRenderParams();
    if (!rp) return;
    TFUtils::LoadColormap(rp->GetMapperFunc(getVariableName()), path);
}

int TFColorMap::findSelectedControlPoint(const glm::vec2 &mouse) const
{
    const ColorMap *cm = getColormap();
    const int       n = cm->numControlPoints();
    for (int i = 0; i < n; i++)
        if (controlPointContainsPixel(cm->controlPointValueNormalized(i), mouse)) return i;
    return -1;
}

bool TFColorMap::controlPointContainsPixel(float cp, const vec2 &pixel) const { return glm::distance(pixel, controlPositionForValue(cp)) <= GetControlPointRadius(); }

QPointF TFColorMap::controlQPositionForValue(float value) const
{
    const vec2 v = controlPositionForValue(value);
    return QPointF(v.x, v.y);
}

glm::vec2 TFColorMap::controlPositionForValue(float value) const { return vec2(controlXForValue(value), height() / 2.f); }

float TFColorMap::controlXForValue(float value) const { return NDCToPixel(vec2(value, 0.f)).x; }

float TFColorMap::valueForControlX(float position) const { return PixelToNDC(vec2(position, 0.f)).x; }

QColor TFColorMap::VColorToQColor(const ColorMap::Color &c)
{
    float rgb[3];
    c.toRGB(rgb);
    return QColor(rgb[0] * 255, rgb[1] * 255, rgb[2] * 255);
}

ColorMap::Color TFColorMap::QColorToVColor(const QColor &c)
{
    double h, s, v;
    c.getHsvF(&h, &s, &v);
    return ColorMap::Color(h, s, v);
}

#include <vapor/STLUtils.h>
#include <QPushButton>

std::map<std::string, QIcon> ColorMapMenuItem::icons;

QIcon ColorMapMenuItem::getCachedIcon(const std::string &path)
{
    auto it = icons.find(path);
    if (it != icons.end()) return it->second;

    ParamsBase::StateSave stateSave;
    MapperFunction        mf(&stateSave);

    mf.LoadColormapFromFile(path);
    ColorMap *cm = mf.GetColorMap();

    QSize          size = getIconSize();
    int            nSamples = size.width();
    unsigned char *buf = new unsigned char[nSamples * 3];
    float          rgb[3];
    for (int i = 0; i < nSamples; i++) {
        cm->colorNormalized(i / (float)nSamples).toRGB(rgb);
        buf[i * 3 + 0] = rgb[0] * 255;
        buf[i * 3 + 1] = rgb[1] * 255;
        buf[i * 3 + 2] = rgb[2] * 255;
    }
    QImage image(buf, nSamples, 1, QImage::Format::Format_RGB888);
    icons[path] = QIcon(QPixmap::fromImage(image).scaled(size.width(), size.height()));

    delete[] buf;
    return icons[path];
}

QSize ColorMapMenuItem::getIconSize() { return QSize(50, 15); }

QSize ColorMapMenuItem::getIconPadding() { return QSize(10, 10); }

ColorMapMenuItem::ColorMapMenuItem(const std::string &path) : QWidgetAction(nullptr), _path(path)
{
    QPushButton *button = new QPushButton;
    setDefaultWidget(button);

    button->setIcon(getCachedIcon(path));
    button->setFixedSize(getIconSize() + getIconPadding());
    button->installEventFilter(this);

    string name = STLUtils::Split(FileUtils::Basename(path), ".")[0];
    button->setToolTip(QString::fromStdString(name));

    button->setStyleSheet(R"(
                          QPushButton {
                              icon-size: 50px 15px;
                          padding: 0px;
                          margin: 0px;
                          background: none;
                          border: none;
                          }
                          QPushButton::hover {
                          background: #aaa;
                          }
                          )");
}

// Manually riggering an action does not close the menu so it has to be done manually.
void ColorMapMenuItem::CloseMenu(QAction *action)
{
    if (!action) return;

    QList<QWidget *> menus = action->associatedWidgets();

    for (QWidget *widget : menus) {
        QMenu *menu = dynamic_cast<QMenu *>(widget);
        if (!menu) continue;
        if (menu->isHidden()) continue;

        menu->hide();
        CloseMenu(menu->menuAction());
    }
}

bool ColorMapMenuItem::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonRelease) {
        trigger();
        emit triggered(_path);
        CloseMenu(this);
        return true;
    }
    return QObject::eventFilter(obj, event);
}
