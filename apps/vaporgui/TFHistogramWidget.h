#pragma once

#include <QWidget>
#include <QFrame>
#include <vapor/VAssert.h>
#include <vapor/Histo.h>
#include "TFMapWidget.h"

class ParamsDropdownMenuItem;

class TFHistogramMap : public TFMap {
    Q_OBJECT

    enum ScalingType { Linear = 0, Logarithmic, Boolean, ScalingTypeCount };

public:
    TFHistogramMap(const std::string &variableNameTag, TFMapWidget *parent = nullptr);

    QSize minimumSizeHint() const;
    void  LostFocus() {}
    void  PopulateSettingsMenu(QMenu *menu) const;

protected:
    void          paramsUpdate();
    TFInfoWidget *createInfoWidget();
    void          _paintEvent(QPainter &p);
    void          mousePressEvent(QMouseEvent *event);
    void          mouseReleaseEvent(QMouseEvent *event);
    void          mouseMoveEvent(QMouseEvent *event);
    //    void mouseDoubleClickEvent(QMouseEvent *event);

private:
    Histo                   _histo;
    ParamsDropdownMenuItem *_scalingMenu;
    bool                    _dynamicScaling = true;

    vector<double> _mapRange;

    ScalingType _getScalingType() const;

private slots:
    void _menuDynamicScalingToggled(bool on);

signals:
    void InfoDeselected();
    void UpdateInfo(float value);
};

class TFHistogramWidget : public TFMapWidget {
    Q_OBJECT
public:
    TFHistogramWidget(const std::string &variableNameTag) : TFMapWidget(new TFHistogramMap(variableNameTag, this)) {}
};
