#pragma once

#include <QWidget>
#include <QFrame>
#include <vapor/VAssert.h>
#include <glm/glm.hpp>
#include "Histo.h"
#include "TFMapWidget.h"

class TFHistogramWidget : public TFMapWidget {
    Q_OBJECT
    
public:
    
    TFHistogramWidget();
    
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
    QSize minimumSizeHint() const;
    void Deactivate() {}
    
protected:
    TFInfoWidget *createInfoWidget();
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
//    void mouseDoubleClickEvent(QMouseEvent *event);
    
private:
    VAPoR::DataMgr *_dataMgr = nullptr;
    VAPoR::RenderParams *_renderParams = nullptr;
    Histo _histo;
    
    glm::vec2 NDCToPixel(const glm::vec2 &v) const;
    QPointF   NDCToQPixel(const glm::vec2 &v) const;
    QPointF   NDCToQPixel(float x, float y) const;
    glm::vec2 PixelToNDC(const QPointF &p) const;
    glm::vec2 PixelToNDC(const glm::vec2 &p) const;
    
signals:
    void InfoDeselected();
    void UpdateInfo(float value);
};
