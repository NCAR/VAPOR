#pragma once

#include <QWidget>
#include <QFrame>
#include <vapor/VAssert.h>
#include "Histo.h"
#include "TFMapWidget.h"

class ParamsDropdownMenuItem;

class TFHistogramMap : public TFMap {
    Q_OBJECT

    enum ScalingType { Linear = 0, Logarithmic, Boolean, ScalingTypeCount };

public:
    TFHistogramMap(TFMapWidget *parent = nullptr);

    QSize minimumSizeHint() const;
    void  LostFocus() {}
    void  PopulateSettingsMenu(QMenu *menu) const;

protected:
    void          paramsUpdate();
    TFInfoWidget *createInfoWidget();
    void          paintEvent(QPainter &p);
    void          mousePressEvent(QMouseEvent *event);
    void          mouseReleaseEvent(QMouseEvent *event);
    void          mouseMoveEvent(QMouseEvent *event);
    //    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    VAPoR::DataMgr *        _dataMgr = nullptr;
    VAPoR::RenderParams *   _renderParams = nullptr;
    Histo                   _histo;
    ParamsDropdownMenuItem *_scalingMenu;
    bool                    _dynamicScaling = true;

    ScalingType _getScalingType() const;

private slots:
    void _menuDynamicScalingToggled(bool on);

signals:
    void InfoDeselected();
    void UpdateInfo(float value);
};

class TFHistogramWidget : public TFMapWidget {
public:
    TFHistogramWidget() : TFMapWidget(new TFHistogramMap(this)) {}
};
