#pragma once

#include <QWidget>
#include <QFrame>
#include <vapor/VAssert.h>
#include "Histo.h"
#include "TFMapWidget.h"

class TFHistogramMap : public TFMap {
    Q_OBJECT
    
    enum class ScalingType {
        Linear=0,
        Logarithmic=1,
        Boolean=2
    };
    
public:
    bool DynamicScaling = true;
    
    TFHistogramMap(TFMapWidget *parent);
    
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
    QSize minimumSizeHint() const;
    void Deactivate() {}
    
protected:
    TFInfoWidget *createInfoWidget();
    void paintEvent(QPainter &p);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
//    void mouseDoubleClickEvent(QMouseEvent *event);
    
private:
    VAPoR::DataMgr *_dataMgr = nullptr;
    VAPoR::RenderParams *_renderParams = nullptr;
    Histo _histo;
    
    ScalingType _getScalingType() const;
    
signals:
    void InfoDeselected();
    void UpdateInfo(float value);
};

class TFHistogramWidget : public TFMapWidget {
public:
    TFHistogramWidget() : TFMapWidget(new TFHistogramMap(this)) {}
};
