#pragma once

#include <QWidget>
#include <QFrame>
#include <vapor/VAssert.h>
#include "Histo.h"
#include "TFMapWidget.h"
#include "ParamsMenuItems.h"

class TFHistogramMap : public TFMap {
    Q_OBJECT
    
    enum ScalingType {
        Linear=0,
        Logarithmic,
        Boolean,
        ScalingTypeCount
    };
    ParamsDropdownMenuItem *scalingMenu;
    
public:
    TFHistogramMap(TFMapWidget *parent);
    void Update(VAPoR::DataMgr *dataMgr, VAPoR::ParamsMgr *paramsMgr, VAPoR::RenderParams *rParams);
    
    QSize minimumSizeHint() const;
    void Deactivate() {}
    void PopulateSettingsMenu(QMenu *menu) const;
    
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
    QAction *_scalingActions[ScalingTypeCount];
    bool _dynamicScaling = true;
    
    ScalingType _getScalingType() const;
    
private slots:
    void _menuSetScalingType();
    void _menuDynamicScalingToggled(bool on);
    
signals:
    void InfoDeselected();
    void UpdateInfo(float value);
};

class TFHistogramWidget : public TFMapWidget {
public:
    TFHistogramWidget() : TFMapWidget(new TFHistogramMap(this)) {}
};
