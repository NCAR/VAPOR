#pragma once

#include "TFMapWidget.h"
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include <vapor/VAssert.h>
#include <glm/glm.hpp>

class TFColorInfoWidget;
class ParamsDropdownMenuItem;
class ParamsCheckboxMenuItem;

class TFColorMap : public TFMap {
    Q_OBJECT

  public:
    TFColorMap(TFMapWidget *parent = nullptr);

    QSize minimumSizeHint() const override;
    void LostFocus() override;
    void PopulateContextMenu(QMenu *menu, const glm::vec2 &p) override;
    void PopulateSettingsMenu(QMenu *menu) const override;

  private:
    void populateBuiltinColormapMenu(QMenu *menu, const std::string &builtinPath) const;

  protected:
    void paramsUpdate() override;
    TFInfoWidget *createInfoWidget() override;
    void paintEvent(QPainter &p) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

  private:
    ParamsDropdownMenuItem *_colorInterpolationMenu;
    ParamsCheckboxMenuItem *_colorInterpolationWhitepointAction;
    bool _isDraggingControl = false;
    int _draggingControlID;
    int _selectedId = -1;
    glm::vec2 _dragOffset;
    glm::vec2 m;

    bool controlPointContainsPixel(const glm::vec2 &cp, const glm::vec2 &pixel) const;

    void moveControlPoint(int *index, float value, const VAPoR::ColorMap::Color &c);
    void moveControlPoint(int *index, float value);
    void deleteControlPoint(int index);
    void addControlPoint(float value);
    VAPoR::ColorMap *getColormap() const;
    void selectControlPoint(int index);
    int findSelectedControlPoint(const glm::vec2 &mouse) const;
    bool controlPointContainsPixel(float cp, const glm::vec2 &pixel) const;
    QPointF controlQPositionForValue(float value) const;
    glm::vec2 controlPositionForValue(float value) const;
    float controlXForValue(float value) const;
    float valueForControlX(float position) const;

    static QColor VColorToQColor(const VAPoR::ColorMap::Color &c);
    static VAPoR::ColorMap::Color QColorToVColor(const QColor &c);

  signals:
    void ControlPointDeselected();
    void UpdateInfo(float value, QColor color);

  public slots:
    void DeselectControlPoint();
    void UpdateFromInfo(float value, QColor color);

  private slots:
    void menuDeleteSelectedControlPoint();
    void menuAddControlPoint();
    void menuLoad();
    void menuSave();
    void menuLoadBuiltin(std::string path);
    void menuReverse();
};

class TFColorWidget : public TFMapWidget {
  public:
    TFColorWidget() : TFMapWidget(new TFColorMap(this)) {}
};

#include <QWidgetAction>
class ColorMapMenuItem : public QWidgetAction {
    Q_OBJECT

    static std::map<std::string, QIcon> icons;
    static QIcon getCachedIcon(const std::string &path);
    static QSize getIconSize();
    static QSize getIconPadding();

    const std::string _path;

  public:
    ColorMapMenuItem(const std::string &path);
    static void CloseMenu(QAction *action);

  protected:
    bool eventFilter(QObject *obj, QEvent *event);

  signals:
    void triggered(std::string colormapPath);
};
