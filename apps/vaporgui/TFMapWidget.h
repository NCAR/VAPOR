#pragma once

#include <QFrame>
#include <glm/glm.hpp>

namespace VAPoR {
    class DataMgr;
    class ParamsMgr;
    class RenderParams;
}

class TFInfoWidget;

class TFMapWidget : public QFrame {
    Q_OBJECT
    
public:
    TFInfoWidget *GetInfoWidget();
    virtual void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams) = 0;
    virtual void Deactivate() = 0;
    
protected:
    void drawControl(QPainter &p, const QPointF &pos, bool selected = false) const;
    virtual TFInfoWidget *createInfoWidget() = 0;
    
    glm::vec2 NDCToPixel(const glm::vec2 &v) const;
    QPointF   NDCToQPixel(const glm::vec2 &v) const;
    QPointF   NDCToQPixel(float x, float y) const;
    glm::vec2 PixelToNDC(const QPointF &p) const;
    glm::vec2 PixelToNDC(const glm::vec2 &p) const;
    QRectF    PaddedRect() const;
    
    int GetPadding() const;
    int GetControlPointRadius() const;
    
private:
    TFInfoWidget *_infoWidget = nullptr;
    
signals:
    void Activated(TFMapWidget *who);
};
