#pragma once

#include "TFMapWidget.h"
#include <vapor/RenderParams.h>
#include <vapor/ParamsMgr.h>
#include <vapor/VAssert.h>
#include <glm/glm.hpp>

class TFColorInfoWidget;

class TFColorWidget : public TFMapWidget {
    Q_OBJECT
    
public:
    TFColorWidget();
    ~TFColorWidget();
    
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
    QSize minimumSizeHint() const;
    TFColorInfoWidget *GetInfoWidget() const;
    
protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    
private:
    VAPoR::ParamsMgr *_paramsMgr = nullptr;
    VAPoR::RenderParams *_renderParams = nullptr;
    bool _isDraggingControl = false;
    int _draggingControlID;
    glm::vec2 _dragOffset;
    glm::vec2 m;
    TFColorInfoWidget *_infoWidget;
    
    void opacityChanged();
    
    bool controlPointContainsPixel(const glm::vec2 &cp, const glm::vec2 &pixel) const;
    
    VAPoR::ColorMap *getColormap() const;
    int findSelectedControlPoint(const glm::vec2 &mouse) const;
    bool controlPointContainsPixel(float cp, const glm::vec2 &pixel) const;
    QPointF controlQPositionForValue(float value) const;
    glm::vec2 controlPositionForValue(float value) const;
    float controlXForValue(float value) const;
    float valueForControlX(float position) const;
};
