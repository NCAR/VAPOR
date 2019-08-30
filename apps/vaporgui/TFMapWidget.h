#pragma once

#include <QFrame>
#include <glm/glm.hpp>

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
    
public:
    TFMap(TFMapWidget *parent);
    
    TFInfoWidget *GetInfoWidget();
    virtual void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) = 0;
    virtual void Deactivate() = 0;
    virtual QSize minimumSizeHint() const = 0;
    
    int width() const { return _width; }
    int height() const { return _height; }
    void resize(int width, int height);
    
    virtual void paintEvent(QPainter &p) = 0;
    virtual void mousePressEvent(QMouseEvent *event) {}
    virtual void mouseReleaseEvent(QMouseEvent *event) {}
    virtual void mouseMoveEvent(QMouseEvent *event) {}
    virtual void mouseDoubleClickEvent(QMouseEvent *event) {}
    
protected:
    void drawControl(QPainter &p, const QPointF &pos, bool selected = false) const;
    virtual TFInfoWidget *createInfoWidget() = 0;
    
    void update();
    
    glm::vec2 NDCToPixel(const glm::vec2 &v) const;
    QPointF   NDCToQPixel(const glm::vec2 &v) const;
    QPointF   NDCToQPixel(float x, float y) const;
    glm::vec2 PixelToNDC(const QPointF &p) const;
    glm::vec2 PixelToNDC(const glm::vec2 &p) const;
    QRectF    PaddedRect() const;
    QRectF    rect() const;
    
    int GetPadding() const;
    int GetControlPointRadius() const;
    
signals:
    void Activated(TFMap *who);
};

class TFMapWidget : public QFrame {
    Q_OBJECT
    
public:
    TFInfoWidget *GetInfoWidget();
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    void Deactivate();
    QSize minimumSizeHint() const override;
    
protected:
    TFMap *_map = nullptr;
    
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent * event);
};
