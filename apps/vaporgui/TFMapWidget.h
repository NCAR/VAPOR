#pragma once

#include <QFrame>
#include <QMenu>
#include <glm/glm.hpp>
#include <vector>

namespace VAPoR {
    class DataMgr;
    class ParamsMgr;
    class RenderParams;
}

class TFInfoWidget;
class TFMapWidget;

class TFMap : public QObject {
    Q_OBJECT
    
    TFMapWidget *_parent = nullptr;
    TFInfoWidget *_infoWidget = nullptr;
    int _width = 0;
    int _height = 0;
    bool _insideSaveStateGroup = false;
    
public:
    TFMap(TFMapWidget *parent = nullptr);
    
    TFInfoWidget *GetInfoWidget();
    virtual void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) = 0;
    virtual void Deactivate() = 0;
    virtual QSize minimumSizeHint() const = 0;
    virtual void PopulateContextMenu(QMenu *menu, const glm::vec2 &p) {}
    virtual void PopulateSettingsMenu(QMenu *menu) const {}
    
    int width() const { return _width; }
    int height() const { return _height; }
    void resize(int width, int height);
    QRect     paddedRect() const;
    QRect     rect() const;
    const QFont getFont() const;
    
    virtual void paintEvent(QPainter &p) = 0;
    virtual void mousePressEvent      (QMouseEvent *event);
    virtual void mouseReleaseEvent    (QMouseEvent *event);
    virtual void mouseMoveEvent       (QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    
    friend class TFMapWidget;
    
public slots:
    void update();
    
protected:
    void drawControl(QPainter &p, const QPointF &pos, bool selected = false) const;
    virtual TFInfoWidget *createInfoWidget() = 0;
    
    glm::vec2 NDCToPixel(const glm::vec2 &v) const;
    QPointF   NDCToQPixel(const glm::vec2 &v) const;
    QPointF   NDCToQPixel(float x, float y) const;
    glm::vec2 PixelToNDC(const QPointF &p) const;
    glm::vec2 PixelToNDC(const glm::vec2 &p) const;
    
    virtual QMargins GetPadding() const;
    int GetControlPointRadius() const;
    
    void BeginSaveStateGroup(VAPoR::ParamsMgr *paramsMgr, const std::string &description="");
    void EndSaveStateGroup(VAPoR::ParamsMgr *paramsMgr);
    void CancelSaveStateGroup(VAPoR::ParamsMgr *paramsMgr);
    
signals:
    void Activated(TFMap *who);
};

class TFMapWidget : public QFrame {
    Q_OBJECT
    
    std::vector<TFMap *>_maps;
    
public:
    TFMapWidget(TFMap *map);
    ~TFMapWidget();
    void AddMap(TFMap *map);
    std::vector<TFMap *> GetMaps() const;
    TFInfoWidget *GetInfoWidget();
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    void Deactivate();
    QSize minimumSizeHint() const override;
    
signals:
    void Activated(TFMapWidget *who);
    
private slots:
    void _mapActivated(TFMap *who);
    void _showContextMenu(const QPoint &);
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent * event) override;
};
