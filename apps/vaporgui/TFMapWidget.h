#pragma once

#include <QFrame>
#include <QMenu>
#include <glm/glm.hpp>
#include <vector>

namespace VAPoR {
class DataMgr;
class ParamsMgr;
class RenderParams;
class MapperFunction;
}    // namespace VAPoR

class TFInfoWidget;
class TFMapWidget;

//! \class TFMap
//! A widget for editing a transfer function mapped value, for example opacity or color.
//! They are not QWidgets because of limitations in Qt which do not allow widgets to be
//! stacked on top of another with passthrough mouse events. Because of this, TFMapWidget
//! manually holds TFMaps and manages such events.

class TFMap : public QObject {
    Q_OBJECT

    const std::string _variableNameTag;
    TFMapWidget *     _parent = nullptr;
    TFInfoWidget *    _infoWidget = nullptr;
    int               _width = 0;
    int               _height = 0;
    bool              _insideSaveStateGroup = false;
    bool              _hidden = false;

    VAPoR::DataMgr *     _dataMgr = nullptr;
    VAPoR::ParamsMgr *   _paramsMgr = nullptr;
    VAPoR::RenderParams *_renderParams = nullptr;

public:
    TFMap(const std::string &variableNameTag, TFMapWidget *parent = nullptr);

    TFInfoWidget *GetInfoWidget();
    void          Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    bool          HasValidParams() const;
    bool          IsShown() const;
    virtual void  LostFocus() = 0;
    virtual QSize minimumSizeHint() const = 0;
    //! (Right-click menu)
    virtual void PopulateContextMenu(QMenu *menu, const glm::vec2 &p) {}
    virtual void PopulateSettingsMenu(QMenu *menu) const {}

    int  width() const { return _width; }
    int  height() const { return _height; }
    void resize(int width, int height);
    //! Sometimes Qt tries painting a 0 sized widget
    bool isLargeEnoughToPaint() const;
    //! Returns the rect of the internal padded area
    QRect       paddedRect() const;
    QRect       rect() const;
    const QFont getFont() const;

    //! These map to the QWidget counterparts
    virtual void paintEvent(QPainter &p) = 0;
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);

    friend class TFMapWidget;

public slots:
    //! These map to the QWidget counterparts
    void update();
    void show();
    void hide();

protected:
    virtual void paramsUpdate() = 0;

    VAPoR::DataMgr *       getDataMgr() const { return _dataMgr; }
    VAPoR::ParamsMgr *     getParamsMgr() const { return _paramsMgr; }
    VAPoR::RenderParams *  getRenderParams() const { return _renderParams; }
    VAPoR::MapperFunction *getMapperFunction() const;
    std::string            getVariableName() const;
    const std::string &    getVariableNameTag() const;

    void                  drawControl(QPainter &p, const QPointF &pos, bool selected = false) const;
    virtual TFInfoWidget *createInfoWidget() = 0;

    glm::vec2 NDCToPixel(const glm::vec2 &v) const;
    QPointF   NDCToQPixel(const glm::vec2 &v) const;
    QPointF   NDCToQPixel(float x, float y) const;
    glm::vec2 PixelToNDC(const QPointF &p) const;
    glm::vec2 PixelToNDC(const glm::vec2 &p) const;

    virtual QMargins GetPadding() const;
    int              GetControlPointRadius() const;

    void BeginSaveStateGroup(VAPoR::ParamsMgr *paramsMgr, const std::string &description = "");
    void EndSaveStateGroup(VAPoR::ParamsMgr *paramsMgr);
    void CancelSaveStateGroup(VAPoR::ParamsMgr *paramsMgr);

signals:
    //! Emittend when focus was gained
    void Activated(TFMap *who);
};

//! \class TFMapWidget
//! Wrap a TFMap(s) in a QWidget

class TFMapWidget : public QWidget {
    Q_OBJECT

    std::vector<TFMap *> _maps;

public:
    TFMapWidget(TFMap *map);
    ~TFMapWidget();
    void                 AddMap(TFMap *map);
    std::vector<TFMap *> GetMaps() const;
    TFInfoWidget *       GetInfoWidget();
    void                 Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    void                 Deactivate();
    QSize                minimumSizeHint() const override;
    void                 showMap(TFMap *map);
    void                 hideMap(TFMap *map);

signals:
    void Activated(TFMapWidget *who);

private slots:
    void _mapActivated(TFMap *who);
    void _showContextMenu(const QPoint &);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
};
